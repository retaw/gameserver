/*
 * Description: 火墙场景协议
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_FIRE_SCENE_HPP
#define PROTOCOL_RAWMSG_PUBLIC_FIRE_SCENE_HPP

#include "water/componet/coord.h"

#pragma pack(1)

using water::componet::Coord1D;

namespace PublicRaw{

struct FireScreenData
{
    Coord1D posx;
    Coord1D posy;
};

//s -> c
//周围火墙的九屏数据
struct FireAroundMe
{
    ArraySize size = 0;
    FireScreenData data[0];
};

//s -> c
//火墙消失 (九屏)
struct FireLeaveInfo
{
    ArraySize size = 0;
    FireScreenData data[0];
};

}

#pragma pack()


#endif
