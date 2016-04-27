/*
 * udpHello.c
 *
 * This program implements a UDP echo server, which echos back any
 * input it receives.
 *
 * Copyright (C) 2007 Texas Instruments Incorporated - http://www.ti.com/ 
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
 *    its contributors may be used TimeOut endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TimeOut, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TimeOut, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include <ti/ndk/inc/netmain.h>
#include <string.h>

#include <ti/csl/csl_cacheAux.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Timer.h>
#include "../CustomHeaderDsp1.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <ti/sysbios/knl/Clock.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Sysmin.h>
#include <ti/ipc/Notify.h>



//
// dtask_udp_hello() - UDP Echo Server Daemon Function
// (SOCK_DGRAM, port 7)
//
// Returns "1" if socket 's' is still open, and "0" if its been closed


//Will be used to send message
extern MsgCore0ToCore1*		Msg0To1Ptr;
extern MsgCore0ToCore2* 	Msg0To2Ptr;
extern MsgCore2ToCore0* 	Msg2To0Ptr;

extern MessageQ_QueueId 	QueueIdCore0ToCore1;
extern MessageQ_QueueId 	QueueIdCore0ToCore2;
extern MessageQ_QueueId 	QueueIdCore2ToCore0;

extern MessageQ_Handle 		MessageQCore2ToCore0;


//---------------ɢ�����Ϣ�洢λ��--------------------------
extern ScatteringPoint		*ScatteringPointPtr;


//send to DSP2
extern HyplinkDataDsp1ToDsp2	HyplinkDataDsp1ToDsp2ToSend;

int			TimerCnt = 0;
int         ManualMsgID = 0;
int			CurrentMode = SLEEP_MODE;
int			MsgID = 0;


//���ڽ��պ���
int 		UdpFunc( SOCKET s, UINT32 unused );




int UdpFunc( SOCKET s, UINT32 unused )
{
    struct sockaddr_in      SocketAddrData;
    struct timeval          TimeOut;
    int                     plen;
    void                    *pBuf;
    HANDLE                  hBuffer;
    int                     status;
    int						UdpRecvLen;
    int						TargetParamSize;

    // Configure our socket timeout TimeOut be 0 seconds
    TimeOut.tv_sec  = 0;
    TimeOut.tv_usec = 0;
    setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, &TimeOut, sizeof( TimeOut ) );
    setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, &TimeOut, sizeof( TimeOut ) );

    plen = sizeof( SocketAddrData );
	pBuf = (void* )malloc(1024);		//������֡������󲻻ᳬ��1024�ֽ�

	UdpRecvLen = recvncfrom( s, (void **)&pBuf, MSG_WAITALL, (PSA)&SocketAddrData, &plen, &hBuffer );

//	//��⵱ǰģʽ�Ƿ�֧���յ���֡
//	if(CurrentMode == SLEEP_MODE)
//	{
//		if(*((char*)pBuf) == WORK_PARAM_SET)
//		{
//
//		}
//	}

	//----����ͨ��----
	if(*((char*)pBuf) == COMMUNICATION_CHECK)
	{
		sendto(s, pBuf, UdpRecvLen, MSG_WAITALL, (PSA)&SocketAddrData, plen);
		System_printf("����ͨ��\n");
	}
	//----����ʹ��----
	else if(*((char*)pBuf) == WORK_ENABLE)
	{
//		Msg0To1Ptr->FrameId = WORK_ENABLE;
	}
	//----ϵͳ����----
	else if(*((char*)pBuf) == SLEEP_ENABLE)
	{
//		Msg0To1Ptr->FrameId = SLEEP_ENABLE;
	}
	//----����ɢ���----
	else if(*((char*)pBuf) == SCATTERING_POINT_INPUT)
	{
		ScatteringPointUdpFrame* ScatteringPointUdpFramePtr = (ScatteringPointUdpFrame*)malloc(sizeof(ScatteringPointUdpFrame));
		memcpy(ScatteringPointUdpFramePtr, pBuf, sizeof(ScatteringPointUdpFrame));	//�������������ڶ�ȡʱ�����⣬��������Ϊcache

		memcpy(	ScatteringPointPtr,
				&(ScatteringPointUdpFramePtr->ScatteringPointData),
				sizeof(ScatteringPointUdpFramePtr->ScatteringPointData.PointNum) +
								sizeof(Point) * (ScatteringPointUdpFramePtr->ScatteringPointData.PointNum));

//		status = Notify_sendEvent(PROC_ID_CORE1, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, QMSS_RDY_NOTIFY, TRUE);

		//�ش�����
		char success = 0x01;
		sendto(s, &success, sizeof(char), MSG_WAITALL, (PSA)&SocketAddrData, plen);

		free(ScatteringPointUdpFramePtr);

		System_printf("����ɢ���\n");
	}
	//----������������----
	else if(*((char*)pBuf) == WORK_PARAM_SET)
	{
		WorkParamUdpFrame* WorkParamUdpFramePtr = (WorkParamUdpFrame*)malloc(sizeof(WorkParamUdpFrame));
		memcpy(WorkParamUdpFramePtr, pBuf, sizeof(WorkParamUdpFrame));	//�������������ڶ�ȡʱ�����⣬��������Ϊcache
		memcpy(	&HyplinkDataDsp1ToDsp2ToSend,
				&(WorkParamUdpFramePtr->JammingFrameId),
				sizeof(HyplinkDataDsp1ToDsp2ToSend));
		MessageQ_setMsgId(&(Msg0To2Ptr->header), MsgID++);


		/* ��ȡĿ����� */
		//��Ŀ��
		if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == POINT_TARGET)
		{
			Msg0To2Ptr->TargetFrameId = WorkParamUdpFramePtr->TargetFrameId;
			memcpy(&(Msg0To2Ptr->TargetParam), &(WorkParamUdpFramePtr->TargetFrame), sizeof(PointTargetParam));
			System_printf("POINT_TARGET\n");
		}
		//��չĿ��,����0
		else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_0)
		{
			Msg0To2Ptr->TargetFrameId = WorkParamUdpFramePtr->TargetFrameId;
			memcpy(&(Msg0To2Ptr->TargetParam), &(WorkParamUdpFramePtr->TargetFrame), sizeof(RangeSpreadTargetParam0));
			System_printf("RANGE_SPREAD_TARGET_0\n");
		}
		//��չĿ��,����1
		else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_1)
		{
			Msg0To2Ptr->TargetFrameId = WorkParamUdpFramePtr->TargetFrameId;
			memcpy(&(Msg0To2Ptr->TargetParam), &(WorkParamUdpFramePtr->TargetFrame), sizeof(RangeSpreadTargetParam1));
			System_printf("RANGE_SPREAD_TARGET_1\n");
		}
		//��չĿ��,����2
		else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_2)
		{
			Msg0To2Ptr->TargetFrameId = WorkParamUdpFramePtr->TargetFrameId;
			memcpy(&(Msg0To2Ptr->TargetParam), &(WorkParamUdpFramePtr->TargetFrame), sizeof(RangeSpreadTargetParam2));
			System_printf("RANGE_SPREAD_TARGET_2\n");
		}

		//������������
		Msg0To2Ptr->NoisePower = WorkParamUdpFramePtr->NoisePower;

		/* Send message to core 2. */
		status = MessageQ_put(QueueIdCore0ToCore2, &(Msg0To2Ptr->header));
		if (status < 0)
		{
			System_abort("MessageQ_put had a failure/error\n");
		}
		System_printf("MessageQ_put(QueueIdCore0ToCore2, &(Msg0To2Ptr->header))\n");


		/* ��ȡ���Ų���  */
//		char *JammingPtrTemp = *((char*)(pBuf) + 1 + TargetParamSize);
//		HyplinkDataDsp1ToDsp2Ptr->JammingFrameId = *JammingPtrTemp;
//		//Copy the value.
//		memcpy((&(HyplinkDataDsp1ToDsp2Ptr->JammingParam)), JammingPtrTemp + 1, sizeof(HyplinkDataDsp1ToDsp2Ptr->JammingParam));
//		//�޸���
//		if(*JammingPtrTemp == NO_JAMMING)
//		{
//
//		}
//		//��Ъ����ת��������0
//		else if(*JammingPtrTemp == ISRJ_0)
//		{
//
//		}
//		//��Ъ����ת��������1
//		else if(*JammingPtrTemp == ISRJ_1)
//		{
//
//		}
//		//��Ъ����ת��������2
//		else if(*JammingPtrTemp == ISRJ_2)
//		{
//
//		}

		/* Send HyperLink data to core0. */

		/* Notify core1 to receive msg from core 3~7. */
		status = Notify_sendEvent(PROC_ID_CORE1, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, WORK_PARAM_SET_NOTIFY, TRUE);
		if (status < 0)
		{
			System_printf("Notify_sendEvent had a failure/error, error code is %d \n", status);
			System_abort(" ");
		}
		System_printf("Notify_sendEvent(PROC_ID_CORE1, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, WORK_PARAM_SET_NOTIFY, TRUE)\n");

		/* ��Ҫ�ش�����  */
		if(((*((char*)pBuf + 1) >> 16) == MANUAL_MODE)||((*((char*)pBuf + 1) >> 16) == AUTO_MODE_PASS_BACK))
		{
			WorkParamSetBack		WorkParamSetBackFrame;

			WorkParamSetBackFrame.FrameId = WorkParamUdpFramePtr->FrameId;
			//��Ŀ��
			if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == POINT_TARGET)
			{
				WorkParamSetBackFrame.TargetFrameId = WorkParamUdpFramePtr->TargetFrameId;
				WorkParamSetBackFrame.TargetParamBack.PointTargetBack = 0x01;
				System_printf("POINT_TARGET\n");
			}
			//��չĿ��,����0
			else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_0)
			{
				//�ȴ����ջش�
				while(MessageQ_get(MessageQCore2ToCore0, (MessageQ_Msg *)&Msg2To0Ptr, MessageQ_FOREVER) != 0);
				WorkParamSetBackFrame.TargetFrameId = WorkParamUdpFramePtr->TargetFrameId;
				WorkParamSetBackFrame.TargetParamBack.RangeSpreadTargetParam0SetBackFrame = Msg2To0Ptr->TargetParamBack.RangeSpreadTargetParam0SetBackFrame;
				System_printf("POINT_TARGET\n");
			}
			//��չĿ��,����1
			else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_1)
			{
				//�ȴ����ջش�
				while(MessageQ_get(MessageQCore2ToCore0, (MessageQ_Msg *)&Msg2To0Ptr, MessageQ_FOREVER) != 0);
				WorkParamSetBackFrame.TargetFrameId = WorkParamUdpFramePtr->TargetFrameId;
				WorkParamSetBackFrame.TargetParamBack.RangeSpreadTargetParam12SetBackFrame = Msg2To0Ptr->TargetParamBack.RangeSpreadTargetParam12SetBackFrame;
				System_printf("RANGE_SPREAD_TARGET_1\n");
			}
			//��չĿ��,����2
			else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_2)
			{
				//�ȴ����ջش�
				while(MessageQ_get(MessageQCore2ToCore0, (MessageQ_Msg *)&Msg2To0Ptr, MessageQ_FOREVER) != 0);
				WorkParamSetBackFrame.TargetFrameId = WorkParamUdpFramePtr->TargetFrameId;
				WorkParamSetBackFrame.TargetParamBack.RangeSpreadTargetParam12SetBackFrame = Msg2To0Ptr->TargetParamBack.RangeSpreadTargetParam12SetBackFrame;
				System_printf("RANGE_SPREAD_TARGET_2\n");
			}

			CACHE_invL1d(Msg2To0Ptr, sizeof(MsgCore2ToCore0), CACHE_WAIT);	//��cache��invalid

			//�޸���
			if(WorkParamUdpFramePtr->JammingFrameId == NO_JAMMING)
			{
				WorkParamSetBackFrame.JammingFrameId = WorkParamUdpFramePtr->JammingFrameId;
				WorkParamSetBackFrame.JammingParamBack.NoJammingBack = 0x01;
			}
			//��Ъ����ת��������0
			else if(WorkParamUdpFramePtr->JammingFrameId == ISRJ_0)
			{

			}
			//��Ъ����ת��������1
			else if(WorkParamUdpFramePtr->JammingFrameId == ISRJ_1)
			{

			}
			//��Ъ����ת��������2
			else if(WorkParamUdpFramePtr->JammingFrameId == ISRJ_2)
			{

			}

			//�ش�����
			sendto(s, &WorkParamSetBackFrame, sizeof(WorkParamSetBack), MSG_WAITALL, (PSA)&SocketAddrData, plen);
		}

		free(WorkParamUdpFramePtr);
		System_printf("������������\n");
	}

    return(1);
}

