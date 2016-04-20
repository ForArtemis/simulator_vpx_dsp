/**
 *   @file  loopbackDioIsr.c
 *
 *   @brief   
 *      This is an example application which shows how DIO transfer
 *      completion interrupts can be registered and serviced.
 *
 *  \par
 *  NOTE:
 *      (C) Copyright 2010-2012 Texas Instruments, Inc.
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
 *  \par
*/
#include <xdc/std.h>
#include <string.h>
#include <c6x.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h> 
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/heaps/HeapBuf.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/family/c64p/Hwi.h>
#include <ti/sysbios/family/c64p/EventCombiner.h> 
#include <ti/sysbios/family/c66/tci66xx/CpIntc.h>

/* IPC includes */ 
#include <ti/ipc/GateMP.h>
#include <ti/ipc/Ipc.h>
#include <ti/ipc/ListMP.h>
#include <ti/ipc/SharedRegion.h>
#include <ti/ipc/MultiProc.h>

#include <xdc/cfg/global.h>

/* SRIO Driver Include File. */
#include <ti/drv/srio/srio_drv.h>
#include <ti/drv/srio/srio_osal.h>

/* CPPI/QMSS Include Files. */
#include <ti/drv/cppi/cppi_drv.h>
#include <ti/drv/qmss/qmss_drv.h>
#include <ti/drv/qmss/qmss_firmware.h>

/* CSL Chip Functional Layer */
#include <ti/csl/csl_chip.h>

/* CSL Cache Functional Layer */
#include <ti/csl/csl_cacheAux.h>

/* PSC CSL Include Files */
#include <ti/csl/csl_psc.h>
#include <ti/csl/csl_pscAux.h>

/* CSL SRIO Functional Layer */
#include <ti/csl/csl_srio.h>
#include <ti/csl/csl_srioAux.h>

/* CSL CPINTC Include Files. */
#include<ti/csl/csl_cpIntc.h>

/*IPC module Headers */
#include <ti/ipc/Ipc.h>
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/HeapBufMP.h>
#include <ti/ipc/MultiProc.h>
/* Custom header */
#include "../CustomHeader.h"
/* IPC register control */
#include<ti/csl/csl_ipcAux.h>
#include <ti/ipc/Notify.h>

#include<stdlib.h>

/**********************************************************************
 ************************** LOCAL Definitions *************************
 **********************************************************************/
/* !!!! NUM_HOST_DESC and SIZE_HOST_DESC also defined in core 0 */
/* This is the Number of host descriptors which are available & configured
 * in the memory region for this example. */
#define NUM_HOST_DESC               128

/* This is the size of each descriptor. */
#define SIZE_HOST_DESC              48

/* MTU of the SRIO Driver. We are currently operating @ MTU of 256 bytes. */
#define SRIO_MAX_MTU				256
//#define SRIO_MAX_MTU				512

/* This is the size of the data buffer which is used for DIO Sockets. */
#define SIZE_DIO_PACKET				128

/* Defines the core number responsible for system initialization. */
#define CORE_SYS_INIT               0

/* Defines number of DIO sockets used in this example */
#define SRIO_DIO_LSU_ISR_NUM_SOCKETS        1

/* Defines number of transfers done by DIO sockets in this example */
#define SRIO_DIO_LSU_ISR_NUM_TRANSFERS      3

/* ISR timeout value (in cycles) used in this example */
//#define SRIO_DIO_LSU_ISR_TIMEOUT        100000
#define SRIO_DIO_LSU_ISR_TIMEOUT        1000000


/**********************************************************************
 ************************** Global Variables **************************
 **********************************************************************/

/* Memory allocated for the descriptors. This is 16 bit aligned. */
#pragma DATA_ALIGN (host_region, 16)
Uint8   host_region[NUM_HOST_DESC * SIZE_HOST_DESC];

/* Memory used for the accumulator list. */
#pragma DATA_ALIGN (gHiPriAccumList, 16)
UInt32              gHiPriAccumList[64];

/* Global SRIO and QMSS Configuration */
Qmss_InitCfg   qmssInitConfig;

/* Global Varialble which keeps track of the core number executing the
 * application. */
UInt32          coreNum = 0xFFFF;

/* Shared Memory Variable to ensure synchronizing SRIO initialization
 * with all the other cores. */
/* Created an array to pad the cache line with SRIO_MAX_CACHE_ALIGN size */
#pragma DATA_ALIGN   (isSRIOInitialized, 128)
#pragma DATA_SECTION (isSRIOInitialized, ".srioSharedMem");
volatile Uint32     isSRIOInitialized[(SRIO_MAX_CACHE_ALIGN / sizeof(Uint32))] = { 0 };

Srio_DrvHandle  hDrvManagedSrioDrv;

CSL_SrioHandle  hSrioCSL;

/* These are the device identifiers used in the Example Application */
const uint32_t DEVICE_ID1_16BIT    = 0x0001;
const uint32_t DEVICE_ID1_8BIT     = 0x01;
const uint32_t DEVICE_ID2_16BIT    = 0x0002;
const uint32_t DEVICE_ID2_8BIT     = 0x02;
const uint32_t DEVICE_ID3_16BIT    = 0x0003;
const uint32_t DEVICE_ID3_8BIT     = 0x03;
const uint32_t DEVICE_ID4_16BIT    = 0x0004;
const uint32_t DEVICE_ID4_8BIT     = 0x04;

/* Array containing SRIO socket handles */
Srio_SockHandle srioSocket[SRIO_DIO_LSU_ISR_NUM_SOCKETS];
Srio_SockHandle srioSocketDoorbellTest;

/* Source and Destination Data Buffers (payload buffers) */
UInt8* srcDataBuffer[SRIO_DIO_LSU_ISR_NUM_SOCKETS * SRIO_DIO_LSU_ISR_NUM_TRANSFERS];
UInt8* dstDataBuffer[SRIO_DIO_LSU_ISR_NUM_SOCKETS * SRIO_DIO_LSU_ISR_NUM_TRANSFERS];

/* Global debug variable to track number of ISRs raised */
volatile UInt32 srioDbgDioIsrCnt = 0;

/* Global variable to indicate completion of ISR processing */
volatile UInt32 srioLsuIsrServiced = 0;

/* Global variables to count good and bad transfers */
volatile UInt32 srioDioLsuGoodTransfers = 0;
volatile UInt32 srioDioLsuBadTransfers  = 0;


/* MessageQ handle */
MessageQ_Handle  	MessageQCore0ToCore1;
MessageQ_Handle  	MessageQCore2ToCore1;
MessageQ_Handle  	MessageQCore3ToCore1;
MessageQ_Handle  	MessageQCore4ToCore1;
MessageQ_Handle  	MessageQCore5ToCore1;
MessageQ_Handle  	MessageQCore6ToCore1;
MessageQ_Handle  	MessageQCore7ToCore1;
MessageQ_Handle  	MessageQCore34567ToCore1;

/* MessageQ */
MsgCore0ToCore1		*Msg0To1Ptr;
MsgCore2ToCore1		*Msg2To1Ptr;
MsgCore34567ToCore1	*Msg3To1Ptr;
MsgCore34567ToCore1	*Msg4To1Ptr;
MsgCore34567ToCore1	*Msg5To1Ptr;
MsgCore34567ToCore1	*Msg6To1Ptr;
MsgCore34567ToCore1	*Msg7To1Ptr;

/* SRIO socket handles */
Srio_SockHandle SrioSocket;

UInt32 NotifyCore0Payload;





/**********************************************************************
 ************************* Extern Definitions *************************
 **********************************************************************/
extern UInt32 malloc_counter;
extern UInt32 free_counter;

extern int32_t SrioDevice_init (void);

/* QMSS device specific configuration */
extern Qmss_GlobalConfigParams  qmssGblCfgParams;

/* CPPI device specific configuration */
extern Cppi_GlobalConfigParams  cppiGblCfgParams;

/* OSAL Data Buffer Memory Initialization. */
extern int32_t Osal_dataBufferInitMemory(uint32_t dataBufferSize);

/* Enable Time Stamp Counter */
extern void CSL_tscEnable(void);



/* Call back function of notify from CORE0 */
void NotifyCore0CbFxn(UInt16 procId, UInt16 lineId, UInt32 eventId, UArg arg, UInt32 payload)
{
	NotifyCore0Payload = payload;
	if(payload == QMSS_RDY_NOTIFY)
	{
		Semaphore_post(QmssRdySemHandle);
	}
	else if((payload == SELF_CHECKING_NOTIFY)||
			(payload == WORK_ENABLE_NOTIFY)||
			(payload == SLEEP_ENABLE_NOTIFY)||
			(payload == WORK_PARAM_SET_NOTIFY))
	{
		Semaphore_post(ManualMsgRdyHandle);
	}
}

/* Find the host connected device */
extern int rio_discovery(Uint32 uiLocalPort);



/* Initialize IPC, messageQ and Notify*/
void IPC_init()
{
    Int status;
	HeapBufMP_Handle              heapHandle;
	HeapBufMP_Params              heapBufParams;


    //Ipc_start does not Ipc_attach, because 'Ipc.procSync' is set to 'Ipc.ProcSync_PAIR' in *.cfg
    //Ipc reserves some shared memory in SharedRegion zero for synchronization.
    status = Ipc_start();
    if (status < 0)
	{
    	System_abort("Ipc_start failed\n");
	}
    //wait IPC attach to core 0, 2~7
    while ((status = Ipc_attach(PROC_ID_CORE0)) < 0)
	{
		Task_sleep(1) ;
	}
	while ((status = Ipc_attach(PROC_ID_CORE2)) < 0)
	{
		Task_sleep(1) ;
	}
	while ((status = Ipc_attach(PROC_ID_CORE3)) < 0)
	{
		Task_sleep(1) ;
	}
	while ((status = Ipc_attach(PROC_ID_CORE4)) < 0)
	{
		Task_sleep(1) ;
	}
	while ((status = Ipc_attach(PROC_ID_CORE5)) < 0)
	{
		Task_sleep(1) ;
	}
	while ((status = Ipc_attach(PROC_ID_CORE6)) < 0)
	{
		Task_sleep(1) ;
	}
	while ((status = Ipc_attach(PROC_ID_CORE7)) < 0)
	{
		Task_sleep(1) ;
	}
    System_printf("Ipc_attach() finished.\n");

    //Create a notify for CORE0
    status = Notify_registerEvent(PROC_ID_CORE0, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, NotifyCore0CbFxn, 0x1010);
	if (status < 0)
	{
		System_abort("Notify_registerEvent failed\n");
	}


	/* Create the heap that will be used to allocate messages. */
    //---------------For CORE0 to CORE1--------------------------
	HeapBufMP_Params_init(&heapBufParams);
	heapBufParams.regionId       = 0;
	heapBufParams.name           = HEAP_BUF_NAME_CORE0_CORE1;
	heapBufParams.numBlocks      = 1;
	heapBufParams.blockSize      = sizeof(MsgCore0ToCore1);
	heapHandle = HeapBufMP_create(&heapBufParams);
	if (heapHandle == NULL)
	{
		System_abort("HeapBufMP_create failed\n" );
	}
	else
	{
		System_printf("HeapBufMP_create for CORE0 finished. \n");
	}
	/* Register this heap with MessageQ */
	status = MessageQ_registerHeap((IHeap_Handle)heapHandle, HEAP_ID_CORE0_CORE1);
	/* Create the local message queue */
	MessageQCore0ToCore1 = MessageQ_create(MSGQ_NAME_CORE0_CORE1, NULL);
	if (MessageQCore0ToCore1 == NULL)
	{
		System_abort("MessageQ_create0 failed\n");
	}
	else
	{
		System_printf("MessageQ_create for CORE0 finished. \n");
	}
    //---------------For CORE2 to CORE1--------------------------
	HeapBufMP_Params_init(&heapBufParams);
	heapBufParams.regionId       = 0;
	heapBufParams.name           = HEAP_BUF_NAME_CORE2_CORE1;
	heapBufParams.numBlocks      = 1;
	heapBufParams.blockSize      = sizeof(MsgCore2ToCore1);
	heapHandle = HeapBufMP_create(&heapBufParams);
	if (heapHandle == NULL)
	{
		System_abort("HeapBufMP_create failed\n" );
	}
	else
	{
		System_printf("HeapBufMP_create for CORE2 finished. \n");
	}
	/* Register this heap with MessageQ */
	status = MessageQ_registerHeap((IHeap_Handle)heapHandle, HEAP_ID_CORE2_CORE1);
	/* Create the local message queue */
	MessageQCore2ToCore1 = MessageQ_create(MSGQ_NAME_CORE2_CORE1, NULL);
	if (MessageQCore2ToCore1 == NULL)
	{
		System_abort("MessageQ_create2 failed\n");
	}
	else
	{
		System_printf("MessageQ_create for CORE2 finished. \n");
	}
	//---------------For CORE3~7 to CORE1--------------------------
	HeapBufMP_Params_init(&heapBufParams);
	heapBufParams.regionId       = 0;
	heapBufParams.name           = HEAP_BUF_NAME_CORE34567_CORE1;
	heapBufParams.numBlocks      = 5;	//5 cores
	heapBufParams.blockSize      = sizeof(MsgCore34567ToCore1);
	heapHandle = HeapBufMP_create(&heapBufParams);
	if (heapHandle == NULL)
	{
		System_abort("HeapBufMP_create failed\n" );
	}
	/* Register this heap with MessageQ */
	status = MessageQ_registerHeap((IHeap_Handle)heapHandle, HEAP_ID_CORE34567_CORE1);
	if(status < 0)
		System_abort("??????????");
	/* Create the local message queue */
	MessageQCore34567ToCore1 = MessageQ_create(MSGQ_NAME_CORE34567_CORE1, NULL);
	if (MessageQCore34567ToCore1 == NULL)
	{
		System_abort("MessageQ_create34567 failed\n");
	}

//	//--------------- For CORE0 to CORE1 --------------------------
//	HeapBufMP_Params_init(&heapBufParams);
//	heapBufParams.regionId       = 0;
//	heapBufParams.name           = HEAP_BUF_NAME_CORE0_CORE1;
//	heapBufParams.numBlocks      = 1;
//	heapBufParams.blockSize      = sizeof(MsgCore0ToCore1);
//	heapHandle = HeapBufMP_create(&heapBufParams);
//	if (heapHandle == NULL)
//	{
//		System_abort("HeapBufMP_create failed\n" );
//	}

	//--------------- For CORE0 to CORE2 --------------------------
	HeapBufMP_Params_init(&heapBufParams);
	heapBufParams.regionId       = 0;
	heapBufParams.name           = HEAP_BUF_NAME_CORE0_CORE2;
	heapBufParams.numBlocks      = 1;
	heapBufParams.blockSize      = sizeof(MsgCore0ToCore2);
	heapHandle = HeapBufMP_create(&heapBufParams);
	if (heapHandle == NULL)
	{
		System_abort("HeapBufMP_create failed\n" );
	}

	//--------------- For CORE2 to CORE0, CORE3~7 --------------------------
	//For CORE2 to CORE0
	HeapBufMP_Params_init(&heapBufParams);
	heapBufParams.regionId       = 0;
	heapBufParams.name           = HEAP_BUF_NAME_CORE2_CORE0;
	heapBufParams.numBlocks      = 1;
	heapBufParams.blockSize      = sizeof(MsgCore2ToCore0);
	heapHandle = HeapBufMP_create(&heapBufParams);
	if (heapHandle == NULL)
	{
		System_abort("HeapBufMP_create failed\n" );
	}
	//For CORE2 to CORE34567
	HeapBufMP_Params_init(&heapBufParams);
	heapBufParams.regionId       = 0;
	heapBufParams.name           = HEAP_BUF_NAME_CORE2_CORE34567;
	heapBufParams.numBlocks      = 5;
	heapBufParams.blockSize      = sizeof(MsgCore2ToCore34567);
	heapHandle = HeapBufMP_create(&heapBufParams);
	if (heapHandle == NULL)
	{
		System_abort("HeapBufMP_create failed\n" );
	}


//	//For CORE2 to CORE3
//	HeapBufMP_Params_init(&heapBufParams);
//	heapBufParams.regionId       = 0;
//	heapBufParams.name           = HEAP_BUF_NAME_CORE2_CORE3;
//	heapBufParams.numBlocks      = 1;
//	heapBufParams.blockSize      = sizeof(MsgCore2ToCore34567);
//	heapHandle = HeapBufMP_create(&heapBufParams);
//	if (heapHandle == NULL)
//	{
//		System_abort("HeapBufMP_create failed\n" );
//	}
//	//For CORE2 to CORE4
//	HeapBufMP_Params_init(&heapBufParams);
//	heapBufParams.regionId       = 0;
//	heapBufParams.name           = HEAP_BUF_NAME_CORE2_CORE4;
//	heapBufParams.numBlocks      = 1;
//	heapBufParams.blockSize      = sizeof(MsgCore2ToCore34567);
//	heapHandle = HeapBufMP_create(&heapBufParams);
//	if (heapHandle == NULL)
//	{
//		System_abort("HeapBufMP_create failed\n" );
//	}
//	//For CORE2 to CORE5
//	HeapBufMP_Params_init(&heapBufParams);
//	heapBufParams.regionId       = 0;
//	heapBufParams.name           = HEAP_BUF_NAME_CORE2_CORE5;
//	heapBufParams.numBlocks      = 1;
//	heapBufParams.blockSize      = sizeof(MsgCore2ToCore34567);
//	heapHandle = HeapBufMP_create(&heapBufParams);
//	if (heapHandle == NULL)
//	{
//		System_abort("HeapBufMP_create failed\n" );
//	}
//	//For CORE2 to CORE6
//	HeapBufMP_Params_init(&heapBufParams);
//	heapBufParams.regionId       = 0;
//	heapBufParams.name           = HEAP_BUF_NAME_CORE2_CORE6;
//	heapBufParams.numBlocks      = 1;
//	heapBufParams.blockSize      = sizeof(MsgCore2ToCore34567);
//	heapHandle = HeapBufMP_create(&heapBufParams);
//	if (heapHandle == NULL)
//	{
//		System_abort("HeapBufMP_create failed\n" );
//	}
//	//For CORE2 to CORE7
//	HeapBufMP_Params_init(&heapBufParams);
//	heapBufParams.regionId       = 0;
//	heapBufParams.name           = HEAP_BUF_NAME_CORE2_CORE7;
//	heapBufParams.numBlocks      = 1;
//	heapBufParams.blockSize      = sizeof(MsgCore2ToCore34567);
//	heapHandle = HeapBufMP_create(&heapBufParams);
//	if (heapHandle == NULL)
//	{
//		System_abort("HeapBufMP_create failed\n" );
//	}


	System_printf("Debug(Core 1): IPC_init() finished. \n");
}



/**********************************************************************
 ************************ SRIO EXAMPLE FUNCTIONS **********************
 **********************************************************************/

/**
 *  @b Description
 *  @n  
 *      Utility function which converts a local address to global.
 *
 *  @param[in]  addr
 *      Local address to be converted
 *
 *  @retval
 *      Global Address
 */
static UInt32 l2_global_address (Uint32 addr)
{
	UInt32 corenum;

	/* Get the core number. */
	corenum = CSL_chipReadReg(CSL_CHIP_DNUM); 

	/* Compute the global address. */
	return (addr + (0x10000000 + (corenum*0x1000000)));
}

/**
 *  @b Description
 *  @n  
 *      Utility function that is required by the IPC module to set the proc Id.
 *      The proc Id is set via this function instead of hard coding it in the .cfg file
 *
 *  @retval
 *      Not Applicable.
 */
Void myStartupFxn (Void)
{
	MultiProc_setLocalId (CSL_chipReadReg (CSL_CHIP_DNUM));
}

/**
 *  @b Description
 *  @n  
 *      This function enables the power/clock domains for SRIO. 
 *
 *  @retval
 *      Not Applicable.
 */
static Int32 enable_srio (void)
{
#ifndef SIMULATOR_SUPPORT
    /* SRIO power domain is turned OFF by default. It needs to be turned on before doing any 
     * SRIO device register access. This not required for the simulator. */

    /* Set SRIO Power domain to ON */        
    CSL_PSC_enablePowerDomain (CSL_PSC_PD_SRIO);

    /* Enable the clocks too for SRIO */
    CSL_PSC_setModuleNextState (CSL_PSC_LPSC_SRIO, PSC_MODSTATE_ENABLE);

    /* Start the state transition */
    CSL_PSC_startStateTransition (CSL_PSC_PD_SRIO);

    /* Wait until the state transition process is completed. */
    while (!CSL_PSC_isStateTransitionDone (CSL_PSC_PD_SRIO));

    /* Return SRIO PSC status */
    if ((CSL_PSC_getPowerDomainState(CSL_PSC_PD_SRIO) == PSC_PDSTATE_ON) &&
        (CSL_PSC_getModuleState (CSL_PSC_LPSC_SRIO) == PSC_MODSTATE_ENABLE))
    {
        /* SRIO ON. Ready for use */            
        return 0;
    }
    else
    {
        /* SRIO Power on failed. Return error */            
        return -1;            
    }
#else
    /* PSC is not supported on simulator. Return success always */
    return 0;
#endif
}

/**
 *  @b Description
 *  @n  
 *      This function is application registered SRIO DIO LSU interrupt 
 *      handler (ISR) which is used to process the pending DIO Interrupts. 
 *      SRIO Driver users need to ensure that this ISR is plugged with 
 *      their OS Interrupt Management API. The function expects the 
 *      Interrupt Destination information to be passed along to the 
 *      API because the DIO interrupt destination mapping is configurable 
 *      during SRIO device initialization. 
 *
 *  @param[in]  argument
 *      SRIO Driver Handle
 *
 *  @retval
 *      Not Applicable
 */
static void myDioTxCompletionIsr 
(
    UArg argument
)
{
    /* Pass the control to the driver DIO Tx Completion ISR handler */
    Srio_dioTxCompletionIsr ((Srio_DrvHandle)argument, hSrioCSL);

    /* Wake up the pending task */
    srioLsuIsrServiced = 1;

    /* Debug: Increment the ISR count */
    srioDbgDioIsrCnt++;

    return;
}

void myDioDoorbellIsr
(
    UArg argument
)
{
	uint8_t intDstDoorbell[4];
    intDstDoorbell[0] = 0x0;
    intDstDoorbell[1] = 0x1;
    intDstDoorbell[2] = 0x2;
    intDstDoorbell[3] = 0x3;
	Srio_dioCompletionIsr((Srio_DrvHandle)argument, intDstDoorbell);
	System_printf("Door bell ISR entered!!!\n");
}

/********************************************************************************
    * The SRIO Driver Instance is going to be created with the following properties:
    * - Driver Managed
    * - Interrupt Support (Pass the Rx Completion Queue as NULL)
    ********************************************************************************/
void SrioDrvStart()
{
	UInt8           isAllocated;
	Srio_DrvConfig  drvCfg;

	/* Initialize the SRIO Driver Configuration. */
	memset ((Void *)&drvCfg, 0, sizeof(Srio_DrvConfig));

   /* Setup the SRIO Driver Managed Configuration. */
   drvCfg.bAppManagedConfig = FALSE;

   /* Driver Managed: Receive Configuration */
   drvCfg.u.drvManagedCfg.bIsRxCfgValid             = 1;
   drvCfg.u.drvManagedCfg.rxCfg.rxMemRegion         = Qmss_MemRegion_MEMORY_REGION1;	//!!!! The memregion is configured in core0
   drvCfg.u.drvManagedCfg.rxCfg.numRxBuffers        = 4;
   drvCfg.u.drvManagedCfg.rxCfg.rxMTU               = SRIO_MAX_MTU;

   /* Accumulator Configuration. */
   {
	    int32_t coreToQueueSelector[4];

     /* This is the table which maps the core to a specific receive queue. */
	    coreToQueueSelector[0] = 704;
	    coreToQueueSelector[1] = 705;
	    coreToQueueSelector[2] = 706;
	    coreToQueueSelector[3] = 707;

	    /* Since we are programming the accumulator we want this queue to be a HIGH PRIORITY Queue */
	    drvCfg.u.drvManagedCfg.rxCfg.rxCompletionQueue = Qmss_queueOpen (Qmss_QueueType_HIGH_PRIORITY_QUEUE,
	    															     coreToQueueSelector[coreNum], &isAllocated);
		if (drvCfg.u.drvManagedCfg.rxCfg.rxCompletionQueue < 0)
		{
			System_printf ("Error: Unable to open the SRIO Receive Completion Queue\n");
			return;
		}

		/* Accumulator Configuration is VALID. */
		drvCfg.u.drvManagedCfg.rxCfg.bIsAccumlatorCfgValid = 1;

		/* Accumulator Configuration. */
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.channel             = coreNum;
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.command             = Qmss_AccCmd_ENABLE_CHANNEL;
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.queueEnMask         = 0;
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.queMgrIndex         = coreToQueueSelector[coreNum];
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.maxPageEntries      = 2;
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.timerLoadCount      = 0;
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.interruptPacingMode = Qmss_AccPacingMode_LAST_INTERRUPT;
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.listEntrySize       = Qmss_AccEntrySize_REG_D;
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.listCountMode       = Qmss_AccCountMode_ENTRY_COUNT;
	    drvCfg.u.drvManagedCfg.rxCfg.accCfg.multiQueueMode      = Qmss_AccQueueMode_SINGLE_QUEUE;

       /* Initialize the accumulator list memory */
       memset ((Void *)&gHiPriAccumList[0], 0, sizeof(gHiPriAccumList));
       drvCfg.u.drvManagedCfg.rxCfg.accCfg.listAddress = l2_global_address((UInt32)&gHiPriAccumList[0]);
   }

   /* Driver Managed: Transmit Configuration */
   drvCfg.u.drvManagedCfg.bIsTxCfgValid             = 1;
   drvCfg.u.drvManagedCfg.txCfg.txMemRegion         = Qmss_MemRegion_MEMORY_REGION1;	//!!!! The memregion is configured in core0
   drvCfg.u.drvManagedCfg.txCfg.numTxBuffers        = 4;
   drvCfg.u.drvManagedCfg.txCfg.txMTU               = SRIO_MAX_MTU;

   /* Start the Driver Managed SRIO Driver. */
   hDrvManagedSrioDrv = Srio_start(&drvCfg);
   if (hDrvManagedSrioDrv == NULL)
   {
       System_abort ("Error: SRIO Driver failed to start\n");
   }
   else
   {
	   System_printf ("SrioDrvStart() finished. \n");
   }
}


/**
 *  @b Description
 *  @n  
 *      This is the main DIO Example Task 
 *
 *  @retval
 *      Not Applicable.
 */
//static Void MainThread(UArg arg0, UArg arg1)
Void MainThread(void)
{

	int 					status;
	int						i;
	Srio_SockBindAddrInfo   SrioSockBindInfo;
	Srio_SockAddrInfo       *SrioSockInfo;
	SrioFrame0 				*SrioFrame0SendPtr;
	SrioFrame1				*SrioFrame1SendPtr;
	SrioFrame2				*SrioFrame2SendPtr;

	SrioSockInfo = (Srio_SockAddrInfo*)malloc(sizeof(Srio_SockAddrInfo));
	System_printf("SrioSockInfo malloc size is %d.\n", sizeof(Srio_SockAddrInfo));
	if(!SrioSockInfo)//如果malloc失败
	{
		System_printf("SrioSockInfo malloc failed./n");
	}
	SrioFrame0SendPtr = (SrioFrame0*)malloc(sizeof(SrioFrame0));
	System_printf("SrioFrame0 malloc size is %d.\n", sizeof(SrioFrame0));
	if(!SrioFrame0SendPtr)//如果malloc失败
	{
		System_printf("SrioFrame0 malloc failed./n");
	}
	SrioFrame1SendPtr = (SrioFrame1*)malloc(sizeof(SrioFrame1));
	System_printf("SrioFrame1 malloc size is %d.\n", sizeof(SrioFrame1));
	if(!SrioFrame1SendPtr)//如果malloc失败
	{
		System_printf("SrioFrame1 malloc failed./n");
	}
	SrioFrame2SendPtr = (SrioFrame2*)malloc(sizeof(SrioFrame2));
	System_printf("SrioFrame2 malloc size is %d.\n", sizeof(SrioFrame2));
	if(!SrioFrame2SendPtr)//如果malloc失败
	{
		System_printf("SrioFrame2 malloc failed./n");
	}
	Msg2To1Ptr = (MsgCore2ToCore1*)malloc(sizeof(MsgCore2ToCore1));
	System_printf("MsgCore2ToCore1 malloc size is %d.\n", sizeof(MsgCore2ToCore1));
	if(!Msg2To1Ptr)//如果malloc失败
	{
		System_printf("MsgCore2ToCore1 malloc failed./n");
	}
	Msg3To1Ptr = (MsgCore34567ToCore1*)malloc(sizeof(MsgCore34567ToCore1));
	System_printf("MsgCore34567ToCore1 malloc size is %d.\n", sizeof(MsgCore34567ToCore1));
	if(!Msg3To1Ptr)//如果malloc失败
	{
		System_printf("MsgCore34567ToCore1 malloc failed./n");
	}
	Msg4To1Ptr = (MsgCore34567ToCore1*)malloc(sizeof(MsgCore34567ToCore1));
	System_printf("MsgCore34567ToCore1 malloc size is %d.\n", sizeof(MsgCore34567ToCore1));
	if(!Msg4To1Ptr)//如果malloc失败
	{
		System_printf("MsgCore34567ToCore1 malloc failed./n");
	}
	Msg5To1Ptr = (MsgCore34567ToCore1*)malloc(sizeof(MsgCore34567ToCore1));
	System_printf("MsgCore34567ToCore1 malloc size is %d.\n", sizeof(MsgCore34567ToCore1));
	if(!Msg5To1Ptr)//如果malloc失败
	{
		System_printf("MsgCore34567ToCore1 malloc failed./n");
	}
	Msg6To1Ptr = (MsgCore34567ToCore1*)malloc(sizeof(MsgCore34567ToCore1));
	System_printf("MsgCore34567ToCore1 malloc size is %d.\n", sizeof(MsgCore34567ToCore1));
	if(!Msg6To1Ptr)//如果malloc失败
	{
		System_printf("MsgCore34567ToCore1 malloc failed./n");
	}
	Msg7To1Ptr = (MsgCore34567ToCore1*)malloc(sizeof(MsgCore34567ToCore1));
	System_printf("MsgCore34567ToCore1 malloc size is %d.\n", sizeof(MsgCore34567ToCore1));
	if(!Msg7To1Ptr)//如果malloc失败
	{
		System_printf("MsgCore34567ToCore1 malloc failed./n");
	}

	/* Get the core number. */
	coreNum = CSL_chipReadReg (CSL_CHIP_DNUM);

	/* Initialize IPC, messageQ and Notify*/
	IPC_init();

	/* Wait core 0 signal of QMSS and CPPI configure finish */
	Semaphore_pend(QmssRdySemHandle, BIOS_WAIT_FOREVER);

	/* Start the QMSS. */
	if (Qmss_start() != QMSS_SOK)
	{
		System_printf ("Error: Unable to start the QMSS\n");
		return;
	}

	 //-------------SRIO Qmss_insertMemoryRegion()---------------------------------
//	 /* Memory Region 1Configuration
//	  * QMSS has initialized in CORE0
//	  * configure memory region for SRIO must be executed in CORE1
//	  * !!!! descSize and descNum also defined in CORE0
//	  * startIndex is 128, because memory region 0 has 128 desc for NDK*/
//	Qmss_MemRegInfo             memCfg;
//	memset ((void *)&host_region, 0, sizeof(host_region));
//	memCfg.descBase         = (uint32_t *) l2_global_address ((uint32_t) host_region);
//	memCfg.descSize         = SIZE_HOST_DESC;
//	memCfg.descNum          = NUM_HOST_DESC;
//	memCfg.manageDescFlag   = Qmss_ManageDesc_MANAGE_DESCRIPTOR;
//	memCfg.memRegion        = Qmss_MemRegion_MEMORY_REGION1;
//	memCfg.startIndex       = 128;
//
//	/* Initialize and inset the memory region. */
//	status = Qmss_insertMemoryRegion (&memCfg);
//	if (status == QMSS_MEMREGION_ALREADY_INITIALIZED)
//	{
//		System_printf ("Memory Region %d already Initialized \n", memCfg.memRegion);
//	}
//	else if (status < QMSS_SOK)
//	{
//		System_printf ("Error: Inserting memory region %d, Error code : %d\n", memCfg.memRegion, status);
//	}
	//--------------------------------------------------------------------

	Qmss_MemRegCfg      memRegionInfo;
	status = Qmss_getMemoryRegionCfg(&memRegionInfo);

	/* Power on SRIO peripheral before using it */
	if (enable_srio () < 0)
	{
	   System_printf ("Error: SRIO PSC Initialization Failed\n");
	   return;
	}

	/* Device Specific SRIO Initializations: This should always be called before
	* initializing the SRIO Driver. */
	if (SrioDevice_init() < 0)
		return;

	/* Initialize the SRIO Driver */
	if (Srio_init () < 0)
	{
	   System_printf ("Error: SRIO Driver Initialization Failed\n");
	   return;
	}

	System_printf ("Debug(Core %d): SRIO can now be used.\n", coreNum);

	/* Initialize bridge CPS1432 */
	System_printf("Start configure CPS1432 ............................................................. \n");
	while(rio_discovery(SRIO_PORT_NUM));
	System_printf(".................................................................CPS1432 config finished \n ");

	/* Initialize the OSAL */
    if (Osal_dataBufferInitMemory(SRIO_MAX_MTU) < 0)
    {
	    System_printf ("Error: Unable to initialize the OSAL. \n");
	    return;
    }

    /*
    * The SRIO Driver Instance is going to be created with the following properties:
    * - Driver Managed
    * - Interrupt Support (Pass the Rx Completion Queue as NULL)
    * */
    SrioDrvStart();

    /* Hook up the SRIO interrupts with the core. */
    //Input Event Number 48: QM Interrupt for Queue 704+n8
	EventCombiner_dispatchPlug (48, (EventCombiner_FuncPtr)Srio_rxCompletionIsr, (UArg)hDrvManagedSrioDrv, TRUE);
	EventCombiner_enableEvent(48);

    /* Enable Time Stamp Counter */
    CSL_tscEnable();

    /* Open DIO SRIO Non-Blocking Socket */
//    SrioSocket = Srio_sockOpen (hDrvManagedSrioDrv, Srio_SocketType_DIO, FALSE);
    SrioSocket = Srio_sockOpen (hDrvManagedSrioDrv, Srio_SocketType_DIO, TRUE);
	if (SrioSocket == NULL)
	{
		System_abort ("Error: Unable to open the DIO socket.\n");
	}

	/* DIO Binding Information: Use 16 bit identifiers and we are bound to the first source id.
	* and we are using 16 bit device identifiers. */
	SrioSockBindInfo.dio.doorbellValid  = 0;
	SrioSockBindInfo.dio.intrRequest    = 1;
	SrioSockBindInfo.dio.supInt         = 0;
	SrioSockBindInfo.dio.xambs          = 0;
	SrioSockBindInfo.dio.priority       = 0;
	SrioSockBindInfo.dio.outPortID      = 0;
	SrioSockBindInfo.dio.idSize         = 1;
	SrioSockBindInfo.dio.srcIDMap       = 0;	//Ralated to LM Base Routing.
	SrioSockBindInfo.dio.hopCount       = 1;	//Because there is a bridge in the system, so hopCount is 1.
	SrioSockBindInfo.dio.doorbellReg    = 0;
	SrioSockBindInfo.dio.doorbellBit    = 0;

	/* Bind the SRIO socket: DIO sockets do not need any binding information. */
	status = Srio_sockBind_DIO (SrioSocket, &SrioSockBindInfo);
	if (status < 0)
	{
		System_abort ("Error: Binding the SIO socket failed.\n");
	}

	void *SrioMemoryPtr;
	SrioMemoryPtr = Osal_srioDataBufferMalloc(256);
	if (SrioMemoryPtr == NULL)
	{
		System_abort ("Error: SRIO Frame Memory Allocation Failed. \n");
	}

    while(1)
    {
    	Semaphore_pend(ManualMsgRdyHandle, BIOS_WAIT_FOREVER);
    	//接收到工作使能帧
    	if(NotifyCore0Payload == WORK_ENABLE_NOTIFY)
    	{}
    	//接收到休眠帧
    	else if(NotifyCore0Payload == SLEEP_ENABLE_NOTIFY)
    	{
    		for(i = 0 ; i < RANGE_PROFILE_NUM ; i++)
    		{
    			SrioFrame0SendPtr->RangeProfile[i] = 0;
    		}
    		SrioFrame0SendPtr->DistanceDelay = 0;
    		SrioFrame0SendPtr->DopplerFrePinc = 0;
    		for(i = 0 ; i < ARRAY_NUM ; i++)
			{
    			SrioFrame1SendPtr->OrientationVectorReal[i] = 0;
    			SrioFrame2SendPtr->OrientationVectorImag[i] = 0;
			}
    	}
    	//接收到工作参数帧
    	else if(NotifyCore0Payload == WORK_PARAM_SET_NOTIFY)
    	{
    		status = MessageQ_get(MessageQCore2ToCore1, (MessageQ_Msg*)&Msg2To1Ptr, MessageQ_FOREVER);
    		System_printf("MessageQ_get() MessageQCore2ToCore1, %d. \n", Msg2To1Ptr->header.msgId);
    		/*------------------- CORE2~5 ----------------------*/
			status = MessageQ_get(MessageQCore34567ToCore1, (MessageQ_Msg*)&Msg3To1Ptr, MessageQ_FOREVER);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, CORE ID is %d. \n", Msg3To1Ptr->ProcId);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, MsgId is %d. \n", Msg3To1Ptr->header.msgId);

			status = MessageQ_get(MessageQCore34567ToCore1, (MessageQ_Msg*)&Msg4To1Ptr, MessageQ_FOREVER);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, CORE ID is %d. \n", Msg4To1Ptr->ProcId);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, MsgId is %d. \n", Msg4To1Ptr->header.msgId);

			status = MessageQ_get(MessageQCore34567ToCore1, (MessageQ_Msg*)&Msg5To1Ptr, MessageQ_FOREVER);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, CORE ID is %d. \n", Msg5To1Ptr->ProcId);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, MsgId is %d. \n", Msg5To1Ptr->header.msgId);

			status = MessageQ_get(MessageQCore34567ToCore1, (MessageQ_Msg*)&Msg6To1Ptr, MessageQ_FOREVER);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, CORE ID is %d. \n", Msg6To1Ptr->ProcId);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, MsgId is %d. \n", Msg6To1Ptr->header.msgId);

			status = MessageQ_get(MessageQCore34567ToCore1, (MessageQ_Msg*)&Msg7To1Ptr, MessageQ_FOREVER);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, CORE ID is %d. \n", Msg7To1Ptr->ProcId);
			System_printf("MessageQ_get() MessageQCore23456ToCore1, MsgId is %d. \n", Msg7To1Ptr->header.msgId);

			SrioFrame0SendPtr->DistanceDelay = Msg2To1Ptr->DistanceDelay;
			SrioFrame0SendPtr->DopplerFrePinc = Msg2To1Ptr->DopplerFrePinc;
			for(i = 0 ; i < RANGE_PROFILE_NUM ; i++)
			{
				SrioFrame0SendPtr->RangeProfile[i] = Msg2To1Ptr->RangeProfile[i];
			}
			for(i = 0 ; i < 20 ; i++)
			{
				SrioFrame2SendPtr->OrientationVectorImag[i] = Msg3To1Ptr->OrientationVectorImag[i];
				SrioFrame1SendPtr->OrientationVectorReal[i] = Msg3To1Ptr->OrientationVectorReal[i];
			}
			for(i = 20 ; i < 40 ; i++)
			{
				SrioFrame2SendPtr->OrientationVectorImag[i] = Msg4To1Ptr->OrientationVectorImag[i];
				SrioFrame1SendPtr->OrientationVectorReal[i] = Msg4To1Ptr->OrientationVectorReal[i];
			}
			for(i = 40 ; i < 60 ; i++)
			{
				SrioFrame2SendPtr->OrientationVectorImag[i] = Msg5To1Ptr->OrientationVectorImag[i];
				SrioFrame1SendPtr->OrientationVectorReal[i] = Msg5To1Ptr->OrientationVectorReal[i];
			}
			for(i = 60 ; i <80 ; i++)
			{
				SrioFrame2SendPtr->OrientationVectorImag[i] = Msg6To1Ptr->OrientationVectorImag[i];
				SrioFrame1SendPtr->OrientationVectorReal[i] = Msg6To1Ptr->OrientationVectorReal[i];
			}
			for(i = 80 ; i < 96 ; i++)
			{
				SrioFrame2SendPtr->OrientationVectorImag[i] = Msg7To1Ptr->OrientationVectorImag[i];
				SrioFrame1SendPtr->OrientationVectorReal[i] = Msg7To1Ptr->OrientationVectorReal[i];
			}
    	}

		/* Populate the DIO Address Information where the data is to be sent. */
		SrioSockInfo->dio.rapidIOMSB    = 0;
		SrioSockInfo->dio.rapidIOLSB    = 0x00000000;
		SrioSockInfo->dio.dstID         = DEVICE_ID2_16BIT;
		SrioSockInfo->dio.ttype         = Srio_Ttype_Write_NWRITE_R;
		SrioSockInfo->dio.ftype         = Srio_Ftype_WRITE;

		//Recommended size is 7, 15, 31, 63, 95 (reads only), 127, 159 (reads only), 191(reads only), 223 (reads only), and 255
		memcpy(SrioMemoryPtr, &(SrioFrame0SendPtr->DopplerFrePinc), sizeof(SrioFrame0));
		status = Srio_sockSend_DIO(SrioSocket, SrioMemoryPtr, 128, SrioSockInfo);
		memcpy(SrioMemoryPtr, (SrioFrame1SendPtr->OrientationVectorReal), sizeof(SrioFrame1));
		status = Srio_sockSend_DIO(SrioSocket, SrioMemoryPtr, 256, SrioSockInfo);
		memcpy(SrioMemoryPtr, (SrioFrame2SendPtr->OrientationVectorImag), sizeof(SrioFrame2));
		status = Srio_sockSend_DIO(SrioSocket, SrioMemoryPtr, 256, SrioSockInfo);

//		while((status = Srio_sockSend_DIO(SrioSocket, SrioMemoryPtr, (sizeof(Uint32) + sizeof(Uint16) + sizeof(Uint16) + sizeof(Uint16)*10), SrioSockInfo)) != 0) ;
		if(status != 0)
		{
			System_printf("Error code is %d.\n", status);
			System_printf("Srio_sockSend_DIO() error!!! \n");
		}

		System_printf("Srio_sockSend_DIO(). \n");
    }
}




/**
 *  @b Description
 *  @n  
 *      Entry point for the example
 *
 *  @retval
 *      Not Applicable.
 */
Void main(Void)
{
//    Task_Params     taskParams;
//    /* Create the DIO Example Task.*/
//    Task_Params_init(&taskParams);
//    Task_create(MainThread, &taskParams, NULL);

    /* Start the BIOS */
    BIOS_start();
}
