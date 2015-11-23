/*
 * Author: zhupengfei
 *
 * Created: 2015-06-16 15:18 +0800
 *
 * Modified: 2015-06-16 15:18 +0800
 *
 * Description: 英雄相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_HERO_SCENE_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_HERO_SCENE_MSG_HPP

#include "water/common/herodef.h"
#include "water/componet/coord.h"

#pragma pack(1)

using water::componet::Coord1D;

namespace PublicRaw{

//s -> c
//周围英雄的九屏数据 (九屏)
struct HerosAroundMe
{
    ArraySize size = 1;
    HeroScreenData heros[1];
};

//s -> c
//英雄的详细信息
struct HeroSelfMainInfo
{
	HeroMainData info;
};

//s -> c
//英雄离开视野 (九屏)
struct HeroLeaveInfo
{
    ArraySize size = 1;
	HeroId heroId[1]; 
};

//s -> c
//将英雄的九屏数据广播到九屏 (九屏)
struct BroadCastHeroScreenDataToNine
{
	HeroScreenData info;
};

//s -> c
//英雄位置改变
struct UpdateHeroPosToClient
{
	HeroId heroId;
	Coord1D posX;
	Coord1D posY;
	uint8_t dir;
	MoveType type;
};

//c -> s
//请求已创建英雄列表
struct RequestCreatedHeroList
{
};

//s -> c
//返回已创建英雄列表
struct RetCreatedHeroList
{
	ArraySize size;
	struct CreatedHero
	{
		Job job;
		Sex sex;
	} data[0];
};

//c -> s
//请求创建英雄
struct RequestCreateHero
{
	Job job;
	Sex sex;
};

//c -> s
//请求召唤英雄
struct RequestSummonHero
{
	Job job;
};

//s -> c
//返回成功召唤的英雄(九屏)
struct RetSummonHeroToNine
{
	RoleId roleId;
	HeroId heroId;
	Job job;
};

//c -> s
//请求召回英雄
struct RequestRecallHero
{
	Job job;
};

//s -> c
//召唤倒计时
struct RetSummonHeroNeedSpanSec
{
	uint32_t needSpanSec;
};

//c -> s
//请求设置默认召唤英雄
struct RequestSetDefaultCallHero
{
	Job job;
};

//s -> c
//同步英雄当前等级获得的经验
struct RetHeroCurLevelGotExp
{
	HeroId heroId;
	uint64_t gotExp;
    uint16_t expRate;
};


//s -> c
//告知需要移动到的位置
struct CommandHeroMoveTo
{
    Coord1D posX = 0;
    Coord1D posY = 0;
};

//c -> s
//英雄移动
struct RequestHeroChangePos
{
	Job job;
	Coord1D posX = 0;
	Coord1D posY = 0;
	uint8_t dir = 1;
	MoveType type = MoveType::walk;
};

//s <-> c
//设置或确认英雄ai模式
struct HeroAIMode
{
    uint8_t aiMode = 0; // 0 跟随; 1 被动; 2 主动
};

//c <-> s
//设定或确认hero的攻击目标
struct HeroLockOnTarget
{
    PKId id = 0;
    uint8_t type = 0; //数值同SceneItemType, 即: 1 角色; 2 英雄; 3 npc(怪和boss); 4 宠物
};

//c -> s
//主角锁定的目标
struct RoleLockOnTarget
{
    PKId id = 0;
    uint8_t type = 0; //数值同SceneItemType, 即: 1 角色; 2 英雄; 3 npc(怪和boss); 4 宠物 
};

//c -> s
//设定英雄技能, 禁用的技能列表
struct HeroDisableSkillList
{
    uint32_t binSize() const
    {
        return sizeof(*this) + sizeof(skillList[0]) * size;
    }

    ArraySize size = 0;
    uint32_t skillList[0];
};


}

#pragma pack()


#endif
