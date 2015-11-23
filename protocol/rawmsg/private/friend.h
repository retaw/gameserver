#ifndef PROTOCOL_RAWMSG_PRIVATE_FRIEND_H
#define PROTOCOL_RAWMSG_PRIVATE_FRIEND_H

#include "water/common/roledef.h"
#include "water/common/frienddef.h"

#pragma pack(1)

namespace PrivateRaw{

//添加仇人
//world->func,func->db
struct InsertEnemy
{
    RoleId roleId;
    RoleId enemyId;
    BeKilledType beKilledType;  //1：死的是角色，2：死的是英雄
};
//world->func，实时更新level
struct UpdateFuncAndSessionRoleLevel
{
    RoleId roleId;
    uint32_t level;
};
//func->world,同步enemy信息
struct UpdateWorldEnemy
{
    RoleId roleId;
    ArraySize size = 0;
    uint8_t buf[0];
};

//func->world,同步黑名单
struct UpdateWorldBlack
{
    RoleId roleId;
    ArraySize size = 0;
    uint8_t buf[0];
};

}

#pragma pack()
#endif
