/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-16 21:55 +0800
 *
 * Modified: 2015-04-16 21:55 +0800
 *
 * Description: 角色场景相关的消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_ROLE_SCENE_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_ROLE_SCENE_MSG_H

#include "water/common/scenedef.h"
#include "water/common/commdef.h"
#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//s -> c
//通知端进入新的场景
struct NewSceneMapId
{
    MapId mapId;
};

//s -> c
//周围玩家的信息
struct RolesAroundMe
{
    ArraySize size = 0;
    RoleScreenData roles[0];
};

//s -> c
//个人的详细信息
struct SelfMainInfo
{
    RoleMainData info;
};

//s -> c
//将玩家的九屏信息广播到九屏
struct BroadcastRoleScreenDataToNine
{
	RoleScreenData info;
};

//s -> c
//玩家离开视野
struct RoleLeaveInfo
{
    ArraySize size = 1;
    RoleId id[1];
};


//s -> c
//玩家要求通过跳转点跳场景
struct GotoOtherSceneByTransmission
{
    uint32_t transmissionId;
};

//s -> c
//跳转点条场景失败
struct GotoOtherSceneByTransmissionFailed
{
};

//s -> c
//通知端离开场景
struct RoleLeavedScene
{
};

//s -> c
//玩家进场景完成
struct RoleEnterSceneOver
{
};

//c -> s
//使用小飞鞋传送
struct RequestFeixieTransfer
{
    MapId mapId;
    Coord1D posX;
    Coord1D posY;
	bool bNeedObj = true;	//是否需要道具
};

//c -> s
//玩家进副本
struct RoleIntoCopyMap
{
    MapId mapId;
};

//c -> s
//玩家退出副本
struct RoleLeaveCopyMap
{
};

}

#pragma pack()


#endif


