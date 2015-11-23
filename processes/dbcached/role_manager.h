/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-31 14:43 +0800
 *
 * Modified: 2015-03-31 14:43 +0800
 *
 * Description: 
 */

#include "role.h"
#include "dbcached.h"
#include "object_manager.h"
#include "skill_manager.h"
#include "counter_manager.h"
#include "mail.h"
#include "sundry_manager.h"

#include "water/process/process_id.h"
#include "water/componet/datetime.h"

#include "common/role_container.h"
#include "componet/string_kit.h"
#include "componet/exception.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"

#include "protocol/rawmsg/private/hero.h"

#include <string>
#include <list>
#include <vector>
#include <unordered_map>

#ifndef DBCACHE_DBROLECACHE_H
#define DBCACHE_DBROLECACHE_H

namespace dbcached{


//控制缓存数量的角色缓存容器
class ContrRoleContainer
{
    enum {MAX_CACHE_NUM = 10000, ERASE_NUM = 20};
public:
    ContrRoleContainer() = default;
    ~ContrRoleContainer() = default;

    std::vector<Role::Ptr> getByAccount(const std::string& account);
    bool insert(Role::Ptr role);
    Role::Ptr getById(RoleId id);
    Role::Ptr getByName(const std::string& name);

//for role
    //roleRarelyUp
    bool updateGuanzhiLevel(RoleId rid, uint32_t level);
	bool updateBufferData(RoleId roleId, const std::string& bufferVec);
    bool updatePackageUnlockCellNum(RoleId rid, uint16_t unlockCellNumOfRole, uint16_t unlockCellNumOfHero, uint16_t unlockCellNumOfStorage);
    bool updateDefaultCallHero(RoleId roleId, Job job);
    bool updateTurnLifeLevel(RoleId roleId, TurnLife turnLifeLevel);
	//roleOftenUp
    bool updateLevelExp(RoleId rid,uint32_t level,uint64_t exp);
    bool updateMoney(const PrivateRaw::UpdateRoleMoney* updateMoney);
    //roleOfflnUp
    bool updateOffln(const PrivateRaw::SaveOffline* rev);

//for object
    bool insertObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj,RoleId roleId);
    bool updateOrInsertObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj,RoleId roleId);
    bool eraseObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj,RoleId roleId);
//for skill
    bool updateOrInsertSkill(const SkillData& modifySkill,RoleId roleId);
    bool eraseSkill(const SkillData& modifySkill,RoleId roleId);
    bool savePKCdStatus(const PrivateRaw::CachePKCdStatus* cachePKiCdStatus);
    bool saveHeroPKCdStatus(const PrivateRaw::CachePKCdStatus* cachePKiCdStatus);
//for buff
    bool updateOrInsertBuff(const BuffData& data, RoleId roleId);
    bool eraseBuff(const BuffData& data, RoleId roleId);
//for counter
    bool updateCounter(const std::string& counterInfo, RoleId roleId);
//for sundry
    bool updateSundry(RoleId roleId, const std::string& data); 
//for timerSundry
    bool updateTimerSundry(RoleId roleId, const std::string& data);
//for hero
    bool insertHero(const HeroInfoPra& heroInfo, RoleId roleId);
    bool updateHeroLevelExp(const PrivateRaw::UpdateHeroLevelExp* rev);
    bool updateHeroTurnLifeLevel(const PrivateRaw::UpdateHeroTurnLifeLevel* rev);
    bool updateHeroClother(const PrivateRaw::UpdateHeroClothes* rev);
    bool saveHeroOffline(const PrivateRaw::SaveHeroOffline* rev);
//其它表
	//title 称号
	bool updateOrInsertTitle(RoleId roleId, const TitleInfo& data);

	//洗练role
	bool updateOrInsertWashProp(RoleId roleId, uint8_t washType, const std::vector<WashPropInfo>& washPropVec);

	//龙珠
	bool updateOrInsertDragonBallExp(RoleId roleId, uint8_t dragonType, uint32_t exp);

	//经验区
	bool updateOrInsertExpAreaSec(RoleId roleId, uint8_t type, uint32_t sec);

//herodata
    //skill
    bool updateOrInsertHeroSkill(const SkillData& modifySkill, RoleId roleId, Job job);
    bool eraseHeroSkill(const SkillData& modifySkill, RoleId roleId, Job job);
    //buff
    bool updateOrInsertHeroBuff(const BuffData& modifySkill, RoleId roleId, Job job);
    bool eraseHeroBuff(const BuffData& modifySkill, RoleId roleId, Job job);
	//洗练hero
	bool updateOrInsertHeroWashProp(RoleId roleId, uint8_t washType, const std::vector<WashPropInfo>& washPropVec);
    
	//mail
    bool insertMail(MailInfo& info, RoleId roleId);
    bool updateMail(const PrivateRaw::UpdateMail::MailModify& modify, RoleId roleId);
    void eraseMail(uint32_t mailIndex, RoleId roleId);

    //horse
    void updateHorseData(RoleId roleId, const std::string& horseStr);
    //中施毒术攻击者属性缓存
    void cachePoisonAttr(const PrivateRaw::CachePoisonAttr* rev);
    //宠物buff缓存
    void updatePetBuff(const BuffData& modifyBuff, RoleId roleId, uint8_t ownerSceneItemType, Job job);
    void erasePetBuff(const BuffData& modifyBuff, RoleId roleId, uint8_t ownerSceneItemType, Job job);

    //任务
    void updateAllTaskInfo(RoleId roleId, const std::string& taskStr);
    void updateFactionTaskState(RoleId roleId, std::string& data);

    //摆摊出售记录
    void updateStallLog(RoleId roleId, const std::string& log);

private:
    void erase(RoleId id);
    void iftouchMAX();
    void mvOnlineuserUp(RoleId id);

    Job getJobByPacType(PackageType type);
private:
    RoleContainer<Role::Ptr> m_roles; //已缓存的user
    //控制缓存数量
    std::list<RoleId> m_roleIdList;
    std::unordered_map<RoleId,std::list<RoleId>::iterator> m_byRoleIdList;
};

//角色管理类
class RoleManager
{
public:
    ~RoleManager() = default;
    void regMsgHandler(); 
    static RoleManager& me();
    void init();
    void test();
private:
    RoleManager() = default;
//角色业务请求,for regMsgHandler
    //allrole
    void servermsg_QuestRoleList(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_CreateRole(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId, const componet::TimePoint& now); 
    void servermsg_UpdateRoleWorldId(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_GetRoleData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    //roleRarelyUp
    void servermsg_UpdateCurObjId(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_UpdateGuanzhiLevel(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_UpdateRoleBufferData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
	void servermsg_UpdateRoleTurnLifeLevel(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
	//roleOfflnUp
    void servermsg_SaveOffline(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    //roleOftenUp
    void servermsg_UpdateRoleLevelExp(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_UpdateRoleMoney(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_UpdatePackageUnlockCellNum(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_UpdateDefaultCallHero(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    //heroData
    void servermsg_ReqHeroSerializeData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_CachePoisonAttr(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);

private:

//业务操作
    //allrole
    Role::Ptr getById(RoleId id);
    bool insert(Role::Ptr role);
    //roleOftenUp
    void updateLevelExp(RoleId rid,uint32_t level,uint64_t exp);
    void updateMoney(const PrivateRaw::UpdateRoleMoney* updateMoney);
    //roleRarelyUp
    std::vector<Role::Ptr> getByAccount(const std::string& account);
    void updateGuanzhiLevel(RoleId rid, uint32_t level);
	void updateBufferData(RoleId roleId, const std::string& bufferVec);
	void updatePackageUnlockCellNum(RoleId rid, uint16_t unlockCellNumOfRole, uint16_t unlockCellNumOfHero, uint16_t unlockCellNumOfStorage);
    void updateDefaultCallHero(RoleId roleId, Job job);
    void updateTurnLifeLevel(RoleId roleId, TurnLife turnLifeLevel);
	//roleOfflnUp
    void updateOffln(const PrivateRaw::SaveOffline* rev);
    //herodata
    Hero::Ptr getHeroByRoleIdAndJob(RoleId roleId, Job job);
    
//直接对角色表的操作,(包括role表和部分其它表的查询)
    //allrole
    std::vector<Role::Ptr> loadByAccount(const std::string& account);
    Role::Ptr loadById(RoleId id);
    //roleOftenUp
    bool updateRoleLevelExp(RoleId rid,uint32_t level,uint64_t exp);
    bool updateRoleMoney(const PrivateRaw::UpdateRoleMoney* updateMoney);
    //roleRarelyUp
    uint32_t loadLastRoleIdCounter();
public:
    bool isExisitName(const std::string& name);
private:
    bool updateRoleGuanzhiLevel(RoleId rid, uint32_t level);
    bool updateRoleBufferData(RoleId roleId, const std::string& bufferVec);
	bool updateRolePackageUnlockCellNum(RoleId rid, uint16_t unlockCellNumOfRole, uint16_t unlockCellNumOfHero, uint16_t unlockCellNumOfStorage);
    bool updateRoleRecallTimePoint(RoleId roleId, water::componet::TimePoint time);
    bool updateRoleDefaultCallHero(RoleId roleId, Job job);
    bool updateRoleTurnLifeLevel(RoleId roleId, TurnLife turnLifeLevel);
	//roleOfflnUp
    bool updateRoleOffln(const PrivateRaw::SaveOffline* rev);
    //roleData
    Hero::Ptr loadHeroData(RoleId roleId, Job job);
private:
    RoleId createRid();//创建角色id
    void initLastRoleCounter();//初始化m_lastRoleIdCounter，得到数据库中最后生成的id
private:
    //角色当前最大id
    uint32_t m_lastRoleIdCounter; 
public:
    ContrRoleContainer m_contrRoles;
};

}//end namespace world

#endif
