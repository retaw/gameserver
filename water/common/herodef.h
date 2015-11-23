/*
 * Author: zhupengfei
 *
 * Created: 2015-06-16 15:53 +0800
 *
 * Modified: 2015-06-16 15:53 +0800
 *
 * Description: 英雄相关的一些类型和常量定义
 */


#ifndef WATER_COMMON_HERODEF_HPP
#define WATER_COMMON_HERODEF_HPP


#include "roledef.h"
#include "commdef.h"
#include <cstring>

//HeroId
typedef PKId HeroId;

//英雄
const uint8_t MAX_HERO_NUM = 3;		//角色可拥有的最大英雄数量


#pragma pack(1)

//英雄基本信息
struct HeroBasicData
{
    HeroBasicData()
    {
        std::memset(this, 0, sizeof(*this));
    }
    RoleId roleId;
	HeroId heroId;
    Job job;
    Sex sex;
};

//英雄九屏数据
struct HeroScreenData : public HeroBasicData
{
    HeroScreenData()
    {
        std::memset(this, 0, sizeof(*this));
    }
    char roleName[MAX_NAME_SZIE+1];
    int16_t posX;
    int16_t posY;
	uint32_t level;
	uint32_t maxhp;
	uint32_t hp;
	bool isDead;
	uint8_t dir;
	
	TplId weapon;	//武器
	TplId clothes;	//衣服
	TplId wing;		//翅膀

    //名字颜色
    uint8_t nameColor;
	TurnLife turnLifeLevel;	//转生等级
    FactionId factionId;    //主人帮派id
};

//英雄主数据
struct HeroMainData : public HeroBasicData
{
    HeroMainData()
    {
        std::memset(this, 0, sizeof(*this));
    }
	uint32_t maxhp;
	uint32_t hp;
	uint32_t maxmp;
	uint32_t mp;

    uint32_t maxPAtk;   //最大物攻
    uint32_t minPAtk;   //最小物攻
    uint32_t maxMAtk;   //最大魔攻
    uint32_t minMAtk;   //最小魔攻
    uint32_t maxWitch;  //最大道术
    uint32_t minWitch;  //最小道术
    uint32_t maxPDef;   //最大物防
    uint32_t minPDef;   //最小物防
    uint32_t maxMDef;   //最大魔防
    uint32_t minMDef;   //最小魔防
    uint32_t shot;      //命中
    uint32_t pEscape;   //物理闪避
    uint32_t mEscape;   //魔法闪避
    uint32_t crit;      //暴击
    uint32_t critdmg;   //暴击伤害
    uint32_t antiCrit;  //暴击抗性(防暴)
    uint32_t lucky;     //幸运
    uint32_t pk;        //pk罪恶值
    uint32_t dmgAddLevel;//增伤等级
    uint32_t dmgReduceLevel;//减伤等级
    uint32_t hpLevel;   //生命等级
    uint32_t mpLevel;   //魔法等级

	TplId weapon;	//武器
	TplId clothes;	//衣服
	TplId wing;		//翅膀

	uint32_t level;
	uint64_t gotExp;	//累计经验值 - 当前等级需要经验值
	uint64_t needExp;	//下一级需要经验值 - 当前等级需要经验值
    uint16_t expRate;   //同role, 当前经验占该等级段经验千分比
	TurnLife turnLifeLevel;	//转生等级
};

//英雄信息
struct HeroInfoPra
{
	Job job;
	Sex sex;
	uint32_t level;
    uint64_t exp;
	uint32_t hp;
	uint32_t mp;
    TurnLife turnLife;
    uint32_t clother;
};


#pragma pack()

#endif
