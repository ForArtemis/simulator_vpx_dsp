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

#include "../CustomHeaderDsp1.h"

#include<stdlib.h>

//Coordinate of array element
const float ArrayElementCoordinateX[5][20] =
{
		{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
		 10, 11, 12, 13, 14, 15, 16, 17, 18, 19},
		{20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		 30, 31, 32, 33, 34, 35, 36, 37, 38, 39},
		{40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
		 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},
		{60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
		 70, 71, 72, 73, 74, 75, 76, 77, 78, 79},
		{80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
		 90, 91, 92, 93, 94, 95, 96, 97, 98, 99}
};
const float ArrayElementCoordinateY[5][20] =
{
		{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
		 10, 11, 12, 13, 14, 15, 16, 17, 18, 19},
		{20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		 30, 31, 32, 33, 34, 35, 36, 37, 38, 39},
		{40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
		 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},
		{60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
		 70, 71, 72, 73, 74, 75, 76, 77, 78, 79},
		{80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
		 90, 91, 92, 93, 94, 95, 96, 97, 98, 99}
};

//Will be used to send message
MessageQ_QueueId 		QueueIdCore34567ToCore1;

MessageQ_Handle  		MessageQCore2ToCore34567;

MsgCore34567ToCore1*	Msg34567To1Ptr;
MsgCore2ToCore34567*	Msg2To34567Ptr;


/* Oriented vector */
void OcrientationVectorCal(
		float 	ArrayElementCoordinateX,
		float 	ArrayElementCoordinateY,
		float   TargetAngleTheta,
		float   TargetAnglePhi,
		Uint16*	OrientationVectorRealPtr,
		Uint16*	OrientationVectorImagPtr
		)
{
	float	OrientationVectorRealTemp;
	float	OrientationVectorImagTemp;

	OrientationVectorRealTemp = cos(2*PI/LAMBDA*ArrayElementCoordinateX*sin(TargetAngleTheta)*cos(TargetAnglePhi) +
									2*PI/LAMBDA*ArrayElementCoordinateY*sin(TargetAngleTheta)*sin(TargetAnglePhi));

	OrientationVectorImagTemp = sin(2*PI/LAMBDA*ArrayElementCoordinateX*sin(TargetAngleTheta)*cos(TargetAnglePhi) +
									2*PI/LAMBDA*ArrayElementCoordinateY*sin(TargetAngleTheta)*sin(TargetAnglePhi));

	*OrientationVectorRealPtr = OrientationVectorRealTemp * 0xffff;
	*OrientationVectorImagPtr = OrientationVectorImagTemp * 0xffff;

}

/* Initialize IPC and MessageQ */
void IPC_init(
		String 	HeapBufMPStringFromCore2,
		String 	MessageQStringFromCore2,
		String	MessageQStringToCore1
)
{
	Int status;
	HeapBufMP_Handle 	HeapHandleCore34567Core1;
	HeapBufMP_Handle    heapHandle;

	//Ipc_start does not Ipc_attach, because 'Ipc.procSync' is set to 'Ipc.ProcSync_PAIR' in *.cfg
	//Ipc reserves some shared memory in SharedRegion zero for synchronization.
	status = Ipc_start();
	if (status < 0)
	{
		System_abort("Ipc_start failed\n");
	}

	//Wait IPC attach
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

	//--------------To CORE1--------------------------------
	do {
		status = HeapBufMP_open(HEAP_BUF_NAME_CORE34567_CORE1, &HeapHandleCore34567Core1);
		Task_sleep(1) ;
	} while (status < 0);
	System_printf("HeapBufMP_open(HEAP_BUF_NAME_CORE34567_CORE1, &HeapHandleCore34567Core1); \n");

	status = MessageQ_registerHeap((IHeap_Handle)HeapHandleCore34567Core1, HEAP_ID_CORE34567_CORE1);
	System_printf("MessageQ_registerHeap((IHeap_Handle)HeapHandleCore34567Core1, HEAP_ID_CORE34567_CORE1); \n");

	do {
//		status = MessageQ_open(MSGQ_NAME_CORE34567_CORE1, &QueueIdCore34567ToCore1);
		status = MessageQ_open(MessageQStringToCore1, &QueueIdCore34567ToCore1);
		Task_sleep(1) ;
	} while (status < 0);
	System_printf("MessageQ_open(MessageQStringToCore1, &QueueIdCore34567ToCore1); \n");

	Msg34567To1Ptr = (MsgCore34567ToCore1*)MessageQ_alloc(HEAP_ID_CORE34567_CORE1, sizeof(MsgCore34567ToCore1));
	Msg34567To1Ptr = (MsgCore34567ToCore1*)MessageQ_alloc(HEAP_ID_CORE34567_CORE1, sizeof(MsgCore34567ToCore1));	//不用第一次分配的空间，因为会有问题，不明
	if (Msg34567To1Ptr == NULL)
	{
		System_abort("MessageQ_alloc failed\n" );
	}
	System_printf("(MsgCore34567ToCore1*)MessageQ_alloc(HEAP_ID_CORE34567_CORE1, sizeof(MsgCore34567ToCore1)); \n");


	/* Create the heap that will be used to allocate messages. */
	//---------------For CORE2--------------------------
	do {
		status = HeapBufMP_open(HeapBufMPStringFromCore2, &heapHandle);
		Task_sleep(1) ;
	} while (status < 0);
	System_printf("HeapBufMP_open(HeapBufMPStringFromCore0, &heapHandle); \n");
	/* Create the local message queue */
	MessageQCore2ToCore34567 = MessageQ_create(MessageQStringFromCore2, NULL);
	if (MessageQCore2ToCore34567 == NULL)
	{
		System_abort("MessageQ_create failed\n");
	}
	System_printf("MessageQ_create(MessageQStringFromCore2, NULL); \n");

	System_printf("Debug(Core 2): IPC_init() finished. \n");
}

void MainThread()
{
	int i;
	int status;
	int CoreNum;
	String 	HeapBufMPStringFromCore2;
	String 	MessageQStringFromCore2;
	String	MessageQStringToCore1;


	/* Read core number */
	CoreNum = CSL_chipReadDNUM();

	/* Select HeapMP and MessageQ
	 * Initialize some parameter, according to core number. */
	if(CoreNum == PROC_ID_CORE3)
	{
		HeapBufMPStringFromCore2 = HEAP_BUF_NAME_CORE2_CORE34567;
		MessageQStringFromCore2  = MSGQ_NAME_CORE2_CORE3;
		MessageQStringToCore1 = MSGQ_NAME_CORE3_CORE1;
	}
	else if(CoreNum == PROC_ID_CORE4)
	{
		HeapBufMPStringFromCore2 = HEAP_BUF_NAME_CORE2_CORE34567;
		MessageQStringFromCore2  = MSGQ_NAME_CORE2_CORE4;
		MessageQStringToCore1 = MSGQ_NAME_CORE4_CORE1;
	}
	else if(CoreNum == PROC_ID_CORE5)
	{
		HeapBufMPStringFromCore2 = HEAP_BUF_NAME_CORE2_CORE34567;
		MessageQStringFromCore2  = MSGQ_NAME_CORE2_CORE5;
		MessageQStringToCore1 = MSGQ_NAME_CORE5_CORE1;
	}
	else if(CoreNum == PROC_ID_CORE6)
	{
		HeapBufMPStringFromCore2 = HEAP_BUF_NAME_CORE2_CORE34567;
		MessageQStringFromCore2  = MSGQ_NAME_CORE2_CORE6;
		MessageQStringToCore1 = MSGQ_NAME_CORE6_CORE1;
	}
	else if(CoreNum == PROC_ID_CORE7)
	{
		HeapBufMPStringFromCore2 = HEAP_BUF_NAME_CORE2_CORE34567;
		MessageQStringFromCore2  = MSGQ_NAME_CORE2_CORE7;
		MessageQStringToCore1 = MSGQ_NAME_CORE7_CORE1;
	}
	else
	{
		System_abort("Wrong PrcID. \n");
	}


	System_printf("HeapBufMPStringFromCore2 = %s\n", HeapBufMPStringFromCore2);
	System_printf("MessageQStringFromCore2 = %s\n", MessageQStringFromCore2);

	Msg2To34567Ptr = (MsgCore2ToCore34567*)malloc(sizeof(MsgCore2ToCore34567));
	System_printf("MsgCore2ToCore34567 malloc size is %d.\n", sizeof(MsgCore2ToCore34567));
	if(!Msg2To34567Ptr)//如果malloc失败
	{
		System_printf("MsgCore2ToCore34567 malloc failed./n");
	}

	/* Initialize IPC and MessageQ */
	IPC_init(HeapBufMPStringFromCore2, MessageQStringFromCore2, MessageQStringToCore1);

	while(1)
	{
		while(MessageQ_get(MessageQCore2ToCore34567, (MessageQ_Msg *)&Msg2To34567Ptr, MessageQ_FOREVER) != 0);
		CACHE_invL1d(Msg2To34567Ptr, sizeof(MsgCore2ToCore34567), CACHE_WAIT);	//从cache中invalid

		//Set Proc ID
		Msg34567To1Ptr->ProcId = CoreNum;

		for(i = 0 ; i < 20 ; i++)
		{
			OcrientationVectorCal(
					ArrayElementCoordinateX[CoreNum-3][i],
					ArrayElementCoordinateY[CoreNum-3][i],
					Msg2To34567Ptr->TargetAngleTheta,
					Msg2To34567Ptr->TargetAnglePhi,
					(Msg34567To1Ptr->OrientationVectorReal + i),
					(Msg34567To1Ptr->OrientationVectorImag + i)
				);
		}


		//send Message to CORE1
		MessageQ_setMsgId(&(Msg34567To1Ptr->header), (Msg2To34567Ptr->header.msgId));
		status = MessageQ_put(QueueIdCore34567ToCore1, &(Msg34567To1Ptr->header));
		if (status < 0)
		{
		   System_abort("MessageQ_put had a failure/error\n");
		}
		System_printf("MessageQ_put\n");
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
