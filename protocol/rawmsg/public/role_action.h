/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-18 16:06 +0800
 *
 * Modified: 2015-04-18 16:06 +0800
 *
 * Description: 玩家行为消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_ROLE_ACTION_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_ROLE_ACTION_MSG_H


#include "water/common/scenedef.h"
#include "water/common/commdef.h"
#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{ 

//c -> s
//客户端要求移动到某个点
struct RoleMoveToPos
{
    Coord1D posX = 0;
    Coord1D posY = 0;
    uint8_t dir  = 1;
    MoveType type = MoveType::walk;
};


//s -> c
//广播视野内的角色移动情况
struct UpdateRolePosToClient
{
    RoleId rid   = 0;
    Coord1D posX = 0;
    Coord1D posY = 0;
    uint8_t dir  = 1;
    MoveType type = MoveType::walk;
};


//s -> c
//同步角色自己的坐标
struct SynchronizeRolePos
{
    Coord1D posX = 0;
    Coord1D posY = 0;
};

//s -> c
//返回角色的缓存数据
struct RetRoleBufData
{
	ArraySize size = 0;
	uint16_t buf[0];
};

//c -> s
//请求设置角色的缓存数据
struct RequestSetRoleBufData
{
	ArraySize size = 0;
	uint16_t buf[0];
};

}

#pragma pack()



#endif

