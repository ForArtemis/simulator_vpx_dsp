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

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Timer.h>
#include "../CustomHeader.h"

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


//---------------散射点信息存储位置--------------------------
extern ScatteringPoint		*ScatteringPointPtr;


//send to DSP2
extern HyplinkDataDsp1ToDsp2	HyplinkDataDsp1ToDsp2Ptr;

int			TimerCnt = 0;
int         ManualMsgID = 0;

int			CurrentMode = SLEEP_MODE;

////---------------------------------------------
//char	pBuf_static[1024];
//#pragma DATA_ALIGN (pBuf, 4);




//extern Timer_Handle ManualModeTimer;
//
//extern Semaphore_Handle ManualTimingSemHandle;


//void 	DistanceDelayCal(UdpFrame* UdpFramePtr, MsgCore0ToCore1* MsgCore0ToCore1Ptr);
//Uint32 	DopplerFrePincCal(float speed);
//void 	AmplitudeCal(float* ScatteringPointPowerX, float* ScatteringPointPowerZ, float* ScatteringPointPowerY,
//			Uint16* ScatteringPointAmplitudeX, Uint16* ScatteringPointAmplitudeY, Uint16* ScatteringPointAmplitudeZ);
//float 	LineDeviationCal(UdpFrame *Frame,
//								Uint16 ScatteringPointAmplitudeY[SCATTERTING_POINT_NUM], Uint16 ScatteringPointAmplitudeZ[SCATTERTING_POINT_NUM],
//										enum AngleDirectionEnum AngleDirection);
int 	UdpFunc( SOCKET s, UINT32 unused );


//void DistanceDelayCal(UdpFrame* UdpFramePtr, MsgCore0ToCore1* MsgCore0ToCore1Ptr)
//{
//	//Minus SCATTERING_POINT_MAX_SIZE/2, because the position can be negative.
//	int	i;
//	MsgCore0ToCore1Ptr->DistanceDelay = UdpFramePtr->TargetDistance / LIGHT_SPEED * FPGA_CLK_FRE - RANGE_PROFILE_NUM/2; 	// delay = 2R/C*FPGA_CLK_FRE
//	for(i = 0 ; i < SCATTERTING_POINT_NUM ; i++)
//	{
//		MsgCore0ToCore1Ptr->ScatteringPointPosition[i] = UdpFramePtr->ScatteringPointPositionX[i] + RANGE_PROFILE_NUM/2;
//	}
//}
//
//Uint32 DopplerFrePincCal(float speed)
//{
//	Uint32 Pinc;
//
//	Pinc = 2 * speed * WAVE_FRE / LIGHT_SPEED * 0xffffffff / FPGA_CLK_FRE;
//	return	Pinc;
//}
//
//void AmplitudeCal(float* ScatteringPointPowerX, float* ScatteringPointPowerY, float* ScatteringPointPowerZ,
//						Uint16* ScatteringPointAmplitudeX, Uint16* ScatteringPointAmplitudeY, Uint16* ScatteringPointAmplitudeZ)
//{
//	int 	i;
//	for(i = 0 ; i < SCATTERTING_POINT_NUM ; i++)
//	{
//		if(ScatteringPointPowerX[i] != (ScatteringPointPowerX[i] + 1))
//			ScatteringPointAmplitudeX[i] = sqrt(pow(10,((ScatteringPointPowerX[i] - POEWR_MAX_DBM)/10)) * AMPLITUDE_MAX * AMPLITUDE_MAX);
//		else
//			ScatteringPointAmplitudeX[i] = 0;
//		if(ScatteringPointPowerY[i] != (ScatteringPointPowerY[i] + 1))
//			ScatteringPointAmplitudeY[i] = sqrt(pow(10,((ScatteringPointPowerY[i] - POEWR_MAX_DBM)/10)) * AMPLITUDE_MAX * AMPLITUDE_MAX);
//		else
//			ScatteringPointAmplitudeY[i] = 0;
//		if(ScatteringPointPowerZ[i] != (ScatteringPointPowerZ[i] + 1))
//			ScatteringPointAmplitudeZ[i] = sqrt(pow(10,((ScatteringPointPowerZ[i] - POEWR_MAX_DBM)/10)) * AMPLITUDE_MAX * AMPLITUDE_MAX);
//		else
//			ScatteringPointAmplitudeZ[i] = 0;
//	}
//}
//
///* Calculate line deviation caused by amplitude fluctuation of each scattering point */
//float LineDeviationCal(UdpFrame *Frame,
//								Uint16 ScatteringPointAmplitudeY[SCATTERTING_POINT_NUM], Uint16 ScatteringPointAmplitudeZ[SCATTERTING_POINT_NUM],
//										enum AngleDirectionEnum AngleDirection)
//{
//	int	i;
//	int j;
//
//	float 	*ScatteringPointPositionPtr;
//
//	float 	LineDeviationMember = 0;
//	float 	LineDeviationDenominator = 0;
//
//	float 	EchoPhase[SCATTERTING_POINT_NUM];
//	Uint16* ScatteringPointAmplitude;
//
//	/* Select scattering point position to be used. */
//	if(AngleDirection == THETA)
//	{
//		ScatteringPointPositionPtr = Frame->ScatteringPointPositionY;
//		ScatteringPointAmplitude   = ScatteringPointAmplitudeY;
//	}
//	else if(AngleDirection == PHI)
//	{
//		ScatteringPointPositionPtr = Frame->ScatteringPointPositionZ;
//		ScatteringPointAmplitude   = ScatteringPointAmplitudeZ;
//	}
//	else
//	{
//		System_abort("Error occurred to select AngleDirectionEnum. \n");
//	}
//
//	/* Initialize echo phase, which is relative to scattering point position. */
//	for(i = 0 ; i < SCATTERTING_POINT_NUM ; i++)
//	{
//		EchoPhase[i] = ScatteringPointPositionPtr[i] * (LIGHT_SPEED / 2 / FPGA_CLK_FRE) / (LIGHT_SPEED / WAVE_FRE) * 2 * PI;
//	}
//
//	/* There are 10 scattering points. */
//	for(i = 0 ; i < SCATTERTING_POINT_NUM ; i++)
//	{
//		for(j= 0 ; j < SCATTERTING_POINT_NUM ; j++)
//		{
//			LineDeviationMember = LineDeviationMember +
//					ScatteringPointAmplitude[j] * ScatteringPointAmplitude[i] * ScatteringPointPositionPtr[j] * (LIGHT_SPEED / 2 / FPGA_CLK_FRE) *
//						cos(EchoPhase[i] - EchoPhase[j] );
//		}
//	}
//	for(i = 0 ; i < SCATTERTING_POINT_NUM ; i++)
//	{
//		for(j = 0 ; j < SCATTERTING_POINT_NUM ; j++)
//		{
//			LineDeviationDenominator = LineDeviationDenominator +
//								ScatteringPointAmplitude[j] * ScatteringPointAmplitude[i] *
//									cos(EchoPhase[i] - EchoPhase[j] );
//		}
//	}
//
//	return (LineDeviationMember/LineDeviationDenominator);
//}


int						MsgID = 0;

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
//	pBuf = (void* )malloc(1024);	//设网口帧数据最大不会超过150

	UdpRecvLen = recvncfrom( s, (void **)&pBuf, MSG_WAITALL, (PSA)&SocketAddrData, &plen, &hBuffer );

//	//检测当前模式是否支持收到的帧
//	if(CurrentMode == SLEEP_MODE)
//	{
//		if(*((char*)pBuf) == WORK_PARAM_SET)
//		{
//
//		}
//	}

	//----建立通信----
	if(*((char*)pBuf) == COMMUNICATION_CHECK)
	{
		sendto(s, pBuf, UdpRecvLen, MSG_WAITALL, (PSA)&SocketAddrData, plen);
	}
	//----工作使能----
	else if(*((char*)pBuf) == WORK_ENABLE)
	{
//		Msg0To1Ptr->FrameId = WORK_ENABLE;
//		/* Send message to core 2. */
//		status = MessageQ_put(QueueIdCore0ToCore1, &(Msg0To1Ptr->header));
//		if (status < 0)
//		{
//			System_abort("MessageQ_put had a failure/error\n");
//		}
//		/* Notify CORE1 to receive message. */
//		status = Notify_sendEvent(PROC_ID_CORE1, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, WORK_ENABLE_NOTIFY, TRUE);
//		if (status < 0)
//		{
//			System_printf("Notify_sendEvent had a failure/error, error code is %d \n", status);
//			System_abort(" ");
//		}
//		sendto(s, pBuf, UdpRecvLen, MSG_WAITALL, (PSA)&SocketAddrData, plen);
	}
	//----系统休眠----
	else if(*((char*)pBuf) == SLEEP_ENABLE)
	{
//		Msg0To1Ptr->FrameId = SLEEP_ENABLE;
//		/* Send message to core 2. */
//		status = MessageQ_put(QueueIdCore0ToCore1, &(Msg0To1Ptr->header));
//		if (status < 0)
//		{
//			System_abort("MessageQ_put had a failure/error\n");
//		}
//		/* Notify CORE1 to receive message. */
//		status = Notify_sendEvent(PROC_ID_CORE1, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, SLEEP_ENABLE_NOTIFY, TRUE);
//		if (status < 0)
//		{
//			System_printf("Notify_sendEvent had a failure/error, error code is %d \n", status);
//			System_abort(" ");
//		}
//		sendto(s, pBuf, UdpRecvLen, MSG_WAITALL, (PSA)&SocketAddrData, plen);
	}
	//----导入散射点----
	else if(*((char*)pBuf) == SCATTERING_POINT_INPUT)
	{
		ScatteringPointUdpFrame* ScatteringPointUdpFramePtr = (ScatteringPointUdpFrame*)malloc(sizeof(ScatteringPointUdpFrame));
//		memcpy(ScatteringPointUdpFramePtr, pBuf, 8);
//		int size = ScatteringPointUdpFramePtr->ScatteringPointData.PointNum;
//		memcpy(ScatteringPointUdpFramePtr, pBuf, sizeof(ScatteringPointUdpFrame));	//不这样操作会在读取时出问题，可能是因为cache
		memcpy(ScatteringPointUdpFramePtr, pBuf, 100);	//不这样操作会在读取时出问题，可能是因为cache

		memcpy(	ScatteringPointPtr,
				&(ScatteringPointUdpFramePtr->ScatteringPointData),
				sizeof(ScatteringPointUdpFramePtr->ScatteringPointData.PointNum) +
								sizeof(Point) * (ScatteringPointUdpFramePtr->ScatteringPointData.PointNum));

		status = Notify_sendEvent(PROC_ID_CORE1, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, QMSS_RDY_NOTIFY, TRUE);

		//回传数据
		char success = 0x01;
		sendto(s, &success, sizeof(char), MSG_WAITALL, (PSA)&SocketAddrData, plen);

		free(ScatteringPointUdpFramePtr);
	}
	//----工作参数设置----
	else if(*((char*)pBuf) == WORK_PARAM_SET)
	{
		WorkParamUdpFrame* WorkParamUdpFramePtr = (WorkParamUdpFrame*)malloc(sizeof(WorkParamUdpFrame));
		memcpy(WorkParamUdpFramePtr, pBuf, sizeof(WorkParamUdpFrame));	//不这样操作会在读取时出问题，可能是因为cache


		/* 提取目标参数 */
		//点目标
		if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == POINT_TARGET)
		{
			Msg0To2Ptr->TargetFrameId = WorkParamUdpFramePtr->FrameId;
			memcpy(&(Msg0To2Ptr->TargetParam), &(WorkParamUdpFramePtr->TargetFrame), sizeof(PointTargetParam));
//			TargetParamSize = sizeof(PointTargetParam);
		}
		//扩展目标,参数0
		else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_0)
		{
			Msg0To2Ptr->TargetFrameId = WorkParamUdpFramePtr->FrameId;
			memcpy(&(Msg0To2Ptr->TargetParam), &(WorkParamUdpFramePtr->TargetFrame), sizeof(RangeSpreadTargetParam0));
//			TargetParamSize = sizeof(RangeSpreadTargetParam0);
		}
		//扩展目标,参数1
		else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_1)
		{
			Msg0To2Ptr->TargetFrameId = WorkParamUdpFramePtr->FrameId;
			memcpy(&(Msg0To2Ptr->TargetParam), &(WorkParamUdpFramePtr->TargetFrame), sizeof(RangeSpreadTargetParam1));
//			TargetParamSize = sizeof(RangeSpreadTargetParam1);
		}
		//扩展目标,参数2
		else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_2)
		{
			Msg0To2Ptr->TargetFrameId = WorkParamUdpFramePtr->FrameId;
			memcpy(&(Msg0To2Ptr->TargetParam), &(WorkParamUdpFramePtr->TargetFrame), sizeof(RangeSpreadTargetParam2));
//			TargetParamSize = sizeof(RangeSpreadTargetParam2);
		}

		//发送噪声功率
		Msg0To2Ptr->NoisePower = WorkParamUdpFramePtr->NoisePower;

		/* Send message to core 2. */
		status = MessageQ_put(QueueIdCore0ToCore2, &(Msg0To2Ptr->header));
		if (status < 0)
		{
			System_abort("MessageQ_put had a failure/error\n");
		}


		/* 提取干扰参数  */
//		char *JammingPtrTemp = *((char*)(pBuf) + 1 + TargetParamSize);
//		HyplinkDataDsp1ToDsp2Ptr->JammingFrameId = *JammingPtrTemp;
//		//Copy the value.
//		memcpy((&(HyplinkDataDsp1ToDsp2Ptr->JammingParam)), JammingPtrTemp + 1, sizeof(HyplinkDataDsp1ToDsp2Ptr->JammingParam));
//		//无干扰
//		if(*JammingPtrTemp == NO_JAMMING)
//		{
//
//		}
//		//间歇采样转发，参数0
//		else if(*JammingPtrTemp == ISRJ_0)
//		{
//
//		}
//		//间歇采样转发，参数1
//		else if(*JammingPtrTemp == ISRJ_1)
//		{
//
//		}
//		//间歇采样转发，参数2
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

		/* 需要回传数据  */
		if(((*((char*)pBuf + 1) >> 16) == MANUAL_MODE)||((*((char*)pBuf + 1) >> 16) == AUTO_MODE_PASS_BACK))
		{
			WorkParamSetBack		WorkParamSetBackFrame;

			//点目标
			if((WorkParamUdpFramePtr->TargetFrameId&0x0f) == POINT_TARGET)
			{
				WorkParamSetBackFrame.TargetFrameId = WorkParamUdpFramePtr->FrameId;
				WorkParamSetBackFrame.TargetParamBack.PointTargetBack = 0x01;
			}
			//扩展目标,参数0
			else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_0)
			{
				//等待接收回传
				while(MessageQ_get(MessageQCore2ToCore0, (MessageQ_Msg *)&Msg2To0Ptr, MessageQ_FOREVER) != 0);
				WorkParamSetBackFrame.TargetFrameId = WorkParamUdpFramePtr->FrameId;
				WorkParamSetBackFrame.TargetParamBack.RangeSpreadTargetParam0SetBackFrame = Msg2To0Ptr->TargetParamBack.RangeSpreadTargetParam0SetBackFrame;
			}
			//扩展目标,参数1
			else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_1)
			{
				//等待接收回传
				while(MessageQ_get(MessageQCore2ToCore0, (MessageQ_Msg *)&Msg2To0Ptr, MessageQ_FOREVER) != 0);
				WorkParamSetBackFrame.TargetFrameId = WorkParamUdpFramePtr->FrameId;
				WorkParamSetBackFrame.TargetParamBack.RangeSpreadTargetParam12SetBackFrame = Msg2To0Ptr->TargetParamBack.RangeSpreadTargetParam12SetBackFrame;
			}
			//扩展目标,参数2
			else if((WorkParamUdpFramePtr->TargetFrameId&0x0000ffff) == RANGE_SPREAD_TARGET_2)
			{
				//等待接收回传
				while(MessageQ_get(MessageQCore2ToCore0, (MessageQ_Msg *)&Msg2To0Ptr, MessageQ_FOREVER) != 0);
				WorkParamSetBackFrame.TargetFrameId = WorkParamUdpFramePtr->FrameId;
				WorkParamSetBackFrame.TargetParamBack.RangeSpreadTargetParam12SetBackFrame = Msg2To0Ptr->TargetParamBack.RangeSpreadTargetParam12SetBackFrame;
			}

			//无干扰
			if(WorkParamUdpFramePtr->JammingFrameId == NO_JAMMING)
			{
				WorkParamSetBackFrame.JammingFrameId = WorkParamUdpFramePtr->JammingFrameId;
				WorkParamSetBackFrame.JammingParamBack.NoJammingBack = 0x01;
			}
			//间歇采样转发，参数0
			else if(WorkParamUdpFramePtr->JammingFrameId == ISRJ_0)
			{

			}
			//间歇采样转发，参数1
			else if(WorkParamUdpFramePtr->JammingFrameId == ISRJ_1)
			{

			}
			//间歇采样转发，参数2
			else if(WorkParamUdpFramePtr->JammingFrameId == ISRJ_2)
			{

			}

			//回传数据
			sendto(s, &WorkParamSetBackFrame, sizeof(WorkParamSetBack), MSG_WAITALL, (PSA)&SocketAddrData, plen);
		}

		free(WorkParamUdpFramePtr);
	}




//	if(status >= 0) //If receive new data, update UdpFrameRecv
//	{
//		memcpy(&UdpFrameRecv, pBuf, sizeof(UdpFrame));
//	}
//	//sendto( s, &UdpFrameRecv, sizeof(MnualModeFrame), 0, (PSA)&SocketAddrData, sizeof(SocketAddrData) );
//
//	/* Calculate some value that need no Reyleigh amplitude */
//	DistanceDelayCal(&UdpFrameRecv, Msg0To1Ptr);
//	Msg0To1Ptr->DopplerFrePinc = DopplerFrePincCal(UdpFrameRecv.TargetSpeed);
//	AmplitudeCal(UdpFrameRecv.ScatteringPointPowerX, UdpFrameRecv.ScatteringPointPowerY, UdpFrameRecv.ScatteringPointPowerZ,
//				Msg0To1Ptr->ScatteringPointAmplitude, ScatteringPointAmplitudeY, ScatteringPointAmplitudeZ);
//
//	/* Calculate angle deviation caused by amplitude of each scattering point.
//	 * Change the target angle according to the angle deviation */
//	LineDeviationTheta = LineDeviationCal(&UdpFrameRecv, ScatteringPointAmplitudeY, ScatteringPointAmplitudeZ, THETA);
//	LineDeviationPhi = LineDeviationCal(&UdpFrameRecv, ScatteringPointAmplitudeY, ScatteringPointAmplitudeZ, PHI);
//	AngleDeviationTheta = LineDeviationTheta / UdpFrameRecv.TargetReceiveDistance;
//	AngleDeviationPhi = LineDeviationPhi / UdpFrameRecv.TargetReceiveDistance;
//	Msg0To2Ptr->TargetAngleTheta = UdpFrameRecv.TargetAngleTheta + AngleDeviationTheta;
//	Msg0To2Ptr->TargetAnglePhi   = UdpFrameRecv.TargetAnglePhi + AngleDeviationPhi;
//	memcpy(Msg0To3Ptr, Msg0To2Ptr, sizeof(MsgCore0ToCore23456));
//	memcpy(Msg0To4Ptr, Msg0To2Ptr, sizeof(MsgCore0ToCore23456));
//	memcpy(Msg0To5Ptr, Msg0To2Ptr, sizeof(MsgCore0ToCore23456));
//	memcpy(Msg0To6Ptr, Msg0To2Ptr, sizeof(MsgCore0ToCore23456));
//
//	/* Set message ID */
//	MessageQ_setMsgId(&(Msg0To1Ptr->header), ManualMsgID);
//	MessageQ_setMsgId(&(Msg0To2Ptr->header), ManualMsgID);
//	MessageQ_setMsgId(&(Msg0To3Ptr->header), ManualMsgID);
//	MessageQ_setMsgId(&(Msg0To4Ptr->header), ManualMsgID);
//	MessageQ_setMsgId(&(Msg0To5Ptr->header), ManualMsgID);
//	MessageQ_setMsgId(&(Msg0To6Ptr->header), ManualMsgID);
//	System_printf("Msg ID is %d \n", ManualMsgID);
//	ManualMsgID++;
//
//	/*------------------- Send message to CORE1 -------------------------------*/
//	status = MessageQ_put(QueueIdCore0ToCore1, &(Msg0To1Ptr->header));
//	if (status < 0)
//	{
//	   System_abort("MessageQ_put had a failure/error\n");
//	}
//
//	/*--------------------- Send data to CORE2 ~ CORE6 -----------------------*/
//	//To Core2
//	status = MessageQ_put(QueueIdCore0ToCore2, &(Msg0To2Ptr->header));
//	if (status < 0)
//	{
//	   System_abort("MessageQ_put had a failure/error\n");
//	}
//	//To Core3
//	status = MessageQ_put(QueueIdCore0ToCore3, &(Msg0To3Ptr->header));
//	if (status < 0)
//	{
//	   System_abort("MessageQ_put had a failure/error\n");
//	}
//	//To Core4
//	status = MessageQ_put(QueueIdCore0ToCore4, &(Msg0To4Ptr->header));
//	if (status < 0)
//	{
//	   System_abort("MessageQ_put had a failure/error\n");
//	}
//	//To Core5
//	status = MessageQ_put(QueueIdCore0ToCore5, &(Msg0To5Ptr->header));
//	if (status < 0)
//	{
//	   System_abort("MessageQ_put had a failure/error\n");
//	}
//	//To Core6
//	status = MessageQ_put(QueueIdCore0ToCore6, &(Msg0To6Ptr->header));
//	if (status < 0)
//	{
//	   System_abort("MessageQ_put had a failure/error\n");
//	}
//	/* Notify CORE1 to receive message from other cores */
//	status = Notify_sendEvent(PROC_ID_CORE1, LINE_ID_CORE0_CORE1, EVENT_ID_CORE0_CORE1, MANUAL_MSG_RDY_NOTIFY, TRUE);
//	if (status < 0)
//	{
//		System_printf("Notify_sendEvent had a failure/error, error code is %d \n", status);
//		System_abort(" ");
//	}

    // Since the socket is still open, return "1"
    // (we need TimeOut leave UDP sockets open)
    return(1);
}

