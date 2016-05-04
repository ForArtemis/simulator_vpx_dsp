
// * HISTORY
 //* Version    Date     Auth.,Loc.     Comments
// *   0.1    27-Jul-04  LM & CC, EKB   First released version
// ********************************************************************/
#include "SRIO_switch.h"
#include "KeyStone_SRIO_init_drv.h"
#include "csl_srio.h"
#include "csl_srioAux.h"
#include <xdc/runtime/System.h>

extern uint32_t DEVICE_ID1_16BIT;
extern uint32_t DEVICE_ID1_8BIT;
extern uint32_t DEVICE_ID2_16BIT;
extern uint32_t DEVICE_ID2_8BIT;
extern uint32_t DEVICE_ID3_16BIT;
extern uint32_t DEVICE_ID3_8BIT;
extern uint32_t DEVICE_ID4_16BIT;
extern uint32_t DEVICE_ID4_8BIT;

Uint32  system_type;
Uint32  nextdevid;
Uint32  hostdeviceid;
Uint32  uiMaintenanceValue, uiResult;

int rio_discovery_fabric(Uint32 uiLocalPort);

// ------------------------------------------------------------
//
// config the host device
//
// ------------------------------------------------------------
int rio_local_config (void)
{
	if (gpSRIO_regs->RIO_DEV_ID == TMS6678_DIDCAR_VALUE)
	{
		System_printf("Checked DIDCAR of local device is TMS6678 \n");
	}
	else
	{
		System_printf ("ERROR : Check CCSBAR local DIDCAR read as 0x%08x \n", gpSRIO_regs->RIO_DEV_ID);
		return UNKNOWN_DEVICE;
	}
	// config as host and set the discovered bit
	gpSRIO_regs->RIO_SP_GEN_CTL= DEVICE_CONFIG_HOST;

	if (gpSRIO_regs->RIO_SP_GEN_CTL== DEVICE_CONFIG_HOST)
		System_printf ("This device is configured as a RIO host \n");
	else
	{
		System_printf ("ERROR : This device is not configured as host, RIO_SP_GEN_CTL=0x%08x \n", gpSRIO_regs->RIO_SP_GEN_CTL);
		return NOT_HOST_PROCESSOR;
	}

	System_printf ("This device initially has RIO device ID 0x%08x \n",
			((gpSRIO_regs->RIO_BASE_ID&CSL_SRIO_RIO_BASE_ID_BASE_ID_MASK) >> CSL_SRIO_RIO_BASE_ID_BASE_ID_SHIFT));

	// config local RIO_BASE_ID = 0
	gpSRIO_regs->RIO_BASE_ID=0;
	System_printf ("This device has been allocated RIO device ID 0x%08x \n",
	((gpSRIO_regs->RIO_BASE_ID&CSL_SRIO_RIO_BASE_ID_BASE_ID_MASK) >>
	CSL_SRIO_RIO_BASE_ID_BASE_ID_SHIFT));

	// set host device ID, then start find other devices and assign IDs
	hostdeviceid = (gpSRIO_regs->RIO_BASE_ID&CSL_SRIO_RIO_BASE_ID_BASE_ID_MASK) >>
	CSL_SRIO_RIO_BASE_ID_BASE_ID_SHIFT;

	/* software will start allocating device ID upwards from here - up to max deviceid */
	nextdevid = hostdeviceid + 1;

	//System_printf ("RapidIO port is trained OK \n");

//	if((srioRegs->RIO_SP[0].RIO_SP_CTL & PWIDTH_MASK) == PWIDTH1L0)
//		System_printf("RapidIO port trained 1x on lane 0\n");
//
//	if((srioRegs->RIO_SP[0].RIO_SP_CTL & PWIDTH_MASK) == PWIDTH1L2)
//		System_printf("RapidIO port trained 1x on lane 2\n");
//
//	if((srioRegs->RIO_SP[0].RIO_SP_CTL & PWIDTH_MASK) == PWIDTH4)
//		System_printf("RapidIO port trained 4x \n");

	return SUCCESS;
}

// ------------------------------------------------------------------------
//
// find the host connected device
//
// ------------------------------------------------------------------------

int rio_discovery(Uint32 uiLocalPort)
{

	Uint32 didcaradj;
	int result;

	/************************************************************
	 STEP 1: Identify the adjacent device
	 ************************************************************/
	uiResult = KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
				0x00, RIO_DIDCAR_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
				SRIO_PKT_TYPE_MTN_READ,0);
	didcaradj= uiMaintenanceValue;
	/************************************************************
	 STEP 2:
	 If adjacent device is CPS1432, execute the fabric version of the
	 descovery process, else Exit
	 *************************************************************/
	if (didcaradj == CPS1432_DIDCAR_VALUE)
	{
		System_printf("Identified adjacent device as CPS1432\n");
		system_type = FABRIC_SYSTEM;
		result = rio_discovery_fabric(uiLocalPort);
	}
	else
	{
		System_printf("ERROR : Adjacent device is not CPS1432, Read back didcar of 0x%08x \n",
				didcaradj);
		System_printf ("Please re-start \n");
		return UNKNOWN_DEVICE;
	}
	return result;
}

/********************************************************************
 A version of the simplified discovery process for a fabric based system
 Makes certain assumptions about the nature of that system i.e. TSI578
 plus between 1 and 7 additional MPC641Ds
 *********************************************************************/
int rio_discovery_fabric(Uint32 uiLocalPort)
{
	Uint32 training;
	Uint32 hbdidl;
	/*UINT32 route;*/
	Uint32 bdid;

	// swport: The port number from which the maintenance read operation accessed this register
	//         i.e. the port of the sw which the host device attached to
	// numport:The total number of S-RIO ports that can be configured through
	//         the CPS-1432¡¯s registers
	Uint32 swport, numport;

	Uint32 temp;
	Uint32 errstatus_offset;
	Uint32 ctl_offset;
	/*UINT32 portwidth;*/
	int portnum, portnum1;
	int portstep, portstep1;


	System_printf("Executing the fabric version of the discovery code \n");

	/************************************************************
	 STEP 1: Check switch port
	 Read back the port of the CPS1432 we are attached to.
	 **************************************************/
	uiResult = KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
					0xff, CPS1432_SW_PORT, GLOBAL_ADDR(&uiMaintenanceValue),
					SRIO_PKT_TYPE_MTN_READ,0);
	swport = uiMaintenanceValue;

	System_printf("read back from SW_PORT = 0x%08x \n", swport);
	numport = swport >> 8;
	swport = swport & BOTTOM_BYTE_ONLY_MASK;
	System_printf("The switch has %d ports\n", numport);
	System_printf("This host device is attached to port %d \n", swport);

	/********************************************
	 STEP 2: Read back the HBDIDL(Host Base deviceID Lock CSR) on the switch
	 This lock register determines which
	 device initialized the device. 0xFFFF indicates
	 that the device has not yet been initialized, if the
	 HBDIDL is some other value, issue warning but continue
	 ********************************************/
	uiResult = KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
						0xff, RIO_HOSTLOCK_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
						SRIO_PKT_TYPE_MTN_READ,0);
	hbdidl= uiMaintenanceValue;

	System_printf("Original CPS1432 HBDIDL =0x%08x \n", hbdidl);
	if (hbdidl != HBDIDLCSR_NOT_LOCKED)
	{
		System_printf("WARNING : HBDIDL of the CPS1432 was 0x%08x \n", hbdidl);
	}

	/********************************************
	 STEP 3 : Attempt to lock HBDIDL, write
	 the device ID of the switch
	 ********************************************/
	//++++ edit by LRH ++++:
	uiMaintenanceValue = DEVICE_ID1_16BIT;

	uiResult= KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
			RIO_HOSTLOCK_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
		SRIO_PKT_TYPE_MTN_WRITE,0);

	/********************************************
	 STEP 4: check that the lock has been accepted
	 If the lock has not been accepted, issue warning, but continue
	 (dangerous!!!!)
	 ********************************************/
	KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
							0xff, RIO_HOSTLOCK_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
							SRIO_PKT_TYPE_MTN_READ,0);
	hbdidl= uiMaintenanceValue;
	System_printf("Updated HBDIDL on CPS1432 = 0x%08x \n", hbdidl);
	if(hbdidl != DEVICE_ID1_16BIT)
	{
		System_printf("WARNING: HBDIDL not updated, Lock not accepted\n");
		System_printf("WARINING : This application continues regardless...");
	}

	/***************************************************************
	 STEP 5 : route all packets for host device id back to this port
	 only use simple route table, so CPS1432 per-port routing is
	 not used in this example
	 use in-direct method to configure the route table
	 ****************************************************************/
	//++++ edit by LRH ++++
	hostdeviceid = DEVICE_ID1_16BIT;
	uiMaintenanceValue = hostdeviceid;

	uiResult= KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
			RIO_ROUTE_CFG_DESTID, GLOBAL_ADDR(&uiMaintenanceValue),
		SRIO_PKT_TYPE_MTN_WRITE,0);

	uiMaintenanceValue = swport;
	uiResult= KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
			RIO_ROUTE_CFG_PORT, GLOBAL_ADDR(&uiMaintenanceValue),
		SRIO_PKT_TYPE_MTN_WRITE,0);

	System_printf("Routed all packets for deviceID 0x%02x to port %d \n",
			hostdeviceid, swport);

	KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, DEVICE_ID1_16BIT,
			RIO_DIDCAR_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
								SRIO_PKT_TYPE_MTN_READ,1);

	temp = uiMaintenanceValue;

	if (temp == ((DEVICE_ID1_16BIT<<16) + 0x0030))
	{
		System_printf("Confirmed route back to host through switch, read bdidcsr\n");
	}
	else
	{
		System_printf("READ BACK THROUGH THE SWITCH HAS BEEN DISABLED FOR NOW....\n");
		System_printf("ERROR : Problem with switch routing - read back unexpected didcar!\n");
		System_printf("The DIDCAR is %32x. \n", temp);
	}


	/***************************************************************
	 The remainder of this function is execute for each of the other ports
	 on the CPS1432
	 ****************************************************************/
	System_printf("\nExamine each port on the CPS1432, 'discover' any devices\n");
	errstatus_offset = P0_ERR_STATUS_OFFSET;
	ctl_offset = P0_CTL_OFFSET;
	for (portnum = 0; portnum < numport;)
	{
		System_printf("\n\nExamining port %d  ", portnum);
		portstep = 1;

		/************************************************************
		 STEP 6 : check training on portX
		 Examine the training registers on this CPS1432 port to determine
		 if it has trained to another device.
		 *************************************************************/
		KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
								0xff, errstatus_offset, GLOBAL_ADDR(&uiMaintenanceValue),
								SRIO_PKT_TYPE_MTN_READ,0);
		training= uiMaintenanceValue;
		System_printf("training =0x%08x \n", training);

		if (training & CPS1432_PORTOK_BIT)
		{
			System_printf("Trained successfully \n");
			KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
											0xff, ctl_offset, GLOBAL_ADDR(&uiMaintenanceValue),
											SRIO_PKT_TYPE_MTN_READ,0);
			temp = uiMaintenanceValue;


			// 2015_8_26, set INPUT_PORT_EN = 1
			uiMaintenanceValue = temp | 0x00200000;
			KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
												0xff, ctl_offset, GLOBAL_ADDR(&uiMaintenanceValue),
												SRIO_PKT_TYPE_MTN_WRITE,0);
			System_printf("set INPUT_PORT_EN for port %d \n", portnum);


			temp &= CPS1432_INIT_PWIDTH_MASK;
			if (temp == CPS1432_INIT_PWIDTH_1L0)
			{
				System_printf("Single-lane port\n");
			}
			if (temp == CPS1432_INIT_PWIDTH_1L2)
			{
				System_printf("Single-lane port; lane R (redundancy lane)\n");
			}
			if (temp == CPS1432_INIT_PWIDTH_4)
			{
				System_printf("4x lane port \n");
				portstep = 2;
			}

			if (portnum == swport)
			{
				System_printf("This port number == host port number. Discovered myself! \n");
			}
			else
			{
				/*************************************************************
				 STEP 7: route packets for DeviceID 0xFF to this port
				 	 	 Extended Configuration Enable.
				 **************************************************************/
				uiMaintenanceValue=0xFF;
				uiResult= KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
						RIO_ROUTE_CFG_DESTID, GLOBAL_ADDR(&uiMaintenanceValue),
					    SRIO_PKT_TYPE_MTN_WRITE,0);

				uiMaintenanceValue=portnum;
				uiResult= KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
						RIO_ROUTE_CFG_PORT, GLOBAL_ADDR(&uiMaintenanceValue),
						SRIO_PKT_TYPE_MTN_WRITE,0);
				System_printf("All packets for DeviceID = %04x, now to port %d \n",
						DEVICE_ID2_16BIT, portnum);

				/********************************************
				 STEP 8: Read back the DIDCAR of the device connected
				 to this port.
				 ********************************************/
				System_printf("Examine didcar of device attached to port %d \n",
						portnum);

				KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
							0xff, RIO_DIDCAR_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
							SRIO_PKT_TYPE_MTN_READ,1);
				temp= uiMaintenanceValue;

				if (temp == TMS6678_DIDCAR_VALUE)
				{
					System_printf(" TMS6678 device connected to port %d \n", portnum);
				}
				else
				{
					if (temp == FPGA_DIDCAR_VALUE)  // v7 data sheet
					{
						System_printf("FPGA rapid IO device connect to port %d\n", portnum);
					}
					else
					{
						System_printf("Find device connect to port %d unknown!!!!\n", portnum);
						System_printf("DIDCAR is %08x, while FPGA_DIDCAR_VALUE is 0x0370000E. \n", temp);
//						return -1;
//						goto next;
					}
				}

				if (nextdevid > MAX_DEVICES)
				{
					System_printf("Found too many devices. Cannot allocate deviceid  \n");
					return 1;
				}
				/********************************************
				 STEP 9: Read back the HBDIDL on the device
				 This is the lock register which determines which
				 device can initialize the agent. 0xFFFF indicates
				 that the device has not yet been initialized, if the
				 HBDIDL is some other value, issue warning but continue
				 ********************************************/
				uiResult = KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
									0xff, RIO_HOSTLOCK_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
									SRIO_PKT_TYPE_MTN_READ,1);
				hbdidl = uiMaintenanceValue;
				System_printf("Original HBDIDL = 0x%08x \n", hbdidl);
				if (hbdidl != HBDIDLCSR_NOT_LOCKED)
				{
					System_printf("WARNING : HBDIDL of this device was 0x%08x \n", hbdidl);
				}

				/********************************************
				 STEP 10 : Attempt to lock HBDIDL, write
				 the device ID of this device
				 ********************************************/

				uiMaintenanceValue = (gpSRIO_regs->RIO_BASE_ID&
						CSL_SRIO_RIO_BASE_ID_BASE_ID_MASK) >>
						CSL_SRIO_RIO_BASE_ID_BASE_ID_SHIFT;

				uiResult= KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
						RIO_HOSTLOCK_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
					SRIO_PKT_TYPE_MTN_WRITE,1);

				/********************************************
				 STEP 11: check that the lock has been accepted
				 If the lock has not been accepted, issue warning, but continue
				 (dangerous!!!!)
				 ********************************************/
				uiResult=KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
									0xff, RIO_HOSTLOCK_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
									SRIO_PKT_TYPE_MTN_READ,1);
				hbdidl= uiMaintenanceValue;
				System_printf("Updated HBDIDL = 0x%08x \n", hbdidl);
				if (hbdidl != ((gpSRIO_regs->RIO_BASE_ID&
						CSL_SRIO_RIO_BASE_ID_BASE_ID_MASK) >>
						CSL_SRIO_RIO_BASE_ID_BASE_ID_SHIFT))
				{
					System_printf("WARNING: HBDIDL not updated, Lock not accepted\n");
					System_printf("WARINING : This application continues regardless...");
				}

				/********************************************
				 STEP 12: Update the device ID
				 Regardless of the DeviceID which was read back, we will update
				 it with the device ID we want
				 *********************************************/
				uiResult = KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
									0xff, RIO_BDIDCSR_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
									SRIO_PKT_TYPE_MTN_READ,1);
				bdid= uiMaintenanceValue;
				//System_printf("0x60 bdid was 0x%08x \n",bdid);/* revised by zhouhu 2012.12.02*/
//				bdid = bdid >> SHIFT_BDIDCSR_TO_DEVICEID;
				System_printf("Original deviceID = 0x%08x \n", bdid);

				nextdevid = DEVICE_ID2_16BIT;
				uiMaintenanceValue = nextdevid << SHIFT_BDIDCSR_TO_DEVICEID;
				uiResult = KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
						RIO_BDIDCSR_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
					SRIO_PKT_TYPE_MTN_WRITE,1);

				System_printf("Allocated deviceID = 0x%08x \n", nextdevid);

				/*********************************************
				 STEP 13: Update the routing tables with this new information
				 ***********************************************/
				uiMaintenanceValue = nextdevid;
				uiResult= KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
						RIO_ROUTE_CFG_DESTID, GLOBAL_ADDR(&uiMaintenanceValue),
					SRIO_PKT_TYPE_MTN_WRITE,0);

				uiMaintenanceValue = portnum;
				uiResult= KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0, 0xff,
						RIO_ROUTE_CFG_PORT, GLOBAL_ADDR(&uiMaintenanceValue),
					SRIO_PKT_TYPE_MTN_WRITE,0);

				System_printf("All packets for DeviceID = 0x%04x, now to port %d \n",
						nextdevid, portnum);

				/********************************************
				 STEP 14: Read back the BDIDCAR using the
				 updated device ID
				 ********************************************/
				uiResult = KeyStone_SRIO_Maintenance_hop(uiLocalPort, 0,
						nextdevid, RIO_BDIDCSR_OFFSET, GLOBAL_ADDR(&uiMaintenanceValue),
									SRIO_PKT_TYPE_MTN_READ,1);
				bdid = uiMaintenanceValue;
				if ((bdid >> SHIFT_BDIDCSR_TO_DEVICEID) == nextdevid)
				{
					System_printf("Comfirmed read operation using new device id \n");
				}
				else
				{
					System_printf(
							"ERROR : Unexpected value returned from bdid read = 0x%08x \n",
							bdid);
				}

				/*********************************************
				 STEP 15: update the next deviceID that the host can allocate
				 **************************************************/
				nextdevid++;
			}
		}
		else
		{
			/********************************************
			 If not trained, Report port not trained and fill arrays
			 with values indicating not valid.
			 *********************************************/
			System_printf("This port is not trained \n");
		}
		if (portstep == 2)
		{
			portnum++;
			errstatus_offset += CPS1432_PORT_STRIDE;
			ctl_offset += CPS1432_PORT_STRIDE;
			portnum++;
			errstatus_offset += CPS1432_PORT_STRIDE;
			ctl_offset += CPS1432_PORT_STRIDE;
		}
		else
		{
next:
			portnum++;
			errstatus_offset += CPS1432_PORT_STRIDE;
			ctl_offset += CPS1432_PORT_STRIDE;
		}

	}

	System_printf("\n");

	return SUCCESS;
}




