/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-14 14:43 +0800
 *
 * Modified: 2015-04-14 14:43 +0800
 *
 * Description: 角色相关的一些类型和常量定义
 */


#ifndef WATER_COMMON_ROLEDEF_H
#define WATER_COMMON_ROLEDEF_H


#include "commdef.h"
#include "objdef.h"
#include <cstring>
#include "frienddef.h"
#include "maildef.h"
#include "factiondef.h"
#include "taskdef.h"

#include <vector>

typedef uint64_t LoginId;
//LoginId 从高位到低位的分割: | 16bits reserve | 16bits gateway pid | 32bits 自增值 |


//RoleId
typedef PKId RoleId;

//RoleId组成, 从高到低位分割：| 8bit reserve | 8bits platform | 16bits zone | 32bits 自增值 |
const RoleId ROLE_PLATFORM_MASK = 0x00ff000000000000;
const RoleId ROLE_ZONE_MASK     = 0x0000ffff00000000;
const RoleId ROLE_AUTO_INC_MASK = 0x00000000ffffffff;
//标准的无效roleId
const RoleId INVALID_ROLE_ID = 0;

//背包相关
const uint8_t MAX_PACKAGE_NUM_OF_ROLE = 6;		//归属角色背包集的最大背包数
const uint8_t MAX_PACKAGE_NUM_OF_HERO = 2;		//归属各个英雄背包集的最大背包数

const uint16_t DEFAULT_UNLOCK_CELL_NUM_OF_ROLE = 42;	//角色背包默认开启格子数
const uint16_t DEFAULT_UNLOCK_CELL_NUM_OF_HERO = 42;	//英雄背包默认开启格子数
const uint16_t DEFAULT_UNLOCK_CELL_NUM_OF_STORAGE = 70;	//仓库默认开启格子

const uint16_t MAX_CELL_NUM_OF_ROLE = 140;		//角色背包的最大格子数
const uint16_t MAX_CELL_NUM_OF_HERO = 100;		//英雄背包的最大格子数
const uint16_t MAX_CELL_NUM_OF_STORAGE = 210;	//仓库的最大格子数
const uint16_t MAX_CELL_NUM_OF_EQUIP = 11;		//装备包的最大格子数
const uint16_t MAX_CELL_NUM_OF_REPURCHASE = 10; //回购背包的最大格子数
const uint16_t MAX_CELL_NUM_OF_STONE = 44;		//宝石包的最大格子数

//官职
const uint8_t MAX_GUANZHI_LEVEL = 15;

//背包
enum class PackageType : uint8_t
{
	none		= 0,	//非法值

	//归属角色背包集
	role		= 1,	//角色背包
	hero		= 2,	//英雄背包
	storage		= 3,	//仓库
	equipOfRole	= 4,	//角色装备包
    repurchase  = 5,    //回购背包
	stoneOfRole = 6,	//角色宝石包

	//归属英雄背包集
	equipOfWarrior	= 101,	//战士英雄装备包
	stoneOfWarrior	= 102,	//战士英雄宝石包
	equipOfMagician	= 103,	//法师英雄装备包
	stoneOfMagician = 104,	//法师英雄宝石包
	equipOfTaoist	= 105,	//道士英雄装备包
	stoneOfTaoist	= 106,	//道士英雄宝石包
};

//DB操作类型
enum class ModifyType : uint8_t
{
	none	= 0,
	erase	= 1,	//删除
	modify	= 2,	//修改
};

//职业
enum class Job : uint8_t
{
	none	 = 0,	//不限制职业
    warrior  = 1,   //战士
    magician = 2,	//法师
	taoist	 = 3,	//道士
};

//性别
enum class Sex : uint8_t
{
	none   = 0,		//不限制性别
    male   = 1,
    female = 2,
};

//转生
enum class TurnLife : uint8_t
{
	zero	= 0,	// 0转
	one		= 1,	// 1转
	two		= 2,    // 2转
	three	= 3,	// 3转
	four	= 4,	// 4转
	five	= 5,	// 5转
};

//移动类型
enum class MoveType : uint8_t
{
    none        = 0,
    walk        = 1,
    run         = 2,
    chongfeng   = 3,    //冲锋
    hitback     = 4,    //击退
	blink		= 5,	//瞬移
};

//死亡复活
enum class ReliveType : uint8_t
{
	reliveArea	= 0,	//复活点复活(地图没有复活点,就回主城复活)
	perfect 	= 1,	//完美原地复活
	weak	    = 2,	//虚弱原地复活
};

//货币类型
enum class MoneyType : uint8_t
{
    none    = 0,
	money_1 = 1,	//绑定金币
	money_2 = 2,	//非绑金币
	money_3 = 3,	//绑定元宝
	money_4 = 4,	//非绑元宝
	money_5 = 5,	//声望
	money_6 = 6,    //强化值
	money_7 = 7,	//战功
	money_8	= 8,	//角色灵力值
	money_9	= 9,	//英雄灵力值
    money_10 = 10,  //龙魂
    money_11 = 11,  //帮贡
};

//奖励状态
enum class Reward : uint8_t
{
	none	= 0,		//没有奖励
	canGet	= 1,		//可领取
	got		= 2,		//已领取
};

//称号
enum class TitleType : uint8_t
{
	none	= 0,
	normal	= 1,	//普通称号
	special	= 2,	//特殊称号
	guanzhi	= 3,	//官职称号
};

//功能活动玩家行为操作结果
enum class OperateRetCode : uint8_t
{
	sucessful			= 1,	//成功
	failed				= 2,	//失败
	materialNotEnough	= 3,	//材料不足
};

#pragma pack(1)
//基本角色信息
struct RoleBasicData
{
    RoleBasicData()
    {
        std::memset(this, 0, sizeof(*this));
    }
    RoleId id;
    char name[NAME_BUFF_SZIE];
    Job job;
    Sex sex;
};

struct RoleScreenData : public RoleBasicData
{
    RoleScreenData()
    {
        std::memset(this, 0, sizeof(*this));
    }
    int16_t posX;
    int16_t posY;
	uint32_t level;
    uint32_t maxhp;
    uint32_t hp;
	bool	isDead;
    uint8_t dir;
    uint32_t pkStatus; //状态效果(按位取)
	
	TplId weapon;	//武器
	TplId clothes;	//衣服
	TplId wing;		//翅膀
    //队伍信息
    TeamId teamId;
    uint8_t nameColor; //名字颜色

	uint32_t titleIdOfNormal;	//普通称号ID
	uint32_t titleIdOfSpecial;	//特殊称号ID
	uint32_t titleIdOfGuanzhi;	//官职称号ID

    uint8_t rideState; //骑乘状态
    uint16_t skin;
    Job heroJob;
    Sex heroSex;
    uint32_t heroClother;
    char factionName[NAME_BUFF_SZIE];
    FactionId factionId;
	TurnLife turnLifeLevel;		//转生等级
	bool bAutoAddExp;			//是否自动加经验
    uint16_t holdBoxLeftTime;   //持有世界boss宝箱剩余时间
	uint8_t duanweiType;		//天下第一副本内段位类型
    uint8_t stall;              //1摆摊  0非摆摊
};

struct RoleMainData : public RoleBasicData
{
    RoleMainData()
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

	uint32_t level;
	uint64_t gotExp;	//累计经验值 - 当前等级需要经验值
	uint64_t needExp;	//下一级需要经验值 - 当前等级需要经验值
    uint16_t expRate;   //当前等级段经验值所占该等级升级经验值的千分比

	TplId weapon;	//武器
	TplId clothes;	//衣服
	TplId wing;		//翅膀

	uint64_t money_1;	//绑定金币
	uint64_t money_2;	//非绑金币
	uint64_t money_3;	//绑定元宝
	uint64_t money_4;	//非绑元宝
	uint64_t money_5;	//声望
	uint64_t money_6;	//强化值
	uint64_t money_7;	//战功
	uint64_t money_8;	//角色灵力值
	uint64_t money_9;	//英雄灵力值
	uint64_t money_10;	//龙魂

	Job     defaultCallHero;//默认召唤的英雄
	uint8_t guanzhiLevel;	//官职等级
    FactionId factionId;    //帮派ID
    FactionPosition factionPosition; //帮派职务
	TurnLife turnLifeLevel; //转生等级
    char factionName[NAME_BUFF_SZIE];
    uint16_t anger;     //怒气值
    uint16_t energe;    //龙心能量
};

//背包基本信息
struct RoleObjData
{
	struct ObjData
	{
		uint64_t objId;				//相对于role唯一的objID
		PackageType packageType;	//背包类型	0角色 1英雄 2仓库
		uint16_t cell;				//格子编号
		uint32_t tplId;				//物品tplId
		uint16_t item;				//物品数量
		uint32_t skillId;			//极品装备触发的被动技能Id(几率触发)
		Bind     bind;
		uint32_t sellTime;
        uint8_t strongLevel;		//强化等级
		uint8_t luckyLevel;			//武器幸运
	} data[0];
};


//技能数据(包括主角和英雄)
struct SkillData
{
    TplId    skillId;
    uint32_t skillLv;       //技能等级
    uint32_t strengthenLv;  //强化等级
    uint32_t exp;           //当前等级经验值
};



struct PKCdStatus
{
    TplId    skillId;
    uint32_t endtime;
};

//buff数据结构
struct BuffData
{
    TplId       buffId;    //id
    uint32_t    sec;       //buff剩余作用时间,单位(秒)(这主要用于下线后不走时间,与endtime互斥)
    uint32_t    endtime;   //buff结束时间(下线后继续走时间)
    uint32_t    dur;       //耐久
};

//释放者PK属性缓存
struct PKAttr
{
    PKId        attackId;   
    Job         attackJob;
    uint8_t     sceneItem;

    uint32_t    atkMin;
    uint32_t    atkMax;
    uint32_t    shot;
    uint32_t    shotRatio;
    uint32_t    lucky;
    uint32_t    evil;
    uint32_t    crit;
    uint32_t    critRatio;
    uint32_t    critDamage;
    uint32_t    damageAdd;
    uint32_t    damageAddLv;

    uint32_t    shotk;
    uint32_t    critk; //暴击k值
    uint16_t    skillPower;
    uint16_t    skillDamage;
    uint16_t    skillDamageAddPer;
    uint16_t    skillConstDamage;
};

//计数器信息
struct CounterInfo
{
    uint32_t    counterType;
    uint32_t    count;
    uint32_t    time;
};

//mail
struct MailInfo
{
    uint32_t mailIndex;
    char title[MAX_MAIL_TITLE_SIZE];    //邮件标题
    char text[MAX_MAIL_TEXT_SIZE];      //邮件内容 
    uint8_t state;
    uint32_t time;
    ObjItem obj[MAX_MAIL_OBJ_NUM];
};

//称号
struct TitleInfo
{
	uint32_t titleId;
	TitleType titleType;
	uint32_t createTime;
	uint32_t disableTime;
	bool used;
};

//洗练属性
struct WashPropInfo
{
	uint8_t washType;    
	uint8_t group;
	uint8_t quality;  
	PropertyType propType; 
	uint32_t prop;   
};

//龙珠
struct DragonBallInfo
{
	uint8_t type;
	uint32_t exp;
};

//任务
struct TaskInfo
{
    TaskId taskId = 0;
    TaskType type = TaskType::none;
    TaskState state	= TaskState::no;
	uint32_t time = 0;
	uint32_t star = 0;
	std::vector<TaskStep> steps;
};


#pragma pack()

#endif
