#ifndef PROTOCOL_RAWMSG_PRIVATE_HERO_MSG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_HERO_MSG_HPP

#include "water/common/roledef.h"
#include "water/common/herodef.h"
#include "water/componet/datetime.h"

#include <stdint.h>

#pragma pack(1)

namespace PrivateRaw{

using namespace water; 
using water::componet::TimePoint;

//world -> db
//hero表插入英雄信息
struct InsertHeroInfo
{
	RoleId roleId = 0;
    HeroInfoPra data;
};

//world -> db
//更新当前默认召唤的英雄
struct UpdateDefaultCallHero
{
	RoleId roleId = 0;
	Job defaultCallHero = Job::none;
};

//world -> db
//更新英雄等级经验
struct UpdateHeroLevelExp
{
	RoleId roleId = 0;
	Job job = Job::none;
	uint32_t level = 0;
	uint64_t exp = 0;
};

//world ->db
//更新英雄转生等级
struct UpdateHeroTurnLifeLevel
{
	RoleId roleId = 0;
	Job job = Job::none;
	TurnLife turnLifeLevel = TurnLife::one;
};

//world -> db
//保存英雄下线数据
struct SaveHeroOffline
{
	RoleId roleId = 0;
	Job job = Job::none;
	uint32_t hp = 0;
	uint32_t mp = 0;
	TimePoint recallTimePoint = componet::EPOCH;
    uint32_t clother;
    uint32_t petTplId;
};

//world -> db
struct UpdateHeroClothes
{
    RoleId roleId = 0;
    Job job = Job::none;
    uint32_t clother;
};

//world -> db
//请求英雄序列化数据(包括背包,技能,buff等等),类似与RetRoleData  
struct ReqHeroSerializeData
{
    RoleId roleId;
    Job job;
};

//db -> world
//返回英雄的序列化数据
struct RetHeroSerializeData
{
    RoleId roleId;
    Job job;
    uint16_t size;
    uint8_t buf[0]; //序列化数据
};


}


#pragma pack()

#endif
