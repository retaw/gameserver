#ifndef PROTOCOL_RAWMSG_PRIVATE_ROLE_SCENE_MSG_H
#define PROTOCOL_RAWMSG_PRIVATE_ROLE_SCENE_MSG_H

#include "water/common/commdef.h"
#include "water/common/scenedef.h"
#include "water/common/roledef.h"
#include "water/componet/datetime.h"

#include <cstring>

#pragma pack(1)

namespace PrivateRaw{
using namespace water;
using water::componet::TimePoint;

/**************切换/进入 地图流程********************/

enum class RoleDataUsage {login, changeScene};
//session -> db
//读取角色信息
struct GetRoleData
{
    RoleDataUsage usage = RoleDataUsage::login;
    RoleId rid = 0;
    LoginId loginId = 0;        //usage == login 时有意义
    uint64_t gatewayId = 0;     //usage == login 时有意义
    SceneId newSceneId;         //usage == changeScene 时有意义
    int16_t posX;               //usage == changeScene 时有意义
    int16_t posY;               //usage == changeScene 时有意义

};

//db -> session
//返回角色数据
struct RetRoleData
{
    RetRoleData()
    {
        std::memset(this, 0, sizeof(*this));
    }
    RoleDataUsage usage = RoleDataUsage::login;
    LoginId loginId = 0;
    RoleBasicData basic;
    char account[ACCOUNT_BUFF_SZIE];
    uint32_t level;
	uint64_t exp;
	TurnLife turnLife;			//转生(1转、2转...)
    uint8_t dir;                //面向
	uint32_t mp;
    uint32_t hp;
	SceneId sceneId;			//usage == changeScene 时有意义
    int16_t posX;               //usage == changeScene 时有意义
    int16_t posY;               //usage == changeScene 时有意义
    SceneId preSceneId;
    int16_t prePosX;
    int16_t prePosY;
    uint64_t gatewayId;
	bool dead;
	TimePoint deathTime;		//角色死亡时间
	uint32_t totalOnlineSec;	//角色累计在线时间

	uint64_t curObjId;			//当前已使用的objId
	uint16_t unlockCellNumOfRole;
	uint16_t unlockCellNumOfHero;
	uint16_t unlockCellNumOfStorage;
	
	uint64_t money_1;			//绑定金币
	uint64_t money_2;			//非绑金币
	uint64_t money_3;			//绑定元宝
	uint64_t money_4;			//非绑元宝
	uint64_t money_5;			//声望
    uint64_t money_6;			//强化值
	uint64_t money_7;			//战功
	uint64_t money_8;			//角色灵力值
	uint64_t money_9;			//英雄灵力值
	uint64_t money_10;			//龙魂

    Job defaultCallHero;
    TimePoint recallHeroTime;
    uint32_t offlnTime;
    uint16_t evilVal;
    uint8_t attackMode;
    TimePoint greynameTime;		//灰名开始时间
	uint8_t guanzhiLevel;		//官职等级
	bool summonHero;
    uint8_t rideState;
    uint32_t petTplId;          //宠物ID
    uint32_t petLevel;
    uint16_t anger;     //怒气
    FactionId factionId;
    char factionName[NAME_BUFF_SZIE];
    FactionPosition factionPosition;
    uint32_t factionLevel;

	uint16_t size;
    uint8_t buf[0];//用于保存下面的所有变长数据，在world进程的role::getAndInitVariableData统一反序列，统一使用
};

//world -> db
//角色离开场景时，将数据写入db
struct SaveOffline
{
    RoleId rid = 0;
    SceneId sceneId;
    uint8_t dir;
    int16_t posX;
    int16_t posY;
    SceneId preSceneId;
    int16_t prePosX;
    int16_t prePosY;
    uint32_t mp;
    uint32_t hp;
	bool dead;
	TimePoint deathTime;		//角色死亡时间
	uint32_t totalOnlineSec;	//角色累计在线时间
    uint32_t offlnTime;
    uint16_t evilVal;
    uint8_t attackMode;
    TimePoint greynameTime;
	bool summonHero;
    uint8_t rideState;
	uint64_t curObjId;	//当前已使用的objId
    uint32_t petTplId;  //宠物tplId
    uint32_t petLevel;
    uint16_t anger;     //怒气
};

//session -> world
//角色进入场景
struct RoleIntoScene : public RetRoleData
{
};

//world -> session
//进入场景的返回
struct RetRoleIntoScene
{
    RoleId rid;
    uint64_t gatewayId;
	SceneId sceneId;
    bool isSuccessful;
};

//session -> world
//检查secene是否可以进入
struct CheckScene
{
    RoleId rid;
    SceneId sceneId;
};

// world -> session
// 要求跳场景
struct RoleChangeScene
{
    RoleId rid;
    SceneId newSceneId;
    int16_t posX;
    int16_t posY;
};


//world -> session
//请求跳到指定玩家所在场景
struct RoleGotoTargetRoleScene
{
    RoleId rid;
    char targetName[NAME_BUFF_SZIE];
};

//session -> world
//session向world请求targetName的sceneId, pos
struct SessionReqTargetRoleScenePos
{
    RoleId rid;
    RoleId targetRoleId;
};

//world -> session
//world返回targetName的sceneId, pos
struct WorldRetTargetRoleScenePos
{
    RoleId rid;
    SceneId targetSceneId;
    int16_t targetPosx;
    int16_t targetPosy;
};

//world -> session
//world请求拉玩家进入场景
struct WorldCatchRole
{
    RoleId rid;
    SceneId newSceneId;
    int16_t newPosx;
    int16_t newPosy;
    char targetName[NAME_BUFF_SZIE];
};

//session / func -> world
//session / func 请求切换场景
struct SessionFuncRoleReqChangeScene
{
    RoleId rid;
    SceneId newSceneId;
    int16_t newPosx;
    int16_t newPosy;
};


// session -> func
// session 同步角色数据到 func
struct SyncOnlineRoleInfoToFunc
{
    RoleId id;
    char name[NAME_BUFF_SZIE];
    char account[ACCOUNT_BUFF_SZIE];
    uint32_t level;
    Job job;
    uint64_t gatewayId;
    uint64_t worldId;
	SceneId sceneId;
};

//func -> session
//fun请求同步session的角色数据
struct FuncQuestSyncAllOnlineRoleInfo
{
};

//session -> func
//session请求同步func的角色数据
struct SessionSyncAllOnlineRoleInfoToFunc
{
    ArraySize size;
    SyncOnlineRoleInfoToFunc roleDataList[0];
};

//world -> db
//缓存中毒时攻击者属性
struct CachePoisonAttr
{
    RoleId roleId;
    PKId id;
    uint8_t sceneItem;
    Job job;
    Job ownerJob;
    PKAttr data;
};

/********************角色货币*************************/
//world -> db
//world请求更新角色的货币
struct UpdateRoleMoney
{
	RoleId roleId;
	MoneyType type;
	uint64_t money;
};



/********************升级经验流程*************************/
//world -> db
//world请求更新角色的等级及经验
struct UpdateRoleLevelExp
{
    RoleId rid;
	uint32_t level;
	uint64_t exp;
};

//world -> db
//world请求更新角色的转生等级
struct UpdateRoleTurnLifeLevel
{
	RoleId roleId;
	TurnLife turnLifeLevel;
};


/********************副本创建与销毁流程*************************/

//* -> s
//其他进程请求session创建一个副本
struct ReqCreateDynamicSceneToSession
{
    uint64_t key;
    MapId mapId;
};

//s->*
//session通知world是否创建成功
struct RetCreateDynamicSceneToOther
{
    uint64_t key;
    SceneId sceneId;
};

//* -> s
//其他进程通知session销毁场景
struct ReqDestroyDynamicSceneToSession
{
    SceneId sceneId;
};

//s -> w
//session通知world建立一个副本
struct CreateDynamicScene
{
    SceneId sceneId;
};

//w -> s
//world通知session地图的创建结果
struct RetCreateDynamicScene
{
    bool isSuccessful;
    SceneId sceneId;
};

//s -> w
//session 通知 world 卸载一个副本
struct DestroyDynamicScene
{
    SceneId sceneId;
};

//w -> s
//world 请求 session 需要销毁一个scene, 保证真正的销毁流程, 是从session开始
struct QuestDestroyDynamicScene
{
    SceneId sceneId;
};

//world -> db
//world请求更新角色缓存数据
struct UpdateRoleBufferData
{
	RoleId roleId;
	ArraySize size;
	uint8_t buf[0];
};


}


#pragma pack()


#endif
