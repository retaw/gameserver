/*
 * Author: zhupengfei
 *
 * Created: 2015-05-23 13:30 +0800
 *
 * Modified: 2015-05-23 13:30 +0800
 *
 * Description: 物品相关的一些类型和常量定义
 */


#ifndef WATER_COMMON_OBJDEF_HPP
#define WATER_COMMON_OBJDEF_HPP

#include "processes/world/pkdef.h"
#include "water/common/commdef.h"

#include <stdint.h>
#include <string>
#include <vector>

const uint32_t STRONG_EQUIP_BASE_PROP_NUM = 1000;   //装备强化成功率基数

//物品是否绑定
enum class Bind : uint8_t
{
	none  = 0,	//空, 表示非法值
	no	  = 1,	//不绑定
	yes   = 2,	//绑定
};

//物品父类型
enum class ObjParentType : uint16_t
{
	equip	= 100,	//装备
	stone	= 200,	//宝石
	drug	= 300,	//药品
	normal	= 400,	//普通
	use		= 500,	//可使用类物品(卷轴、券、礼包)
	money	= 600,	//货币

};

//物品子类型
enum class ObjChildType : uint16_t
{
	none				= 0,
                    
	//装备类        	
	weapon				= 101,	//武器
	clothes				= 102,	//衣服 
	helmet				= 103,	//头盔
	belt				= 104,	//腰带
	shoes				= 105,	//鞋子
	necklace			= 106,	//项链
	braceletLeft		= 107,	//手镯左
	braceletRight		= 108,	//手镯右
	ringLeft			= 109,	//戒指左
	ringRight			= 110,	//戒指右
	wing				= 111,	//翅膀
                    
	//宝石类        	
	stone_pAttack		= 201,	//物攻宝石
	stone_mAttack		= 202,	//魔攻宝石
	stone_witch			= 203,	//道术宝石
	stone_pDefence  	= 204,	//物防宝石
	stone_mDefence  	= 205,	//魔防宝石
	stone_hp			= 206,	//生命宝石
	stone_mp			= 207,	//魔法宝石
	stone_shot			= 208,  //命中宝石
	stone_pEscape   	= 209,	//物闪宝石
	stone_mEscape   	= 210,	//魔闪宝石
	stone_crit      	= 211,	//暴击宝石
	stone_antiCrit  	= 212,	//防爆宝石
	stone_hpLv			= 213,	//生命等级宝石
	stone_mpLv			= 214,	//魔法等级宝石
                    
	//药品类        	
	drug_addBuff		= 301,	//使用后获得buff
	drug_addRoleExp		= 302,	//主角经验丹
	drug_addHeroExp 	= 303,	//英雄经验丹
    
	//可使用类物品(卷轴、券、礼包)
	use_gift_fixed		= 501,	//固定礼包
	use_gift_random		= 502,	//随机礼包
	use_backCity		= 503,	//回城卷轴
	use_randomPos		= 504,  //随机卷轴     	
	use_money_1			= 505,	//绑定金币券
	use_money_2			= 506,  //非绑金币券
	use_money_3			= 507,  //绑定元宝券
	use_money_4			= 508,  //非绑元宝券
    use_expSec			= 509,	//自动加经验类型的剩余时间
	use_bonfire			= 510,	//召唤篝火
	use_wine			= 511,	//喝酒(女儿红)

	//货币类        	
	money_1				= 601,	//绑定金币
	money_2				= 602,	//非绑金币
	money_3				= 603,	//绑定元宝
	money_4				= 604,	//非绑元宝
	money_5				= 605,	//声望
	money_6				= 606,  //强化值
	money_7				= 607,	//战功
	money_8				= 608,	//角色灵力值
	money_9				= 609,	//英雄灵力值

    //经验和货币直接使用类
    role_exp_usage      = 701,  //主角直接加经验
    hero_exp_usage      = 702,  //英雄直接加经验
    money_usage         = 703,  //直接加货币

};

enum class BroadCast : uint8_t
{
    none    = 0,
    world   = 1,    //全服
};


#pragma pack(1)

//装备强化属性
struct StrongPropItem
{
	uint32_t level = 0;	//等级
	PropertyType propType = PropertyType::none;	//属性类型
	uint32_t prop = 0;	//属性值
};

//装备分解产出
struct FenjieRewardItem
{
	TplId	 tplId;
	uint32_t num;
	Bind	 bind;
	uint32_t prob;
};

//基本物品数据信息
struct ObjBasicData
{
	uint64_t	objId;			//道具唯一Id
	uint32_t    skillId;		//极品装备触发的被动技能Id(几率触发)
	uint8_t     strongLevel;	//强化等级
	uint8_t		luckyLevel;		//武器幸运等级

	TplId		tplId;			//道具类型Id
	std::string name;			//道具名字
	uint16_t	childType;		//道具子类型
	uint8_t		quality;		//道具品质(1、2、3、4、5)
	uint8_t		turnLife;		//转生等级限制
	uint8_t		level;			//等级限制
	uint8_t		job;			//职业限制
	uint8_t		sex;			//性别限制
	uint8_t     needMoneyType;	//使用道具消耗货币类型
	uint32_t    needMoneyNum;	//使用道具消耗货币数量
	uint16_t	maxStackNum;	//最大可叠加数量
	uint8_t	    moneyType;		//货币类型(为空时不允许出售)
	uint32_t	price;			//出售价格(为空时不允许出售)
	uint8_t		lifeType;		//计算寿命类型(获取时计算，使用时计算，服务器时间计算)
	uint32_t	lifeSpan;		//使用寿命(获取时，使用时，服务器时间)
	bool		bBatUse;		//可否批量使用
	bool		bDiscard;		//可否丢弃
	uint32_t	prob;			//爆出此道具的概率(万分比)
	uint32_t    objLevel;		//物品等级

	uint32_t	p_attackMin;	//物攻Min
	uint32_t	p_attackMax;	//物攻Max
	uint32_t	m_attackMin;	//魔攻Min
	uint32_t	m_attackMax;	//魔攻Max
	uint32_t	witchMin;		//道术Min
	uint32_t	witchMax;		//道术Max
	uint32_t	p_defenceMin;	//物防Min
	uint32_t	p_defenceMax;	//物防Max
	uint32_t	m_defenceMin;	//魔防Min
	uint32_t	m_defenceMax;	//魔防Max

	uint32_t 	hp;				//生命
	uint32_t 	mp;				//魔法
	uint32_t 	shot;			//命中
	uint32_t 	p_escape;		//物闪
	uint32_t 	m_escape;		//魔闪
	uint32_t 	crit;			//暴击
	uint32_t 	antiCrit;		//防爆
	uint32_t 	lucky;			//幸运
	uint32_t 	evil;			//诅咒
	uint32_t 	critDamage;		//暴伤
	uint32_t 	shotRatio;		//命中率
	uint32_t 	escapeRatio;	//闪避率
	uint32_t 	critRatio;		//暴击率
             	
	uint32_t 	hpLv;			//生命等级
	uint32_t 	mpLv;			//魔法等级
	uint32_t 	damageAdd;		//增伤
	uint32_t 	damageReduce;	//减伤
	uint32_t 	damageAddLv;	//增伤等级
	uint32_t 	damageReduceLv;	//减伤等级
    uint32_t    antiDropEquip;  //防爆(装备掉落)
	uint32_t 	suitId;			//套装Id
	uint32_t 	nonsuchId;		//极品Id

	std::vector<StrongPropItem>		strongPropVec;		//装备强化等级属性
	std::vector<FenjieRewardItem>	fenjieRewardVec;	//装备分解产出

    union
    {
	    uint32_t 	sep1;		//特殊1 预留
        uint32_t    horseExp;   //消耗道具加坐骑经验
        uint32_t    energe;     //消耗道具增加的龙心能量
    };
	uint32_t 	sep2;			//特殊2
	uint32_t 	sep3;			//特殊3
    BroadCast   broadCast;
};

//奖励数据信息
struct RewardData 
{
	TplId		tplId;  
	uint32_t	num; 
	Bind		bind;     
	uint8_t		job; 
	uint32_t	minLevel;
	uint32_t	maxLevel; 
};

//服务器内部使用的道具参数数据结构
struct ObjItem
{
	TplId    tplId		= 0;;
	uint32_t num		= 0;;
	Bind     bind		= Bind::none;
	uint32_t skillId	= (uint32_t)-1;
	uint8_t	strongLevel	= 0;
	uint8_t	luckyLevel	= 0;
};

#pragma pack()

#endif
