/*
 * 机关场景消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_TRIGGER_SCENE_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_TRIGGER_SCENE_MSG_H


#include "water/common/commdef.h"
#include "water/componet/coord.h"

using water::componet::Coord1D;

#pragma pack(1)

namespace PublicRaw{

struct TriggerScreenData
{
    uint16_t triggerTplId;
    TriggerId id;
    Coord1D posx;
    Coord1D posy;
    uint16_t reserve;   //预留
};

//s -> c
//周围机关九屏数据
struct TriggerAroundMe
{
    ArraySize size = 0;
    TriggerScreenData data[0];
};

//s -> c
//机关离开 九屏
struct TriggerLeaveInfo
{
    ArraySize size = 0;
    TriggerScreenData data[0];
};

//c -> s
//触发机关事件
struct TouchTrigger
{
    TriggerId id;
};

}

#pragma pack()

#endif
