/* BIOS6 include */
#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/heaps/HeapBuf.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/csl/csl_ipcAux.h>
#include <ti/csl/csl_chipAux.h>
#include <ti/csl/csl_cacheAux.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/IHeap.h>

#include <ti/ipc/GateMP.h>
#include <ti/ipc/Ipc.h>
#include <ti/ipc/ListMP.h>
#include <ti/ipc/SharedRegion.h>
#include <ti/ipc/MultiProc.h>

#include <ti/ipc/Ipc.h>
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/HeapBufMP.h>
#include <ti/ipc/MultiProc.h>

#include <math.h>

#include "../CustomHeaderDsp2.h"

#include<stdlib.h>

#include <ti/sysbios/family/c66/tci66xx/CpIntc.h>
#include <ti/sysbios/hal/Hwi.h>
#include <xdc/runtime/Error.h>

#include "hyplnkLLDIFace.h"
#include "hyplnkLLDCfg.h"
#include "hyplnkIsr.h"
#include "hyplnkPlatCfg.h"


//Will be used to send message
MessageQ_QueueId 		QueueIdCore34567ToCore1;

MessageQ_Handle  		MessageQCore2ToCore34567;

MsgCore34567ToCore1*	Msg34567To1Ptr;
MsgCore2ToCore34567*	Msg2To34567Ptr;


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
#pragma DATA_SECTION (HyplinkDataDsp1Dsp2Buffer, "hyperlink");
#pragma DATA_ALIGN (HyplinkDataDsp1Dsp2Buffer, hyplnk_EXAMPLE_LINE_SIZE);
HyplinkDataDsp1Dsp2 HyplinkDataDsp1Dsp2Buffer;
/*
 * This pointer is the local address within the Hyperlink's address
 * space which will access dataBuffer via HyperLink.
 *
 */
HyplinkDataDsp1Dsp2 *bufferThroughHypLnk;


void HyperlinkIsr()
{

}

/* 初始化Hyperlink */
void HyperlinkInit()
{
	TSCL = 0;
	TSCH = 0;

	int retVal;

	/* Set up the system PLL, PSC, and DDR as required for this HW */
	System_printf ("About to do Hyperlink system setup (PLL, PSC, and DDR)\n");
	if ((retVal = hyplnkExampleSysSetup()) != hyplnk_RET_OK) {
		System_printf ("Hyperlink system setup failed (%d)\n", (int)retVal);
		exit(1);
	}
	System_printf ("Hyperlink system setup worked\n");


	/* Hyperlink interrupt init */
	Int eventId;
	Hwi_Params params;
	Error_Block eb;

	// Initialize the error block
	Error_init(&eb);

	// Map system interrupt 111 to host interrupt 10 on Intc 0
	CpIntc_mapSysIntToHostInt(0, CSL_INTC0_VUSR_INT_O, 10);

	// Plug the function and argument for System interrupt 15 then enable it
	CpIntc_dispatchPlug(CSL_INTC0_VUSR_INT_O, &HyperlinkIsr, 0, TRUE);

	// Enable Host interrupt 10 on Intc 0
	CpIntc_enableHostInt(0, 10);

	// Get the eventId associated with Host interrupt 10
	eventId = CpIntc_getEventId(10);

	// Initialize the Hwi parameters
	Hwi_Params_init(&params);

	// Set the eventId associated with the Host Interrupt
	params.eventId = eventId;

	// The arg must be set to the Host interrupt
	params.arg = 10;

	// Enable the interrupt vector
	params.enableInt = TRUE;

	// Create the Hwi on interrupt 5 then specify 'CpIntc_dispatch'
	// as the function.
	Hwi_create(6, &CpIntc_dispatch, &params, &eb);



	/* Enable the peripheral */
	System_printf ("About to set up HyperLink Peripheral\n");
	if ((retVal = hyplnkExamplePeriphSetup()) != hyplnk_RET_OK) {
		System_printf ("HyperLink system setup failed (%d)\n", (int)retVal);
		exit(1);
	}
	System_printf ("HyperLink Peripheral setup worked\n");

	/* Hyperlink 初始化 */
	/* Hyperlink is not up yet, so we don't have to worry about concurrence */
	memset (&HyplinkDataDsp1Dsp2Buffer, 0, sizeof(HyplinkDataDsp1Dsp2Buffer));

	/* Set up address mapsrc */
	if ((retVal = hyplnkExampleAddrMap (&HyplinkDataDsp1Dsp2Buffer, (void **)&bufferThroughHypLnk)) != hyplnk_RET_OK) {
		System_printf ("Address map setup failed (%d)\n", (int)retVal);
		exit(1);
	}

}

/* Initialize IPC and MessageQ */
//void IPC_init()
//{
//    Int                 status;
//    HeapBufMP_Handle    heapHandle;
//
//    //Ipc_start does not Ipc_attach, because 'Ipc.procSync' is set to 'Ipc.ProcSync_PAIR' in *.cfg
//    //Ipc reserves some shared memory in SharedRegion zero for synchronization.
//    status = Ipc_start();
//    if (status < 0)
//    {
//		System_abort("Ipc_start failed\n");
//    }
//    /*--------------- Wait IPC attach to other cores ----------------*/
//    //!!!! Must attach to the share memory owner - CORE1 firstly
//    while ((status = Ipc_attach(PROC_ID_CORE1)) < 0)
//	{
//    	Task_sleep(1) ;
//	}
//	while ((status = Ipc_attach(PROC_ID_CORE2)) < 0)
//	{
//		Task_sleep(1) ;
//	}
//    System_printf("Ipc_attach() finished.\n");
//
//    /*------------------------- To CORE1 ----------------------*/
//	do {
//		status = HeapBufMP_open(HEAP_BUF_NAME_CORE0_CORE1, &HeapHandleCore0ToCore1);
//		Task_sleep(1) ;
//	} while (status < 0);
//	System_printf("HeapBufMP_open HEAP_BUF_NAME_CORE0_CORE1 finished. \n");
//
//	do {
//		status = MessageQ_open(MSGQ_NAME_CORE0_CORE1, &QueueIdCore0ToCore1);
//		Task_sleep(1) ;
//	} while (status < 0);
//	System_printf("MessageQ_open MSGQ_NAME_CORE0_CORE1 finished. \n");
//
//	status = MessageQ_registerHeap((IHeap_Handle)HeapHandleCore0ToCore1, HEAP_ID_CORE0_CORE1);
//
//	Msg0To1Ptr = (MsgCore0ToCore1*)MessageQ_alloc(HEAP_ID_CORE0_CORE1, sizeof(MsgCore0ToCore1));
//	if (Msg0To1Ptr == NULL)
//	{
//	   System_abort("MessageQ_alloc failed\n" );
//	}
//	else
//	{
//		System_printf("MessageQ_alloc(HEAP_ID_CORE0_CORE1, sizeof(MsgCore0ToCore1)) finished. \n");
//	}
//
//	/*------------------------- To CORE2 ----------------------*/
//	do {
//		status = HeapBufMP_open(HEAP_BUF_NAME_CORE0_CORE2, &HeapHandleCore0ToCore2);
//		Task_sleep(1) ;
//	} while (status < 0);
//	System_printf("HeapBufMP_open HEAP_BUF_NAME_CORE0_CORE2 finished. \n");
//
//	do {
//		status = MessageQ_open(MSGQ_NAME_CORE0_CORE2, &QueueIdCore0ToCore2);
//		Task_sleep(1) ;
//	} while (status < 0);
//	System_printf("MessageQ_open MSGQ_NAME_CORE0_CORE2 finished. \n");
//
//	MessageQ_registerHeap((IHeap_Handle)HeapHandleCore0ToCore2, HEAP_ID_CORE0_CORE2);
//
//	Msg0To2Ptr= (MsgCore0ToCore2*)MessageQ_alloc(HEAP_ID_CORE0_CORE2, sizeof(MsgCore0ToCore2));
//	if (Msg0To2Ptr == NULL)
//	{
//	   System_abort("MessageQ_alloc failed\n" );
//	}
//	else
//	{
//		System_printf("MessageQ_alloc(HEAP_ID_CORE0_CORE2, sizeof(MsgCore0ToCore23456)) finished. \n");
//	}
//
//	/* Create the heap that will be used to allocate messages. */
//	//---------------For CORE2--------------------------
////	do {
////		status = HeapBufMP_open(HEAP_BUF_NAME_CORE2_CORE0, &heapHandle);
////		Task_sleep(1) ;
////	} while (status < 0);
////	System_printf("HeapBufMP_open(HeapBufMPStringFromCore2, &heapHandle); \n");
//	/* Create the local message queue */
//	MessageQCore2ToCore0 = MessageQ_create(MSGQ_NAME_CORE2_CORE0, NULL);
//	if (MessageQCore2ToCore0 == NULL)
//	{
//		System_abort("MessageQ_create failed\n");
//	}
//	System_printf("MessageQ_create(MSGQ_NAME_CORE2_CORE0, NULL); \n");
//
//	//-------获得散射点信息存储空间-----------------
//	SharedRegion_SRPtr ScatteringPointSrPtr;	//共享存储空间下的指针
//	ScatteringPointPtr = Memory_alloc(SharedRegion_getHeap(1), sizeof(ScatteringPoint), 128, NULL);	//128为对齐字节，可设置与cacheline相同，此处128为临时设置
//	if(ScatteringPointPtr == NULL)
//	{
//		System_printf("SharedRegion_getHeap() failed!!\n");
//	}
////	ScatteringPointSrPtr = SharedRegion_getSRPtr((Ptr)ScatteringPointPtr, 1);
//
//	status = Notify_sendEvent(PROC_ID_CORE2, LINE_ID_CORE0_CORE2, EVENT_ID_CORE0_CORE2, (Uint32)ScatteringPointPtr, TRUE);
//	if(status < 0)
//	{
//		System_printf("Notify_sendEvent() failed!!\n");
//	}
//
//	System_printf("Debug(Core 0): IPC_init() finished. \n");
//}

void MainThread()
{
	int status;
	int CoreNum;
	String 	HeapBufMPStringFromCore2;
	String 	MessageQStringFromCore2;
	String	MessageQStringToCore1;



	/* Initialize IPC and MessageQ */
//	IPC_init();

	HyperlinkInit();


	while(1)
	{
//		while(MessageQ_get(MessageQCore2ToCore34567, (MessageQ_Msg *)&Msg2To34567Ptr, MessageQ_FOREVER) != 0);
//		CACHE_invL1d(Msg2To34567Ptr, sizeof(MsgCore2ToCore34567), CACHE_WAIT);	//从cache中invalid
//
//		//Set Proc ID
//		Msg34567To1Ptr->ProcId = CoreNum;
//
//		for(i = 0 ; i < 20 ; i++)
//		{
//			OcrientationVectorCal(
//					ArrayElementCoordinateX[CoreNum-3][i],
//					ArrayElementCoordinateY[CoreNum-3][i],
//					Msg2To34567Ptr->TargetAngleTheta,
//					Msg2To34567Ptr->TargetAnglePhi,
//					(Msg34567To1Ptr->OrientationVectorReal + i),
//					(Msg34567To1Ptr->OrientationVectorImag + i)
//				);
//		}
//
//
//		//send Message to CORE1
//		MessageQ_setMsgId(&(Msg34567To1Ptr->header), (Msg2To34567Ptr->header.msgId));
//		status = MessageQ_put(QueueIdCore34567ToCore1, &(Msg34567To1Ptr->header));
//		if (status < 0)
//		{
//		   System_abort("MessageQ_put had a failure/error\n");
//		}
//		System_printf("MessageQ_put\n");
	}
}

//---------------------------------------------------------------------
// Main Entry Point
//---------------------------------------------------------------------
int main()
{
	/* Start the BIOS 6 Scheduler */
	BIOS_start ();
}
