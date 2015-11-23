#ifndef PROCESSES_FUNC_FACTION_STRUCT_H
#define PROCESSES_FUNC_FACTION_STRUCT_H

#include "water/common/roledef.h"

namespace func{
struct LevelItem
{
    uint16_t memberNum;
    uint16_t viceLeaderNum;//该级别可拥有的副帮主数
    uint64_t exp;   //到底该经验才可升为该级别
};

struct FactionCfg
{
    uint32_t createFactionLevel = 0;
    uint32_t applyLevel = 0;
    std::vector<LevelItem> levelItemVec;
    uint64_t propId;
    uint32_t propNum;
    MoneyType moneyId;
    uint64_t moneyNum;
    uint32_t reuseFactionDuration = 0;    //离开帮派后可再加入帮派的时间间隔(小时)
};

struct FactionInfoOfRole
{
    RoleId roleId;
    FactionId factionId;
    std::string name;
    uint32_t level;
    Job job;
    uint64_t banggong;
    uint32_t offlnTime;
};

}

#endif
