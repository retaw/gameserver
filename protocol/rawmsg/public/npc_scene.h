/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-14 17:20 +0800
 *
 * Modified: 2015-05-14 17:20 +0800
 *
 * Description: 
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_NPC_SCENE_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_NPC_SCENE_MSG_H

#include "water/common/commdef.h"
#include "water/common/scenedef.h"

#pragma pack(1)

namespace PublicRaw{


//s -> c
//更新视野中npc的数据
struct NpcScreenData
{
    PKId id;
    uint32_t tplId;
    char name[NAME_BUFF_SZIE];
    uint32_t level;
    int16_t posX;
    int16_t posY;
    uint32_t maxhp;
    uint32_t hp;
    uint8_t dir;
    uint32_t pkStatus;
};

//s -> c
//周围全部npc的9屏数据
struct NpcsAroundMe
{
    ArraySize size = 1;
    NpcScreenData npcs[1];
};

//s -> c
//npc离开role的视野
struct NpcLeaveInfo
{
    ArraySize size = 1;
    PKId id[1];
};

//s -> c
//npc位置改变
struct UpdateNpcPosToClient
{
    PKId npcId;
    Coord1D posX;
    Coord1D posY;
    uint8_t dir;
    MoveType type;
};

//s -> c
//npc死亡通知
struct NpcDieInfo
{
    PKId id         = 0;
};

}

#pragma pack()

#endif
