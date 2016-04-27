#include "../CustomHeader.h"

/* 点目标模型参数计算 */
void PointTargetCal(MsgCore0ToCore2 *Msg0To2Ptr, MsgCore2ToCore1 *Msg2To1Ptr, MsgCore2ToCore34567 *Msg2To34567Ptr);

/* 扩展目标模型，参数0计算 */
void RangeSpreadTargetParam0Cal(MsgCore0ToCore2 *Msg0To2Ptr, MsgCore2ToCore1 *Msg2To1Ptr,
		MsgCore2ToCore34567 *Msg2To34567Ptr, MsgCore2ToCore0 *Msg2To0Ptr, ScatteringPoint *ScatteringPointPtr);

/* 扩展目标模型，参数1计算 */
void RangeSpreadTargetParam1Cal(MsgCore0ToCore2 *Msg0To2Ptr, MsgCore2ToCore1 *Msg2To1Ptr,
		MsgCore2ToCore34567 *Msg2To34567Ptr, MsgCore2ToCore0 *Msg2To0Ptr, ScatteringPoint *ScatteringPointPtr);

/* 扩展目标模型，参数2计算 */
void RangeSpreadTargetParam2Cal(MsgCore0ToCore2 *Msg0To2Ptr, MsgCore2ToCore1 *Msg2To1Ptr,
		MsgCore2ToCore34567 *Msg2To34567Ptr, MsgCore2ToCore0 *Msg2To0Ptr, ScatteringPoint *ScatteringPointPtr);
