/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-22 15:18 +0800
 *
 * Modified: 2015-04-22 15:18 +0800
 *
 * Description: 
 */

#ifndef PROCESS_DBCACHED_ROLE_H
#define PROCESS_DBCACHED_ROLE_H

#include "common/roledef.h"
#include "common/scenedef.h"
#include "common/herodef.h"

#include "water/process/process_id.h"
#include "water/componet/class_helper.h"
#include "water/componet/datetime.h"
#include "water/process/tcp_message.h"
#include <unordered_map>
#include <set>

namespace dbcached{

using namespace water;
using namespace water::componet;
using water::process::ProcessIdentity;
using water::componet::EPOCH;
using water::componet::TimePoint;
using water::process::TcpMsgCode;

class Hero
{
    friend class RoleManager;
    friend class ContrRoleContainer;
public:
    TYPEDEF_PTR(Hero);
    CREATE_FUN_NEW(Hero);
    ~Hero() = default;
private:
    Hero(Job job);

    //skill数据
    std::unordered_map<uint32_t, SkillData> m_skillDataBykeyMap;
    //object数据
    std::unordered_map<uint64_t, RoleObjData::ObjData> m_objDataByKeyMap;
    std::vector<PKCdStatus> pkCdStatusVec;//pk中技能cd的临时状态，只缓存
    //buff
    std::unordered_map<uint32_t, BuffData> m_buffMap;
	//洗练
	std::unordered_map<uint8_t, std::vector<WashPropInfo> > m_WashPropMap;

    //中施毒术攻击者的属性
    PKAttr  m_poisonAttackerAttr;

    //宠物tplId缓存
    uint32_t    m_petTplId = 0;
    //宠物中施毒术时攻击者属性缓存
    PKAttr  m_petPoisonAttr;
    //宠物buff缓存
    std::unordered_map<uint32_t, BuffData> m_petBuffMap;
	
private:
    Job m_job;
};

class Role
{
    friend class RoleManager;
    friend class ContrRoleContainer;

public:
    TYPEDEF_PTR(Role)
    CREATE_FUN_NEW(Role)

    ~Role() = default;

public:
    RoleId id() const;
    //roleRarelyUp
    const std::string& name() const;
    const std::string& account() const;
    TurnLife turnLife() const;
    Job job() const;
    Sex sex() const;
    uint64_t curObjId() const;
    uint16_t unlockCellNumOfRole() const;
    uint16_t unlockCellNumOfHero() const;
    uint16_t unlockCellNumOfStorage() const;
    TimePoint recallHeroTime() const;
    uint8_t guanzhiLevel() const;
	const std::string& bufferVec() const;
	//roleOftenUp
    uint32_t level() const;
    uint64_t exp() const;
    uint32_t offlnTime() const;
    uint16_t evilVal() const;
    uint8_t attackMode() const;
    //roleOfflnUp
    uint32_t mp() const;
    uint32_t hp() const;
    SceneId sceneId() const;
    uint8_t dir() const;
    uint16_t posX() const;
    uint16_t posY() const;
    SceneId preSceneId() const;
    uint16_t prePosX() const;
    uint16_t prePosY() const;
    bool dead() const;
    TimePoint deathTime() const;
    uint32_t totalOnlineSec() const;
    TimePoint greynameTime() const;
    uint32_t curMailIndex();
    bool summonHero() const;
	ProcessIdentity worldId() const;

    std::string stallLog() const;

    bool isOnline() const;
    void online();
    void offline();

    void setFaction(FactionId factionId, std::string& factionName, FactionPosition position, uint32_t level);

    bool save(uint32_t m_lastRoleIdCounter);
    void timerExec();

    std::unordered_map<uint64_t,RoleObjData::ObjData> getObjDataByKeyMap();

public:
    bool sendToWorld(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const;

private:
    Role(RoleId id);

    void updateGateway(ProcessIdentity newGateway);

private:
    bool m_isOnline = false;
    const RoleId m_id;
    ProcessIdentity m_gatewayId;
    ProcessIdentity m_worldId;
    //roleRarelyUp
    std::string m_name;
    TurnLife m_turnLife = TurnLife::zero;
    std::string m_account = "";
    Sex m_sex;
    Job m_job;
    uint64_t m_curObjId=0;	//当前已使用的objId (取值于object表中角色的最大objId)
    uint16_t m_unlockCellNumOfRole = DEFAULT_UNLOCK_CELL_NUM_OF_ROLE;
    uint16_t m_unlockCellNumOfHero = DEFAULT_UNLOCK_CELL_NUM_OF_HERO;
    uint16_t m_unlockCellNumOfStorage = DEFAULT_UNLOCK_CELL_NUM_OF_STORAGE;

    //英雄
    Job m_defaultCallHero = Job::none;  //设置的默认召唤英雄
    TimePoint m_recallHeroTime = EPOCH;

    uint8_t m_guanzhiLevel = 0;
    std::string m_bufferVec = "";	//缓存数据(供客户端使用)
	//roleOftenUp
    uint32_t m_level = 1;
    uint64_t m_exp = 0;
    uint64_t m_money_1 = 0;
    uint64_t m_money_2 = 0;
    uint64_t m_money_3 = 0;
    uint64_t m_money_4 = 0;
    uint64_t m_money_5 = 0;
    uint64_t m_money_6 = 0;
    uint64_t m_money_7 = 0;
	uint64_t m_money_8 = 0;
	uint64_t m_money_9 = 0;
	uint64_t m_money_10 = 0;
	//roleOfflnUp
    uint32_t m_mp = 0;
    uint32_t m_hp = 0;
    SceneId m_sceneId = 0;   //地图坐标（默认新手村)
    uint8_t m_dir = 1;
    uint16_t m_posX = 0;
    uint16_t m_posY = 0;
    SceneId m_preSceneId = 0; //上一个静态地图id
    uint16_t m_prePosX = 0;
    uint16_t m_prePosY = 0;
    bool m_dead = false;
    TimePoint m_deathTime = EPOCH; 
    uint32_t m_totalOnlineSec = 0;
    uint32_t m_offlnTime = 0;
    uint16_t m_evilVal = 0;
    uint8_t m_attackMode = 1;
    uint16_t m_anger = 0; //怒气

    TimePoint m_greynameTime = EPOCH;
    uint32_t m_curMailIndex = 0;
	bool m_summonHero = false;	//是否召唤英雄 (缓存，不真正存储数据库) 

    //object数据,key是objId,每个角色自己的唯一标识object的id。
    std::unordered_map<uint64_t,RoleObjData::ObjData> m_objDataByKeyMap;
    //skill数据，key是skillId
    std::unordered_map<uint32_t,SkillData> m_skillDataBykeyMap;
    std::vector<PKCdStatus> pkCdStatusVec;//pk中技能cd的临时状态，只缓存

    //buff
    std::unordered_map<uint32_t,BuffData> m_buffMap;
    //counter
    std::string m_counters;
    //sundry(数据杂项)
    std::string m_sundry;
    //timerSundry(频繁更新的数据杂项)
    std::string m_timerSundry;
    //hero(要跟随roledata返回)
    std::unordered_map<uint8_t, HeroInfoPra> m_heroMap;
	//hero（要随着herodata返回）
    std::unordered_map<uint8_t, Hero::Ptr> m_heros;//heroData
    //
    std::unordered_map<uint32_t, MailInfo> m_mailQueue;

    //坐骑数据
    std::string m_horseStr;
    //人物骑乘状态
    uint8_t m_rideState = 0;

	//title称号
	std::unordered_map<uint32_t, TitleInfo> m_titleMap;		//<titleId, TitleInfo>

	//洗练
	std::unordered_map<uint8_t, std::vector<WashPropInfo> > m_WashPropMap;

	//龙珠
	std::unordered_map<uint8_t, DragonBallInfo> m_dragonMap;	//<dragonType, exp>

	//经验区
	std::unordered_map<uint8_t, uint32_t> m_autoExpMap;		//<type, sec>

    //中施毒术攻击者的缓存
    PKAttr      m_poisonAttackerAttr;

    //宠物
    uint32_t    m_petTplId = 0;
    //宠物中施毒术时攻击者属性缓存
    PKAttr      m_petPoisonAttr;
    //宠物buff缓存
    std::unordered_map<uint32_t, BuffData> m_petBuffMap;

    //任务
    std::string m_taskStr;
    std::string m_factionTaskInfo;
    //帮派
    FactionId m_factionId = 0;
    std::string m_factionName;
    FactionPosition m_position;
    uint32_t m_factionLevel;


    //摆摊日志
    std::string m_stallLog;
};


}


#endif
