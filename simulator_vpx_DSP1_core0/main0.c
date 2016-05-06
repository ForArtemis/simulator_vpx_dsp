/*
 * helloWorld_bios6.c
 *
 * TCP/IP Stack 'Hello World!' Example ported to use BIOS6 OS.
 *
 * Copyright (C) 2007, 2011 Texas Instruments Incorporated - http://www.ti.com/
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

//--------------------------------------------------------------------------
// IP Stack 'Hello World!' Example
//
// This is a skeleton application, intended to provide application  
// programmers with a basic Stack setup, to which they can start 
// adding their code.
//
// To test it as is, use with helloWorld.exe from \winapps directory
//

#include <stdio.h>
#include <ti/ndk/inc/netmain.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
/* BIOS6 include */
#include <ti/sysbios/BIOS.h>
/* Platform utilities include */
#include "ti/platform/platform.h"
#include "resource_mgr.h"
/*IPC module Headers */
#include <ti/ipc/Ipc.h>
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/HeapBufMP.h>
#include <ti/ipc/MultiProc.h>
#include <ti/ipc/Notify.h>
/* Timer */
#include <ti/sysbios/hal/Timer.h>
/* IPC register control */
#include<ti/csl/csl_ipcAux.h>
/* Custom header */
#include "../CustomHeaderDsp1.h"

#include <ti/ndk/inc/os/osif.h>

#include <stdlib.h>
#include <ti/ipc/SharedRegion.h>

#include <xdc/runtime/Memory.h>

#include <ti/sysbios/family/c66/tci66xx/CpIntc.h>
#include <ti/sysbios/hal/Hwi.h>
#include <xdc/runtime/Error.h>

#include "hyplnkLLDIFace.h"
#include "hyplnkLLDCfg.h"
#include "hyplnkIsr.h"
#include "hyplnkPlatCfg.h"


//-------------------------------------------------------------------------
//Will be used to send message
MessageQ_QueueId 	QueueIdCore0ToCore1;
MessageQ_QueueId 	QueueIdCore0ToCore2;
MessageQ_QueueId 	QueueIdCore2ToCore0;

MsgCore0ToCore2* 	Msg0To2Ptr;
MsgCore0ToCore1* 	Msg0To1Ptr;
MsgCore2ToCore0* 	Msg2To0Ptr;

HeapBufMP_Handle 	HeapHandleCore0ToCore1;
HeapBufMP_Handle 	HeapHandleCore0ToCore2;
HeapBufMP_Handle 	HeapHandleCore2ToCore0;

MessageQ_Handle 	MessageQCore2ToCore0;

//---------------散射点信息存储位置--------------------------
ScatteringPoint		*ScatteringPointPtr;

//---------------------------------------------------------------------------
// Title String
//
char *VerStr = "\nTCP/IP Stack 'Hello World!' Application\n\n";

// Our NETCTRL callback functions
static void   NetworkOpen();
static void   NetworkClose();
static void   NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd );

// External references
extern int UdpFunc();

#include <ti/ndk/inc/os/osif.h>

IntSetup IntInit;

Timer_Params timerParams;
Timer_Handle ManualModeTimer;
//---------------------------------------------------------------------------
// Configuration
//
char *HostName    = "tidsp";
char *LocalIPAddr = "192.168.1.100";
char *LocalIPMask = "255.255.255.0";    // Not used when using DHCP
char *GatewayIP   = "192.168.1.1";    	// Not used when using DHCP
char *DomainName  = "demo.net";         // Not used when using DHCP
char *DNSServer   = "0.0.0.0";          // Used when set to anything but zero

// Simulator EMAC Switch does not handle ALE_LEARN mode, so please configure the
// MAC address of the PC where you want to launch the webpages and initiate PING to NDK */

Uint8 clientMACAddress [6] = {0x5C, 0x26, 0x0A, 0x69, 0x44, 0x0B}; /* MAC address for my PC */


/*****************************************************************************
 * Cache line size (128 works on any location)
 *****************************************************************************/
#define hyplnk_EXAMPLE_LINE_SIZE      128
/* This is the actual data buffer that will be accessed.
 *
 * When accessing this location directly, the program is playing
 * the role of the remote device (remember we are in loopback).
 *
 * If this is placed in an L2 RAM there are no other dependancies.
 *
 * If this is placed in MSMC or DDR RAM, then the SMS
 * or SES MPAX must be configured to allow the access.
 */
//#pragma DATA_SECTION (HyplinkDataDsp1Dsp2Buffer, ".bss:remoteable");
#pragma DATA_SECTION (HyplinkDataDsp1Dsp2Buffer, "hyperlink");
#pragma DATA_ALIGN (HyplinkDataDsp1Dsp2Buffer, hyplnk_EXAMPLE_LINE_SIZE);
HyplinkDataDsp1Dsp2 HyplinkDataDsp1Dsp2Buffer;
/*
 * This pointer is the local address within the Hyperlink's address
 * space which will access dataBuffer via HyperLink.
 *
 */
HyplinkDataDsp1Dsp2 *bufferThroughHypLnk;


/*************************************************************************
 *  @b EVM_init()
 * 
 *  @n
 *     
 *  Initializes the platform hardware. This routine is configured to start in 
 *   the evm.cfg configuration file. It is the first routine that BIOS 
 *   calls and is executed before Main is called. If you are debugging within
 *  CCS the default option in your target configuration file may be to execute 
 *  all code up until Main as the image loads. To debug this you should disable
 *  that option. 
 *
 *  @param[in]  None
 * 
 *  @retval
 *      None
 ************************************************************************/
void EVM_init()
{
 platform_init_flags   sFlags;
 platform_init_config  sConfig;
 /* Status of the call to initialize the platform */
 int32_t pform_status;

 /* 
  * You can choose what to initialize on the platform by setting the following 
  * flags. Things like the DDR, PLL, etc should have been set by the boot loader.
 */
 memset( (void *) &sFlags,  0, sizeof(platform_init_flags));
 memset( (void *) &sConfig, 0, sizeof(platform_init_config));

    sFlags.pll  = 0; 		/* PLLs for clocking    */
    sFlags.ddr  = 0;     	/* External memory    */
    sFlags.tcsl = 1; 		/* Time stamp counter   */
#ifdef _SCBP6618X_
    sFlags.phy  = 0; /* Ethernet       */
#else
    sFlags.phy  = 1; /* Ethernet       */
#endif
    sFlags.ecc  = 0; /* Memory ECC       */

    sConfig.pllm = 0;  /* Use libraries default clock divisor */

 pform_status = platform_init(&sFlags, &sConfig);

 /* If we initialized the platform okay */
 if (pform_status != Platform_EOK) {
   /* Initialization of the platform failed... die */
   while (1) {
     (void) platform_led(1, PLATFORM_LED_ON, PLATFORM_USER_LED_CLASS);
     (void) platform_delay(50000);
     (void) platform_led(1, PLATFORM_LED_OFF, PLATFORM_USER_LED_CLASS);
     (void) platform_delay(50000);
   }
 }
}


/* Hyperlink ISR */
void HyperlinkIsr()
{
	System_printf("HyperlinkIsr()\n");
}

/* Initialize IPC and messageQ */
void IPC_init()
{
    Int                 status;
    HeapBufMP_Handle    heapHandle;

    //Ipc_start does not Ipc_attach, because 'Ipc.procSync' is set to 'Ipc.ProcSync_PAIR' in *.cfg
    //Ipc reserves some shared memory in SharedRegion zero for synchronization.
    status = Ipc_start();
    if (status < 0)
    {
		System_abort("Ipc_start failed\n");
    }
    /*--------------- Wait IPC attach to other cores ----------------*/
    //!!!! Must attach to the share memory owner - CORE1 firstly
    while ((status = Ipc_attach(PROC_ID_CORE1)) < 0)
	{
    	Task_sleep(1) ;
	}
	while ((status = Ipc_attach(PROC_ID_CORE2)) < 0)
	{
		Task_sleep(1) ;
	}
    System_printf("Ipc_attach() finished.\n");

    /*------------------------- To CORE1 ----------------------*/
	do {
		status = HeapBufMP_open(HEAP_BUF_NAME_CORE0_CORE1, &HeapHandleCore0ToCore1);
		Task_sleep(1) ;
	} while (status < 0);
	System_printf("HeapBufMP_open HEAP_BUF_NAME_CORE0_CORE1 finished. \n");

	do {
		status = MessageQ_open(MSGQ_NAME_CORE0_CORE1, &QueueIdCore0ToCore1);
		Task_sleep(1) ;
	} while (status < 0);
	System_printf("MessageQ_open MSGQ_NAME_CORE0_CORE1 finished. \n");

	status = MessageQ_registerHeap((IHeap_Handle)HeapHandleCore0ToCore1, HEAP_ID_CORE0_CORE1);

	Msg0To1Ptr = (MsgCore0ToCore1*)MessageQ_alloc(HEAP_ID_CORE0_CORE1, sizeof(MsgCore0ToCore1));
	if (Msg0To1Ptr == NULL)
	{
	   System_abort("MessageQ_alloc failed\n" );
	}
	else
	{
		System_printf("MessageQ_alloc(HEAP_ID_CORE0_CORE1, sizeof(MsgCore0ToCore1)) finished. \n");
	}

	/*------------------------- To CORE2 ----------------------*/
	do {
		status = HeapBufMP_open(HEAP_BUF_NAME_CORE0_CORE2, &HeapHandleCore0ToCore2);
		Task_sleep(1) ;
	} while (status < 0);
	System_printf("HeapBufMP_open HEAP_BUF_NAME_CORE0_CORE2 finished. \n");

	do {
		status = MessageQ_open(MSGQ_NAME_CORE0_CORE2, &QueueIdCore0ToCore2);
		Task_sleep(1) ;
	} while (status < 0);
	System_printf("MessageQ_open MSGQ_NAME_CORE0_CORE2 finished. \n");

	MessageQ_registerHeap((IHeap_Handle)HeapHandleCore0ToCore2, HEAP_ID_CORE0_CORE2);

	Msg0To2Ptr= (MsgCore0ToCore2*)MessageQ_alloc(HEAP_ID_CORE0_CORE2, sizeof(MsgCore0ToCore2));
	if (Msg0To2Ptr == NULL)
	{
	   System_abort("MessageQ_alloc failed\n" );
	}
	else
	{
		System_printf("MessageQ_alloc(HEAP_ID_CORE0_CORE2, sizeof(MsgCore0ToCore23456)) finished. \n");
	}

	/* Create the heap that will be used to allocate messages. */
	//---------------For CORE2--------------------------
//	do {
//		status = HeapBufMP_open(HEAP_BUF_NAME_CORE2_CORE0, &heapHandle);
//		Task_sleep(1) ;
//	} while (status < 0);
//	System_printf("HeapBufMP_open(HeapBufMPStringFromCore2, &heapHandle); \n");
	/* Create the local message queue */
	MessageQCore2ToCore0 = MessageQ_create(MSGQ_NAME_CORE2_CORE0, NULL);
	if (MessageQCore2ToCore0 == NULL)
	{
		System_abort("MessageQ_create failed\n");
	}
	System_printf("MessageQ_create(MSGQ_NAME_CORE2_CORE0, NULL); \n");

	//-------获得散射点信息存储空间-----------------
	SharedRegion_SRPtr ScatteringPointSrPtr;	//共享存储空间下的指针
	ScatteringPointPtr = Memory_alloc(SharedRegion_getHeap(1), sizeof(ScatteringPoint), 128, NULL);	//128为对齐字节，可设置与cacheline相同，此处128为临时设置
	if(ScatteringPointPtr == NULL)
	{
		System_printf("SharedRegion_getHeap() failed!!\n");
	}
//	ScatteringPointSrPtr = SharedRegion_getSRPtr((Ptr)ScatteringPointPtr, 1);

	status = Notify_sendEvent(PROC_ID_CORE2, LINE_ID_CORE0_CORE2, EVENT_ID_CORE0_CORE2, (Uint32)ScatteringPointPtr, TRUE);
	if(status < 0)
	{
		System_printf("Notify_sendEvent() failed!!\n");
	}

	System_printf("Debug(Core 0): IPC_init() finished. \n");
}


void HyperlinkInit()
{
	/* Hyperlink interrupt init */
	Int eventId;
	int retVal;
	Hwi_Params params;
	Error_Block eb;

	/* Hyperlink 初始化 */
	/* Hyperlink is not up yet, so we don't have to worry about concurrence */
	memset (&HyplinkDataDsp1Dsp2Buffer, 0, sizeof(HyplinkDataDsp1Dsp2Buffer));

	/* Set up the system PLL, PSC, and DDR as required for this HW */
	System_printf ("About to do system setup (PLL, PSC, and DDR)\n");
	if ((retVal = hyplnkExampleSysSetup()) != hyplnk_RET_OK) {
		System_printf ("system setup failed (%d)\n", (int)retVal);
		exit(1);
	}
	System_printf ("system setup worked\n");

//	/* Hyperlink ISR 初始化 */
////	hyplnkExampleInstallIsr();
//
//	// Initialize the error block
//	Error_init(&eb);
//
//	// Map system interrupt 111 to host interrupt 10 on Intc 0
//	CpIntc_mapSysIntToHostInt(0, CSL_INTC0_VUSR_INT_O, 10);
//
//	// Plug the function and argument for System interrupt 15 then enable it
//	CpIntc_dispatchPlug(CSL_INTC0_VUSR_INT_O, &HyperlinkIsr, 0, TRUE);
//
//	// Enable Host interrupt 10 on Intc 0
//	CpIntc_enableHostInt(0, 10);
//
//	// Get the eventId associated with Host interrupt 10
//	eventId = CpIntc_getEventId(10);
//
//	// Initialize the Hwi parameters
//	Hwi_Params_init(&params);
//
//	// Set the eventId associated with the Host Interrupt
//	params.eventId = eventId;
//
//	// The arg must be set to the Host interrupt
//	params.arg = 10;
//
//	// Enable the interrupt vector
//	params.enableInt = TRUE;
//
//	// Create the Hwi on interrupt 5 then specify 'CpIntc_dispatch'
//	// as the function.
//	Hwi_create(6, &CpIntc_dispatch, &params, &eb);

	/* Enable the peripheral */
	System_printf ("About to set up HyperLink Peripheral\n");
	if ((retVal = hyplnkExamplePeriphSetup()) != hyplnk_RET_OK) {
		System_printf ("HyperLink system setup failed (%d)\n", (int)retVal);
		exit(1);
	}
	System_printf ("HyperLink Peripheral setup worked\n");


	/* Set up address mapsrc */
	if ((retVal = hyplnkExampleAddrMap (&HyplinkDataDsp1Dsp2Buffer, (void **)&bufferThroughHypLnk)) != hyplnk_RET_OK) {
		System_printf ("Address map setup failed (%d)\n", (int)retVal);
		exit(1);
	}

}


/* Timer ISR */
extern void ManualModeTimerIsr();



//---------------------------------------------------------------------
// Main Entry Point
//---------------------------------------------------------------------
int main()
{
	/* Hyperlink初始化 */
	HyperlinkInit();

	EVM_init();

	/* Start the BIOS 6 Scheduler */
	BIOS_start ();
}

//
// Main Thread
//
int MainThread()
{
	int status;
	int             rc;
	HANDLE          hCfg;
	QMSS_CFG_T      qmss_cfg_ndk;
	QMSS_CFG_T      qmss_cfg_srio;
	CPPI_CFG_T      cppi_cfg;



	/* 核间通信初始化 */
	IPC_init();



//	//为发送给DSP2的数据帧分配空间
//	HyplinkDataDsp1ToDsp2* HyplinkDataDsp1ToDsp2Ptr = (HyplinkDataDsp1ToDsp2*)malloc(sizeof(HyplinkDataDsp1ToDsp2));

	/* Initialize the components required to run this application:
	*  (1) QMSS
	*  (2) CPPI
	*  (3) Packet Accelerator
	*/

	/* Initialize QMSS
	* Initialize memregion for both NDK and SRIO
	*/
	//For NDK
	qmss_cfg_ndk.master_core        = 1;
	qmss_cfg_ndk.max_num_desc       = MAX_NUM_DESC;
	qmss_cfg_ndk.desc_size          = MAX_DESC_SIZE;
	qmss_cfg_ndk.mem_region         = Qmss_MemRegion_MEMORY_REGION0;	//Memory region 0 is for NDK
	//For SRIO
	qmss_cfg_srio.max_num_desc       = NUM_HOST_DESC;
	qmss_cfg_srio.desc_size          = SIZE_HOST_DESC;
	qmss_cfg_srio.mem_region         = Qmss_MemRegion_MEMORY_REGION1;	//Memory region 1 is for SRIO
	if (res_mgr_init_qmss (&qmss_cfg_ndk, &qmss_cfg_srio) != 0)
	{
		System_printf ("Failed to initialize the QMSS subsystem \n");
		goto main_exit;
	}
	else
	{
		System_printf ("QMSS successfully initialized \n");
	}

	Qmss_MemRegCfg      memRegionInfo;
	status = Qmss_getMemoryRegionCfg(&memRegionInfo);
	System_printf("Qmss_getMemoryRegionCfg(&memRegionInfo) = %d", status);

	/* Initialize CPPI */
	cppi_cfg.master_core        = 1;
	cppi_cfg.dma_num            = Cppi_CpDma_PASS_CPDMA;
	cppi_cfg.num_tx_queues      = NUM_PA_TX_QUEUES;
	cppi_cfg.num_rx_channels    = NUM_PA_RX_CHANNELS;
	if (res_mgr_init_cppi (&cppi_cfg) != 0)
	{
		System_printf ("Failed to initialize CPPI subsystem \n");
		goto main_exit;
	}
	else
	{
		System_printf ("CPPI successfully initialized \n");
	}

	/* Initialize Packet Accelerator */
	if (res_mgr_init_pass()!= 0) {
		System_printf ("Failed to initialize the Packet Accelerator \n");
		goto main_exit;
	}
	else
	{
		System_printf ("PA successfully initialized \n");
	}


	//Tell core1 qmss and cppi are initialized
//	CSL_IPC_genHostInterrupt(IPCGRH_SRC_ID);
	status = Notify_sendEvent(PROC_ID_CORE1, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, QMSS_RDY_NOTIFY, TRUE);
	if(status < 0)
		System_abort("Notify_sendEvent failed.\n");
	System_printf("Notify_sendEvent(QMSS_RDY_NOTIFY) finished.\n");




	//Wait for CORE0 Srio_start() finished

	/* Initialize IPC and MessageQ */
	//NDK start should be excuted after SRIO start.
//    IPC_init();

    // THIS MUST BE THE ABSOLUTE FIRST THING DONE IN AN APPLICATION!!
    rc = NC_SystemOpen( NC_PRIORITY_LOW, NC_OPMODE_INTERRUPT );
    if( rc )
    {
        System_printf("NC_SystemOpen Failed (%d)\n",rc);
        for(;;);
    }
    // Print out our banner
    System_printf(VerStr);

    //
    // Create and build the system configuration from scratch.
    //

    // Create a new configuration
    hCfg = CfgNew();
    if( !hCfg )
    {
        System_printf("Unable to create configuration\n");
        goto main_exit;
    }

    //validate the length of the supplied names
    if( strlen( DomainName ) >= CFG_DOMAIN_MAX ||
        strlen( HostName ) >= CFG_HOSTNAME_MAX )
    {
        System_printf("Names too long\n");
        goto main_exit;
    }

    // Add our global hostname to hCfg (to be claimed in all connected domains)
    CfgAddEntry( hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_HOSTNAME, 0,
                 strlen(HostName), (UINT8 *)HostName, 0 );

    //manually configure IP and Gateway
	CI_IPNET NA;
	CI_ROUTE RT;

	// Setup manual IP address
	bzero( &NA, sizeof(NA) );
	NA.IPAddr  = inet_addr(LocalIPAddr);
	NA.IPMask  = inet_addr(LocalIPMask);
	strcpy( NA.Domain, DomainName );
	NA.NetType = 0;
	// Add the address to interface 1
	CfgAddEntry( hCfg, CFGTAG_IPNET, 1, 0,
					   sizeof(CI_IPNET), (UINT8 *)&NA, 0 );

	// Add the default gateway. Since it is the default, the
	// destination address and mask are both zero (we go ahead
	// and show the assignment for clarity).
	bzero( &RT, sizeof(RT) );
	RT.IPDestAddr = 0;
	RT.IPDestMask = 0;
	RT.IPGateAddr = inet_addr(GatewayIP);
	// Add the route
	CfgAddEntry( hCfg, CFGTAG_ROUTE, 0, 0,
					   sizeof(CI_ROUTE), (UINT8 *)&RT, 0 );

    //
    // Configure IPStack/OS Options
    //

    // We don't want to see debug messages less than WARNINGS
    rc = DBG_WARN;
    CfgAddEntry( hCfg, CFGTAG_OS, CFGITEM_OS_DBGPRINTLEVEL,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    //
    // This code sets up the TCP and UDP buffer sizes
    // (Note 8192 is actually the default. This code is here to
    // illustrate how the buffer and limit sizes are configured.)
    //

    // UDP Receive limit
    rc = 8192;
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKUDPRXLIMIT,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    //
    // Boot the system using this configuration
    //
    // We keep booting until the function returns 0. This allows
    // us to have a "reboot" command.
    //
//    do
//    {
//    	//the function will block followed program
//        rc = NC_NetStart( hCfg, NetworkOpen, NetworkClose, NetworkIPAddr );
//    } while( rc > 0 );

    rc = NC_NetStart( hCfg, NetworkOpen, NetworkClose, NetworkIPAddr );

    // Delete Configuration
    CfgFree( hCfg );

    // Close the OS
main_exit:
    NC_SystemClose();
    return(0);
}


//
// System Task Code [ Server Daemon Servers ]
//
static HANDLE hHello=0;

//
// NetworkOpen
//
// This function is called after the configuration has booted
//
static void NetworkOpen()
{
    // Create our local server
	//first 0 means any IP address, 45454 is port num
	hHello = DaemonNew( SOCK_DGRAM, 0, 45454, UdpFunc,
					   OS_TASKPRINORM, OS_TASKSTKNORM, 0, 1 );
}

//
// NetworkClose
//
// This function is called when the network is shutting down,
// or when it no longer has any IP addresses assigned to it.
//
static void NetworkClose()
{
    DaemonFree( hHello );
}


//
// NetworkIPAddr
//
// This function is called whenever an IP address binding is
// added or removed from the system.
//
static void NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd )
{
    IPN IPTmp;

    if( fAdd )
        System_printf("Network Added: ");
    else
        System_printf("Network Removed: ");

    // Print a message
    IPTmp = ntohl( IPAddr );
    System_printf("If-%d:%d.%d.%d.%d\n", IfIdx,
            (UINT8)(IPTmp>>24)&0xFF, (UINT8)(IPTmp>>16)&0xFF,
            (UINT8)(IPTmp>>8)&0xFF, (UINT8)IPTmp&0xFF );
}


