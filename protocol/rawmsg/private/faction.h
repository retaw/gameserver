#ifndef PROTOCOL_RAWMSG_PRIVATE_FACTION_H
#define PROTOCOL_RAWMSG_PRIVATE_FACTION_H

#include "water/common/roledef.h"
#include "water/common/factiondef.h"

#pragma pack(1)

namespace PrivateRaw{

//func->world,dbcached
//更新帮会id
struct UpdateFaction
{
    RoleId roleId;
    FactionId factionId;
    char factionName[NAME_BUFF_SZIE];//帮会
    FactionPosition position;
    uint32_t level;
};

//world->func
//物品捐献
struct ObjectDonate
{
    RoleId roleId;
    uint64_t exp;   //捐献经验
    uint64_t resource;  //捐献资源
    uint64_t banggong;
};

//func->world
//func可以创建帮派之后告诉world消耗对应金钱和道具
struct CreateFactionCost
{
    RoleId roleId;
    char factionName[NAME_BUFF_SZIE];
    uint64_t propId;
    uint32_t propNum;
    MoneyType moneyId;
    uint64_t moneyNum;
};

//world->func
//worl判断消耗品足够并消耗之后，告诉func创建帮派
struct CreateFaction
{
    RoleId roleId;
    char factionName[NAME_BUFF_SZIE];
};

//w->f,f->w
//同步帮贡
struct SynBanggong
{
    RoleId roleId;
    uint64_t banggong;
};

struct SysnFactionLevel
{
    RoleId roleId;
    uint32_t level;
};

}

#pragma pack()
#endif
