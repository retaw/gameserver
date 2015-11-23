#ifndef PROTOCOL_RAWMSG_PRIVATE_TEAM_H
#define PROTOCOL_RAWMSG_PRIVATE_TEAM_H

#include "water/common/roledef.h"
#include "water/common/frienddef.h"

#pragma pack(1)

namespace PrivateRaw{

//func->world
//更新队伍信息
struct UpdateTeamInfo
{
    RoleId roleId;
    TeamId teamId;
    bool insertOrErase;
};
}

#pragma pack()
#endif
