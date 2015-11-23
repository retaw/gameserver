#include "role_manager.h"
#include "hero_manager.h"
#include "buff_manager.h"
#include "counter_manager.h"
#include "hero_manager.h"
#include "horse.h"
#include "title_manager.h"
#include "role_table_structure.h"
#include "wash_manager.h"
#include "dragon_ball_manager.h"
#include "task_manager.h"
#include "exp_area_manager.h"
#include "stall_sell_log.h"
#include "faction_role_manager.h"

#include "water/componet/serialize.h"
#include "water/componet/logger.h"

#include "protocol/rawmsg/private/login.h"
#include "protocol/rawmsg/private/login.codedef.private.h"

#include "protocol/rawmsg/private/package.h"
#include "protocol/rawmsg/private/package.codedef.private.h"

#include "protocol/rawmsg/private/hero.h"
#include "protocol/rawmsg/private/hero.codedef.private.h"

#include "protocol/rawmsg/private/guanzhi.h"
#include "protocol/rawmsg/private/guanzhi.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

using namespace water;
using namespace componet;
namespace dbcached{

/*********************************************************/
std::vector<Role::Ptr> ContrRoleContainer::getByAccount(const std::string& account)
{
    auto ret = m_roles.getByAccount(account);
    return ret;
}

Role::Ptr ContrRoleContainer::getById(RoleId id)
{
    //return m_roles.getById(id);
    Role::Ptr ret = m_roles.getById(id);    
    if(ret == nullptr)
        return ret;
    mvOnlineuserUp(id);
    return ret;
}

Role::Ptr ContrRoleContainer::getByName(const std::string& name)
{
    return m_roles.getByName(name);
}

bool ContrRoleContainer::insert(Role::Ptr role)
{
    //达到上限之后进行处理
    iftouchMAX();
    if(m_roles.insert(role))
    {
        m_roleIdList.push_front(role->m_id);
        //这里map插入失败则认为已经存在key,逻辑上m_role中不存在，则此处也不存在，假如真的存在，那就把缓存的关于此角色的都删除
        auto ret = m_byRoleIdList.insert({role->m_id,m_roleIdList.begin()});
        if(ret.second == false)
        {
            m_roles.eraseById(role->m_id);
            m_roleIdList.pop_front();
            LOG_ERROR("ContrRoleContainer::insert, m_byRoleIdList中出现了m_roles中不存在的Id");
            return false;
        }
        LOG_DEBUG("ContrRoleContainer::insert, 缓存插入的角色ID: {} {}",m_roleIdList.front(),*m_byRoleIdList[m_roleIdList.front()]);
        LOG_DEBUG("ContrRoleContainer::insert, 缓存数量: m_roleIdList:{} , m_byRoleIdList:{}", m_roleIdList.size(), m_byRoleIdList.size());
        return true;
    }
    else
    {
        LOG_ERROR("ContrRoleContainer::insert, 插入RoleContainer失败");
        return false;
    }
}
//for object
bool ContrRoleContainer::updateOrInsertObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj,RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
    {
        return false;
    }
    if(uint8_t(modifyObj.packageType) < uint8_t(PackageType::equipOfWarrior))//更新角色
    {
        role->m_objDataByKeyMap[modifyObj.objId].objId = modifyObj.objId;
        role->m_objDataByKeyMap[modifyObj.objId].packageType = modifyObj.packageType;
        role->m_objDataByKeyMap[modifyObj.objId].cell = modifyObj.cell;
        role->m_objDataByKeyMap[modifyObj.objId].tplId = modifyObj.tplId;
        role->m_objDataByKeyMap[modifyObj.objId].item = modifyObj.item;
        role->m_objDataByKeyMap[modifyObj.objId].skillId = modifyObj.skillId;
        role->m_objDataByKeyMap[modifyObj.objId].bind = modifyObj.bind;
        role->m_objDataByKeyMap[modifyObj.objId].sellTime = modifyObj.sellTime;
        role->m_objDataByKeyMap[modifyObj.objId].strongLevel = modifyObj.strongLevel;
        role->m_objDataByKeyMap[modifyObj.objId].luckyLevel = modifyObj.luckyLevel;
	}
    else//更新英雄
    {
        Job job = getJobByPacType(modifyObj.packageType);
		if(job == Job::none)
			return false;

        auto& it = role->m_heros[(uint8_t)job]->m_objDataByKeyMap[modifyObj.objId];
        it.objId = modifyObj.objId;
        it.packageType = modifyObj.packageType;
        it.cell = modifyObj.cell;
        it.tplId = modifyObj.tplId;
        it.item = modifyObj.item;
        it.skillId = modifyObj.skillId;
        it.bind = modifyObj.bind;
        it.sellTime = modifyObj.sellTime;
		it.strongLevel = modifyObj.strongLevel;
		it.luckyLevel = modifyObj.luckyLevel;
    }
    LOG_DEBUG("ContrRoleContainer::updateObject(),缓存修改object成功，roleId={},objId={},",roleId,modifyObj.objId);
    return true;
}

bool ContrRoleContainer::eraseObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj,RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
    {
        return false;
    }
    if(uint8_t(modifyObj.packageType) < uint8_t(PackageType::equipOfWarrior))//更新角色
    {
        if(role->m_objDataByKeyMap.erase(modifyObj.objId) == 0)
        {
            LOG_ERROR("ContrRoleContainer::eraseObject,要删除缓存的object不存在, roleId={}, objId={}, packageType={}",
                      roleId, modifyObj.objId, modifyObj.packageType);
        }
    }
    else
    {
        auto job = getJobByPacType(modifyObj.packageType);
        if(role->m_heros[(uint8_t)job]->m_objDataByKeyMap.erase(modifyObj.objId) == 0)
        {
            LOG_ERROR("ContrRoleContainer::eraseObject,要删除缓存的object不存在, roleId={}, objId={}, packageType={}",
                      roleId, modifyObj.objId, modifyObj.packageType);
        }
    }
    return true;
}

//for skill

bool ContrRoleContainer::updateOrInsertSkill(const SkillData& modifySkill,RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
    {
        return false;
    }
    role->m_skillDataBykeyMap[modifySkill.skillId] = modifySkill;
    return true;
}

bool ContrRoleContainer::eraseSkill(const SkillData& modifySkill,RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
    {
        return false;
    }
    role->m_skillDataBykeyMap.erase(modifySkill.skillId);
    return true;
}


bool ContrRoleContainer::savePKCdStatus(const PrivateRaw::CachePKCdStatus* cachePKCdStatus)
{
   auto role = m_roles.getById(cachePKCdStatus->roleId); 
   if(!role)
   {
        LOG_ERROR("ContrRoleContainer::savePKCdStatus, 要更新的缓存不存在, roleId={}",
                  cachePKCdStatus->roleId);
        return false;
   }
   role->pkCdStatusVec.clear();
   role->pkCdStatusVec.reserve(cachePKCdStatus->size);
   for(int i = 0; i < cachePKCdStatus->size; i++)
   {
       role->pkCdStatusVec.push_back(cachePKCdStatus->data[i]);
   }
   return true;
}

bool ContrRoleContainer::saveHeroPKCdStatus(const PrivateRaw::CachePKCdStatus* cachePKCdStatus)
{
   auto role = m_roles.getById(cachePKCdStatus->roleId); 
   if(!role)
   {
        LOG_ERROR("ContrRoleContainer::saveHeroPKCdStatus, 要更新缓存的角色不存在, roleId={}",
                  cachePKCdStatus->roleId);
        return false;
   }
   auto it = role->m_heros.find(uint8_t(cachePKCdStatus->job));
   if(it == role->m_heros.end())
   {
        LOG_ERROR("ContrRoleContainer::saveHeroPKCdStatus, 要更新缓存的英雄不存在, roleId={}, job={}",
                  cachePKCdStatus->roleId, cachePKCdStatus->job);
        return false;
   }
   auto hero = it->second;
   hero->pkCdStatusVec.clear();
   hero->pkCdStatusVec.reserve(cachePKCdStatus->size);
   for(int i = 0; i < cachePKCdStatus->size; i++)
   {
       role->pkCdStatusVec.push_back(cachePKCdStatus->data[i]);
   }

   return true;
}

bool ContrRoleContainer::updateOffln(const PrivateRaw::SaveOffline* rev)
{
    auto role = m_roles.getById(rev->rid);
    if(!role)
    {
        LOG_ERROR("ContrRoleContainer::updatePos(),要更新的缓存roleId={}不存在",rev->rid);
        return false;
    }
    role->m_sceneId = rev->sceneId;
    role->m_dir = rev->dir;
    role->m_posX = rev->posX;
    role->m_posY = rev->posY;
    role->m_preSceneId = rev->preSceneId;
    role->m_prePosX = rev->prePosX;
    role->m_prePosY = rev->prePosY;
    role->m_mp = rev->mp;
    role->m_hp = rev->hp;
	role->m_dead = rev->dead;
	role->m_deathTime = rev->deathTime;
	role->m_totalOnlineSec = rev->totalOnlineSec;
    role->m_evilVal = rev->evilVal;
    role->m_offlnTime = rev->offlnTime;
    role->m_attackMode = rev->attackMode;
    role->m_greynameTime = rev->greynameTime;
	role->m_summonHero = rev->summonHero;
    role->m_rideState = rev->rideState;
	role->m_curObjId = rev->curObjId;
    role->m_petTplId = rev->petTplId;
    role->m_anger = rev->anger;
	return true;
}

bool ContrRoleContainer::updateLevelExp(RoleId rid,uint32_t level,uint64_t exp)
{
    auto role = m_roles.getById(rid);
    if(!role)
    {
        LOG_ERROR("ContrRoleContainer::updateLevelExp, 要更新的缓存roleId={}不存在",rid);
        return false;
    }
    role->m_level = level;
    role->m_exp = exp;
    return true;
}
    
bool ContrRoleContainer::updateGuanzhiLevel(RoleId rid, uint32_t level)
{
    auto role = m_roles.getById(rid);
    if(!role)
    {
        LOG_DEBUG("ContrRoleContainer::updateGuanzhiLevel, 要更新的缓存roleId={}不存在",rid);
        return false;
    }
    role->m_guanzhiLevel = level;
    return true;
}

bool ContrRoleContainer::updateBufferData(RoleId roleId, const std::string& bufferVec)
{
	auto role = m_roles.getById(roleId);
	if(role == nullptr)
		return false;

	role->m_bufferVec = bufferVec;
	return true;
}

bool ContrRoleContainer::updatePackageUnlockCellNum(RoleId rid, uint16_t unlockCellNumOfRole, uint16_t unlockCellNumOfHero, uint16_t unlockCellNumOfStorage)
{
    auto role = m_roles.getById(rid);
    if(!role)
    {
        LOG_ERROR("ContrRoleContainer::updatePackageUnlockCellNum, 要更新的缓存roleId={}不存在", rid);
        return false;
    }
    role->m_unlockCellNumOfRole = unlockCellNumOfRole;
    role->m_unlockCellNumOfHero = unlockCellNumOfHero;
    role->m_unlockCellNumOfStorage = unlockCellNumOfStorage;
    return true;
}


bool ContrRoleContainer::updateMoney(const PrivateRaw::UpdateRoleMoney* updateMoney)
{
    auto role = m_roles.getById(updateMoney->roleId);
    if(!role)
    {
        LOG_ERROR("ContrRoleContainer::updateMoney, 要更新的缓存roleId={}不存在",updateMoney->roleId);
        return false;
    }
    switch(updateMoney->type)
    {
    case MoneyType::money_1:
         role->m_money_1 = updateMoney->money;
         break;
    case MoneyType::money_2:
         role->m_money_2 = updateMoney->money;
         break;
    case MoneyType::money_3:
         role->m_money_3 = updateMoney->money;
         break;
    case MoneyType::money_4:
         role->m_money_4 = updateMoney->money;
         break;
    case MoneyType::money_5:
         role->m_money_5 = updateMoney->money;
         break;
    case MoneyType::money_6:
         role->m_money_6 = updateMoney->money;
         break;
	case MoneyType::money_7:
		 role->m_money_7 = updateMoney->money;
		 break;
	case MoneyType::money_8:
		 role->m_money_8 = updateMoney->money;
		 break;
	case MoneyType::money_9:
		 role->m_money_9 = updateMoney->money;
		 break;
	case MoneyType::money_10:
		 role->m_money_10 = updateMoney->money;
		 break;
    default:
         break;
    }
    return true;
}

bool ContrRoleContainer::updateDefaultCallHero(RoleId roleId, Job job)
{
    auto role = m_roles.getById(roleId);
    if(!role)
    {
        LOG_ERROR("ContrRoleContainer::updateDefaultCallHero, 要更新的roleId = {}不存在", roleId);
          return false;
    }
    role->m_defaultCallHero = job;
    return true;
}

bool ContrRoleContainer::updateTurnLifeLevel(RoleId roleId, TurnLife turnLifeLevel)
{
    auto role = m_roles.getById(roleId);
    if(!role)
    {
        LOG_DEBUG("转生, 要更新的缓存roleId={}不存在, level={}", roleId, turnLifeLevel);
        return false;
    }
    role->m_turnLife = turnLifeLevel;
    return true;
}

//===================buff start=====================
bool ContrRoleContainer::updateOrInsertBuff(const BuffData& data, RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
    role->m_buffMap[data.buffId] = data;
    return true;
}

bool ContrRoleContainer::eraseBuff(const BuffData& data, RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
    role->m_buffMap.erase(data.buffId);
    return true;
}

//===================buff end========================


//
bool ContrRoleContainer::updateCounter(const std::string& counterInfo, RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;

    role->m_counters.clear();
    role->m_counters = counterInfo;
    return true;
}

bool ContrRoleContainer::updateSundry(RoleId roleId, const std::string& data)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
     
    role->m_sundry = data;
    return true;
}

bool ContrRoleContainer::updateTimerSundry(RoleId roleId, const std::string& data)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;

    role->m_timerSundry = data;
    return true;
}

//title 称号
bool ContrRoleContainer::updateOrInsertTitle(RoleId roleId, const TitleInfo& data)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
    
	role->m_titleMap[data.titleId] = data;
    return true;
}

//洗练role
bool ContrRoleContainer::updateOrInsertWashProp(RoleId roleId, uint8_t washType, const std::vector<WashPropInfo>& washPropVec)
{
	auto role = m_roles.getById(roleId);
	if(role == nullptr)
		return false;

	role->m_WashPropMap[washType] = washPropVec;
	return true;
}

//龙珠
bool ContrRoleContainer::updateOrInsertDragonBallExp(RoleId roleId, uint8_t dragonType, uint32_t exp)
{
	auto role = m_roles.getById(roleId);
	if(role == nullptr)
		return false;

	DragonBallInfo temp;
	temp.type = dragonType;
	temp.exp = exp;

	role->m_dragonMap[dragonType] = temp;
	return true;
}

//经验区
bool ContrRoleContainer::updateOrInsertExpAreaSec(RoleId roleId, uint8_t type, uint32_t sec)
{
	auto role = m_roles.getById(roleId);
	if(role == nullptr)
		return false;

	role->m_autoExpMap[type] = sec;
	return true;
}

bool ContrRoleContainer::insertHero(const HeroInfoPra& heroInfo, RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
    role->m_heroMap[uint8_t(heroInfo.job)] = heroInfo;
    return true;
}

bool ContrRoleContainer::updateHeroLevelExp(const PrivateRaw::UpdateHeroLevelExp* rev)
{
    auto role = m_roles.getById(rev->roleId);
    if(!role)
        return false;
    role->m_heroMap[uint8_t(rev->job)].exp = rev->exp;
    role->m_heroMap[uint8_t(rev->job)].level = rev->level;
    return true;
}

bool ContrRoleContainer::updateHeroTurnLifeLevel(const PrivateRaw::UpdateHeroTurnLifeLevel* rev)
{
    auto role = m_roles.getById(rev->roleId);
    if(!role)
        return false;
    role->m_heroMap[uint8_t(rev->job)].turnLife = rev->turnLifeLevel;
    return true;
}

bool ContrRoleContainer::updateHeroClother(const PrivateRaw::UpdateHeroClothes* rev)
{
    auto role = m_roles.getById(rev->roleId);
    if(!role)
        return false;
    role->m_heroMap[uint8_t(rev->job)].clother = rev->clother;
    return true;
}

bool ContrRoleContainer::saveHeroOffline(const PrivateRaw::SaveHeroOffline* rev)
{
    auto role = m_roles.getById(rev->roleId);
    if(!role)
        return false;
    role->m_heroMap[uint8_t(rev->job)].hp = rev->hp;
    role->m_heroMap[uint8_t(rev->job)].mp = rev->mp;
    role->m_heroMap[uint8_t(rev->job)].clother = rev->clother;
    role->m_recallHeroTime = rev->recallTimePoint;

    auto it = role->m_heros.find(uint8_t(rev->job));
    if(it == role->m_heros.end())
        return false;
    auto hero = it->second;
    hero->m_petTplId = rev->petTplId;

    return true;
}

bool ContrRoleContainer::eraseHeroSkill(const SkillData& modifySkill, RoleId roleId, Job job)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
    auto it = role->m_heros.find(uint8_t(job));
    if(it == role->m_heros.end())
        return false;
    auto hero = it->second;
    hero->m_skillDataBykeyMap.erase(modifySkill.skillId);
    return true;
}

bool ContrRoleContainer::updateOrInsertHeroSkill(const SkillData& modifySkill, RoleId roleId, Job job)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
    auto it = role->m_heros.find(uint8_t(job));
    if(it == role->m_heros.end())
        return false;
    auto hero = it->second;
    hero->m_skillDataBykeyMap[modifySkill.skillId] = modifySkill; 
    return true;
}

bool ContrRoleContainer::eraseHeroBuff(const BuffData& modifyBuff, RoleId roleId, Job job)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
    auto it = role->m_heros.find(uint8_t(job));
    if(it == role->m_heros.end())
        return false;
    auto hero = it->second;
    hero->m_buffMap.erase(modifyBuff.buffId);
    return true;
}

bool ContrRoleContainer::updateOrInsertHeroBuff(const BuffData& modifyBuff, RoleId roleId, Job job)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;
    auto it = role->m_heros.find(uint8_t(job));
    if(it == role->m_heros.end())
        return false;
    auto hero = it->second;
    hero->m_buffMap[modifyBuff.buffId] = modifyBuff; 
    return true;
}

//洗练hero
bool ContrRoleContainer::updateOrInsertHeroWashProp(RoleId roleId, uint8_t washType, const std::vector<WashPropInfo>& washPropVec)
{
	auto role = m_roles.getById(roleId);
	if(role == nullptr)
		return false;

	bool flag = false;
	for(auto iter = role->m_heros.begin(); iter != role->m_heros.end(); ++iter)
	{
		if(iter->second == nullptr)
			continue;

		Hero::Ptr hero = iter->second;
		hero->m_WashPropMap[washType] = washPropVec;
		flag = true;
	}

	return flag;
}


bool ContrRoleContainer::insertMail(MailInfo& info, RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;

    return role->m_mailQueue.insert({info.mailIndex, info}).second;
}

bool ContrRoleContainer::updateMail(const PrivateRaw::UpdateMail::MailModify& modify, RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return false;

    if(role->m_mailQueue.find(modify.mailIndex) == role->m_mailQueue.end())
        return false;

    role->m_mailQueue[modify.mailIndex].state = modify.state;
    return true;
}

void ContrRoleContainer::eraseMail(uint32_t mailIndex, RoleId roleId)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return;

    role->m_mailQueue.erase(mailIndex);
}

void ContrRoleContainer::updateHorseData(RoleId roleId, const std::string& horseStr)
{
    auto role = m_roles.getById(roleId);
    if(!role)
        return;

    role->m_horseStr = horseStr;
}


void ContrRoleContainer::cachePoisonAttr(const PrivateRaw::CachePoisonAttr* rev)
{
    auto role = m_roles.getById(rev->roleId);
    if(nullptr == role)
        return;

    if(rev->sceneItem == 1)//主角
        role->m_poisonAttackerAttr = rev->data;
    else if(rev->sceneItem == 2) //hero
    {
        if(role->m_heros.find(static_cast<uint8_t>(rev->job)) != role->m_heros.end())
        {
            Hero::Ptr hero = role->m_heros[static_cast<uint8_t>(rev->job)];
            hero->m_poisonAttackerAttr = rev->data;
        }
    }
    else //pet
    {
        if(rev->roleId == rev->id)
            role->m_petPoisonAttr = rev->data;
        else if(role->m_heros.find(static_cast<uint8_t>(rev->ownerJob)) != role->m_heros.end())
        {
            Hero::Ptr hero = role->m_heros[static_cast<uint8_t>(rev->ownerJob)];
            hero->m_petPoisonAttr = rev->data;
        }
    }
}


void ContrRoleContainer::updatePetBuff(const BuffData& modifyBuff, RoleId roleId, uint8_t ownerSceneItemType, Job ownerJob)
{
    auto role = m_roles.getById(roleId);
    if(nullptr == role)
        return;

    if(ownerSceneItemType == 1)
        role->m_petBuffMap[modifyBuff.buffId] = modifyBuff;
    else
    {
        auto it = role->m_heros.find((uint8_t)ownerJob);
        if(it == role->m_heros.end())
            return;
        
        auto hero = it->second;
        hero->m_petBuffMap[modifyBuff.buffId] = modifyBuff;
    }
}


void ContrRoleContainer::erasePetBuff(const BuffData& modifyBuff, RoleId roleId, uint8_t ownerSceneItemType, Job ownerJob)
{
    auto role = m_roles.getById(roleId);
    if(nullptr == role)
        return;

    if(ownerSceneItemType == 1)
        role->m_petBuffMap.erase(modifyBuff.buffId);
    else
    {
        auto it = role->m_heros.find((uint8_t)ownerJob);
        if(it == role->m_heros.end())
            return;
        
        auto hero = it->second;
        hero->m_petBuffMap.erase(modifyBuff.buffId);
    }
}

void ContrRoleContainer::updateAllTaskInfo(RoleId roleId, const std::string& taskStr)
{
    auto role = m_roles.getById(roleId);
    if(nullptr == role)
        return;

	role->m_taskStr = taskStr;
}

void ContrRoleContainer::updateFactionTaskState(RoleId roleId, std::string& data)
{
    auto role = m_roles.getById(roleId);
    if(nullptr == role)
        return;

    role->m_factionTaskInfo.swap(data);
}


void ContrRoleContainer::updateStallLog(RoleId roleId, const std::string& log)
{
    auto role = m_roles.getById(roleId);
    if(nullptr == role)
        return;

    role->m_stallLog = log;
}


void ContrRoleContainer::erase(RoleId roleId)
{
    m_roles.eraseById(roleId);
    m_roleIdList.erase(m_byRoleIdList[roleId]);
    m_byRoleIdList.erase(roleId);
}

void ContrRoleContainer::iftouchMAX()
{
    if(m_roleIdList.size() > MAX_CACHE_NUM)
    {
        for(unsigned int i = 0; i < ERASE_NUM; i++)
        {
            m_roles.eraseById(m_roleIdList.back());
            m_byRoleIdList.erase(m_roleIdList.back());
            m_roleIdList.pop_back();
        }
    }
}

void ContrRoleContainer::mvOnlineuserUp(RoleId id)
{
    auto ret = m_byRoleIdList.find(id);
    //这里如果不存在的情况正常不会出现，如果出现，即m_roles中有,m_byRoleIdList中没有,肯定是插入或者删除处理有问题
    if(ret == m_byRoleIdList.end())
    {
        LOG_ERROR("ContrRoleContainer::mvOnlineuserUp, m_roles中存在的roleId在m_byRoleIdLis中不存在");
        return;
    }
    m_roleIdList.push_front(*(ret->second));
    m_roleIdList.erase(ret->second);
    m_byRoleIdList.erase(ret);
    m_byRoleIdList.insert({m_roleIdList.front(),m_roleIdList.begin()});
    LOG_TRACE("ContrRoleContainer::mvOnlineuserUp, 提前的角色, roleId = {}",m_roleIdList.front());
}

Job ContrRoleContainer::getJobByPacType(PackageType type)
{
	switch(type)
	{
	case PackageType::equipOfWarrior:
	case PackageType::stoneOfWarrior:
		return Job::warrior;
	case PackageType::equipOfMagician:
	case PackageType::stoneOfMagician:
		return Job::magician;
	case PackageType::equipOfTaoist:
	case PackageType::stoneOfTaoist:
		return Job::taoist;
	default:
		break;
	}

	return Job::none;
}

/*********************************************************/
//public
RoleManager& RoleManager::me()
{
    static RoleManager me;
    //me.initLastRoleCounter();
    return me;
}

void RoleManager::init()
{
    initLastRoleCounter();
}

void RoleManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(QuestRoleList, std::bind(&RoleManager::servermsg_QuestRoleList, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(GetRoleData, std::bind(&RoleManager::servermsg_GetRoleData, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(CreateRole, std::bind(&RoleManager::servermsg_CreateRole, this, _1, _2, _3, _4));
    REG_RAWMSG_PRIVATE(UpdateRoleWorldId, std::bind(&RoleManager::servermsg_UpdateRoleWorldId, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(UpdateRoleLevelExp, std::bind(&RoleManager::servermsg_UpdateRoleLevelExp, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(UpdateRoleMoney, std::bind(&RoleManager::servermsg_UpdateRoleMoney, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(SaveOffline, std::bind(&RoleManager::servermsg_SaveOffline, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(UpdateGuanzhiLevelToDB, std::bind(&RoleManager::servermsg_UpdateGuanzhiLevel, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(UpdateRoleBufferData, std::bind(&RoleManager::servermsg_UpdateRoleBufferData, this, _1, _2, _3));
	REG_RAWMSG_PRIVATE(UpdatePackageUnlockCellNum, std::bind(&RoleManager::servermsg_UpdatePackageUnlockCellNum, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(UpdateDefaultCallHero, std::bind(&RoleManager::servermsg_UpdateDefaultCallHero, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(ReqHeroSerializeData, std::bind(&RoleManager::servermsg_ReqHeroSerializeData, this, _1, _2,_3));
    REG_RAWMSG_PRIVATE(CachePoisonAttr, std::bind(&RoleManager::servermsg_CachePoisonAttr, this, _1, _2,_3));
	REG_RAWMSG_PRIVATE(UpdateRoleTurnLifeLevel, std::bind(&RoleManager::servermsg_UpdateRoleTurnLifeLevel, this, _1, _2, _3));
}

//private角色业务请求,for regMsgHandler
void RoleManager::servermsg_QuestRoleList(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::QuestRoleList*>(msgData);

    LOG_DEBUG("登录, 获取角色列表, loginId={}", rev->loginId);

    std::vector<Role::Ptr> roleList = getByAccount(rev->account);
    if(roleList.size() > MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE)
    {
        LOG_ERROR("登录, 获取基本信息, 账号{}拥有{}个角色", rev->account, roleList.size());
        roleList.resize(MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE);
    }
    PrivateRaw::RetRoleList send;
    send.loginId  = rev->loginId;
    send.listSize = roleList.size();
    for(uint32_t i = 0; i < roleList.size(); ++i)
    {
        roleList[i]->name().copy(send.roleList[i].name, sizeof(send.roleList[i].name) - 1);
        send.roleList[i].id  = roleList[i]->id();
        send.roleList[i].job = roleList[i]->job();
        send.roleList[i].sex = roleList[i]->sex();
    }

    ProcessIdentity pid(remoteProcessId);
    DbCached::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetRoleList), &send, sizeof(send));
    LOG_DEBUG("登录, 请求角色列表, 成功, account={}", rev->account);
}

void RoleManager::servermsg_CreateRole(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId, const componet::TimePoint& now)
{
    auto rev = reinterpret_cast<const PrivateRaw::CreateRole*>(msgData);

    PrivateRaw::RetCreateRole send;
    send.loginId = rev->loginId;
    if(isExisitName(rev->basicInfo.name))
    {
        send.code = LoginRetCode::conflictedRoleName;
        ProcessIdentity pid(remoteProcessId);
        DbCached::me().sendToPrivate(remoteProcessId, RAWMSG_CODE_PRIVATE(RetCreateRole), &send, sizeof(send));
        LOG_DEBUG("登录, 建角, 角色名冲突, account={}, name={}", rev->account, rev->basicInfo.name);
        return;
    }

    std::vector<Role::Ptr> roleList = getByAccount(rev->account);
    if(roleList.size() >= MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE)
        return;

    auto role = Role::create(createRid());
    role->m_gatewayId = remoteProcessId;
    role->m_account   = rev->account;
    role->m_name  = rev->basicInfo.name;
    role->m_job   = rev->basicInfo.job;
    role->m_sex   = rev->basicInfo.sex; 

    if(!insert(role))
    {
        send.code = LoginRetCode::failed;
        ProcessIdentity pid(remoteProcessId);
        DbCached::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetCreateRole), &send, sizeof(send));
        LOG_ERROR("登录, 建角, 插入RoleManager失败, account={}", rev->account);
        return;
    }
    roleList.push_back(role);

    send.code = LoginRetCode::successful;

    send.listSize = roleList.size();
    for(uint32_t i = 0; i < roleList.size(); ++i)
    {
        roleList[i]->name().copy(send.roleList[i].name, sizeof(send.roleList[i].name) - 1);
        send.roleList[i].id  = roleList[i]->id();
        send.roleList[i].job = roleList[i]->job();
        send.roleList[i].sex = roleList[i]->sex();
    }

    ProcessIdentity pid(remoteProcessId);
    DbCached::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetCreateRole), &send, sizeof(send));
    LOG_DEBUG("登录, 建角, 成功, account={}, role=({},{})", rev->account, role->id(), role->name());
}

void RoleManager::servermsg_UpdateRoleWorldId(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateRoleWorldId*>(msgData);
    auto role = m_contrRoles.getById(rev->rid);
    if(nullptr == role)
        return;

    role->m_worldId = ProcessIdentity(rev->worldId);
}

void RoleManager::servermsg_GetRoleData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::GetRoleData*>(msgData);
    ProcessIdentity pid(remoteProcessId);

    LOG_DEBUG("收到角色数据请求, remotePid={}, rid={}, loginId={}", pid, rev->rid, rev->loginId);
    Role::Ptr role = getById(rev->rid);
    //选择角色非法
    if(role == nullptr)
    {
        LOG_ERROR("请求的角色信息在role中不存在, id={}", rev->rid);
        return;
    }

    //直接视为登录成功, 并通知session分配地图
    //即：db->session->scene->gateway
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PrivateRaw::RetRoleData)); 

    Serialize<std::string> ss;
    ss.reset();
    std::vector<RoleObjData::ObjData> objVec;
    std::vector<SkillData> skillVec;
    std::vector<BuffData> buffVec;
    std::vector<HeroInfoPra> heroInfoVec;
	std::vector<TitleInfo> titleVec;
	std::vector<WashPropInfo> washPropVec;
    std::vector<BuffData> petBuffVec;
	std::vector<DragonBallInfo> dragonVec;
	std::vector<std::pair<uint8_t, uint32_t> > expSecVec;
	for(auto& object : role->m_objDataByKeyMap)
        objVec.push_back(object.second);
    for(auto& skill : role->m_skillDataBykeyMap)
        skillVec.push_back(skill.second);
    for(const auto& iter : role->m_buffMap)
        buffVec.push_back(iter.second);
    for(const auto& hero : role->m_heroMap)
        heroInfoVec.push_back(hero.second);
	for(const auto& iter : role->m_titleMap)
		titleVec.push_back(iter.second);
	for(const auto& pos : role->m_WashPropMap)
	{
		for(const auto& iter : pos.second)
		{
			washPropVec.push_back(iter);
		}
	}
    for(const auto& iter : role->m_petBuffMap)
        petBuffVec.push_back(iter.second);
	for(const auto& iter : role->m_dragonMap)
		dragonVec.push_back(iter.second);
	for(const auto& iter : role->m_autoExpMap)
		expSecVec.push_back(std::make_pair(iter.first, iter.second));

    ss << objVec;
    ss << skillVec;
    ss << role->pkCdStatusVec;
    ss << buffVec;
    ss << role->m_counters;
    ss << heroInfoVec;

    ss << (uint32_t)(role->m_mailQueue.size());
    for(const auto& iter : role->m_mailQueue)
    {
        ss << iter.second.mailIndex;
        ss << iter.second.title;
        ss << iter.second.text;
        ss << iter.second.state;
        ss << iter.second.time;
        for(auto i = 0; i < MAX_MAIL_OBJ_NUM; ++i)
        {
            ss << iter.second.obj[i].tplId;
            ss << iter.second.obj[i].num;
            ss << iter.second.obj[i].bind;
        }
    }

    ss << role->m_horseStr;
	ss << titleVec;
	ss << role->m_bufferVec;
	ss << washPropVec;
    ss << role->m_poisonAttackerAttr;
    ss << role->m_petPoisonAttr;
    ss << petBuffVec;
	ss << dragonVec;

    ss << role->m_taskStr;
    ss << role->m_factionTaskInfo;

	ss << expSecVec;
    ss << role->m_sundry;//sundry本身也是序列化,stirng
    ss << role->m_timerSundry;//身也是序列化,stirng

    buf.resize(buf.size()+ss.tellp());
    auto* msg  = reinterpret_cast<PrivateRaw::RetRoleData*>(buf.data());  
    msg->usage = rev->usage;
    if(rev->usage == PrivateRaw::RoleDataUsage::changeScene)
    {
        msg->sceneId = rev->newSceneId;
        msg->posX = rev->posX;
        msg->posY = rev->posY;
    }
    else// if (rev->usage == PrivateRaw::RoleDataUsage::login)
    {
        //登录的情况下, 没时间了, 先写死 实际应该从db去读 
        msg->sceneId = role->sceneId(); //role->offlineSceneId();
        msg->posX = role->posX();
        msg->posY = role->posY();
    }
	msg->preSceneId = role->preSceneId();
	msg->prePosX = role->prePosX();
	msg->prePosY = role->prePosY();
    msg->loginId = rev->loginId;
    msg->basic.id = role->id();
    role->name().copy(msg->basic.name, sizeof(msg->basic.name) - 1);
    msg->level = role->level();
    msg->exp = role->exp();
    msg->turnLife = role->turnLife();
    msg->dir = role->dir();
    msg->mp = role->mp();
    msg->hp = role->hp();
    msg->basic.job = role->job();
    msg->basic.sex = role->sex();
    role->account().copy(msg->account, sizeof(msg->account) - 1);
    msg->gatewayId = rev->gatewayId;
    msg->curObjId = role->curObjId();
    msg->dead = role->dead();
    msg->deathTime = role->deathTime();
	msg->totalOnlineSec = role->totalOnlineSec();
    msg->greynameTime = role->greynameTime();
	msg->guanzhiLevel = role->guanzhiLevel();
	msg->summonHero = role->summonHero();

    //背包数据,role表
    msg->unlockCellNumOfRole = role->unlockCellNumOfRole();
    msg->unlockCellNumOfHero = role->unlockCellNumOfHero();
    msg->unlockCellNumOfStorage = role->unlockCellNumOfStorage();
    //money
    msg->money_1 = role->m_money_1;
    msg->money_2 = role->m_money_2;
    msg->money_3 = role->m_money_3;
    msg->money_4 = role->m_money_4;
    msg->money_5 = role->m_money_5;
    msg->money_6 = role->m_money_6;
    msg->money_7 = role->m_money_7;
	msg->money_8 = role->m_money_8;
	msg->money_9 = role->m_money_9;
	msg->money_10 = role->m_money_10;
	//
    msg->defaultCallHero = role->m_defaultCallHero;
    msg->recallHeroTime = role->recallHeroTime();
    msg->offlnTime = role->m_offlnTime;
    msg->evilVal = role->m_evilVal;
    msg->attackMode = role->m_attackMode;
    msg->rideState = role->m_rideState;
    msg->petTplId = role->m_petTplId;
    msg->anger = role->m_anger;
    //帮派信息
    msg->factionId = role->m_factionId;
    role->m_factionName.copy(msg->factionName, NAME_BUFF_SZIE);
    msg->factionPosition = role->m_position;
    msg->factionLevel = role->m_factionLevel;

    //变长数据相关数据拷贝，以及字节长度赋给msg
    std::memcpy(msg->buf,ss.buffer()->data(),ss.tellp());
    msg->size = ss.tellp();
    
    DbCached::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetRoleData), buf.data(), buf.size());
    LOG_DEBUG("返回角色数据到{}, account={}, role=[{}, {}, {}]", pid, role->id(), role->name(), role->account());
}

void RoleManager::servermsg_UpdateRoleLevelExp(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到角色等级和经验更新");
    auto rev = reinterpret_cast<const PrivateRaw::UpdateRoleLevelExp*>(msgData);
    updateLevelExp(rev->rid,rev->level,rev->exp);
}

void RoleManager::servermsg_UpdateRoleMoney(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到角色money更新");
    auto rev = reinterpret_cast<const PrivateRaw::UpdateRoleMoney*>(msgData);
    updateMoney(rev);
}

void RoleManager::servermsg_SaveOffline(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessI)
{
    LOG_DEBUG("收到角色下线数据更新");
    auto rev = reinterpret_cast<const PrivateRaw::SaveOffline*>(msgData);
    updateOffln(rev);
}

void RoleManager::servermsg_UpdateGuanzhiLevel(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到官职更新消息");
    auto rev = reinterpret_cast<const PrivateRaw::UpdateGuanzhiLevelToDB*>(msgData);
    updateGuanzhiLevel(rev->roleId, rev->level);
}

void RoleManager::servermsg_UpdateRoleBufferData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
	LOG_DEBUG("收到更新角色的缓存数据消息");
	auto rev = reinterpret_cast<const PrivateRaw::UpdateRoleBufferData*>(msgData);
    std::string buffer("");
    buffer.append((const char*)rev->buf, rev->size);
	updateBufferData(rev->roleId, buffer);
}

void RoleManager::servermsg_UpdateRoleTurnLifeLevel(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到转生等级更新消息");
    auto rev = reinterpret_cast<const PrivateRaw::UpdateRoleTurnLifeLevel*>(msgData);
    updateTurnLifeLevel(rev->roleId, rev->turnLifeLevel);
}

void RoleManager::servermsg_UpdatePackageUnlockCellNum(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到更新背包解锁格子数消息");
    auto rev = reinterpret_cast<const PrivateRaw::UpdatePackageUnlockCellNum*>(msgData);
    updatePackageUnlockCellNum(rev->roleId, rev->unlockCellNumOfRole, rev->unlockCellNumOfHero, rev->unlockCellNumOfStorage);
}

void RoleManager::servermsg_UpdateDefaultCallHero(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到更新defaultCallHero消息");
    auto rev = reinterpret_cast<const PrivateRaw::UpdateDefaultCallHero*>(msgData);
    updateDefaultCallHero(rev->roleId, rev->defaultCallHero);
}

void RoleManager::servermsg_ReqHeroSerializeData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到英雄数据请求");
    auto rev = reinterpret_cast<const PrivateRaw::ReqHeroSerializeData*>(msgData);
    ProcessIdentity pid(remoteProcessId);
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PrivateRaw::RetHeroSerializeData));
    Hero::Ptr hero = getHeroByRoleIdAndJob(rev->roleId, rev->job);
    if(nullptr == hero)
    {
        LOG_ERROR("英雄, world请求英雄数据时 hero为空");
        return;
    }
    Serialize<std::string> ss;
    ss.reset();
    std::vector<RoleObjData::ObjData> objVec;
    std::vector<SkillData> skillVec;
    std::vector<BuffData> buffVec;
	std::vector<WashPropInfo> washPropVec;
    std::vector<BuffData> petBuffVec;

    for(auto& it : hero->m_objDataByKeyMap)
    {
        objVec.push_back(it.second);
    }
    for(auto& it : hero->m_skillDataBykeyMap)
    {
        skillVec.push_back(it.second);
    }
    for(auto& it : hero->m_buffMap)
    {
        buffVec.push_back(it.second);
    }
	for(const auto& pos : hero->m_WashPropMap)
	{
		for(const auto& iter : pos.second)
		{
			washPropVec.push_back(iter);
		}
	}
    for(const auto& it : hero->m_petBuffMap)
        petBuffVec.push_back(it.second);
    ss << objVec;
    ss << skillVec;
    ss << buffVec;
    ss << hero->pkCdStatusVec;
	ss << washPropVec;
    ss << hero->m_poisonAttackerAttr;
    ss << hero->m_petTplId;
    ss << hero->m_petPoisonAttr;
    ss << petBuffVec;

    buf.resize(buf.size()+ss.tellp());
    auto* msg = reinterpret_cast<PrivateRaw::RetHeroSerializeData*>(buf.data());
    msg->roleId = rev->roleId;
    msg->job = rev->job;
    //变长数据相关数据拷贝，以及字节长度赋给msg
    std::memcpy(msg->buf,ss.buffer()->data(),ss.tellp());
    msg->size = ss.tellp();
    DbCached::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetHeroSerializeData), buf.data(), buf.size());

}


void RoleManager::servermsg_CachePoisonAttr(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::CachePoisonAttr*>(msgData);
    m_contrRoles.cachePoisonAttr(rev);
}


void RoleManager::initLastRoleCounter()
{
    m_lastRoleIdCounter =  loadLastRoleIdCounter();
}

std::vector<Role::Ptr> RoleManager::loadByAccount(const std::string& account)
{
    std::vector<Role::Ptr> ret;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select id,name,account,sex,job from roleRarelyUp where account = ";
        query << sql << mysqlpp::quote << account;
        std::vector<RowOfRoleRarelyUp> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("DB:RoleManager::loadByAccount, 数据库中未找到用户, account={}", account);
            ret.clear();
            return ret;
        }
        for(auto it = res.begin(); it != res.end(); it++)
        {
            Role::Ptr role = Role::create(it->id);
            role->m_name = it->name;
            role->m_account = it->account;
            role->m_sex = (Sex)it->sex;
            role->m_job = (Job)it->job;
            ret.push_back(role);
        }
        return ret;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:RoleManager::loadByAccount, DB error:{}", er.what());
        ret.clear();
        return ret;
    }
}

Role::Ptr RoleManager::loadById(RoleId id)
{
    std::vector<Role::Ptr> ret;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sqlOfRarelyUp = "select * from roleRarelyUp where id  = "; 
        query << sqlOfRarelyUp << id;
        std::vector<RowOfRoleRarelyUp> resOfRarelyUp;
        query.storein(resOfRarelyUp);

        query.reset();
        std::string sqlOfOftenUp = "select * from roleOftenUp where id  = "; 
        query << sqlOfOftenUp << id;
        std::vector<RowOfRoleOftenUp> resOfOftenUp;
        query.storein(resOfOftenUp);

        query.reset();
        std::string sqlOfOfflnUp = "select * from roleOfflnUp where id  = "; 
        query << sqlOfOfflnUp << id;
        std::vector<RowOfRoleOfflnUp> resOfOfflnUp;
        query.storein(resOfOfflnUp);

        if(resOfRarelyUp.empty() || resOfOftenUp.empty() || resOfOfflnUp.empty())
        {
            return nullptr;
        }
        //一定只有一个roleId
        auto rarely = resOfRarelyUp.begin();
        auto often = resOfOftenUp.begin();
        auto offln = resOfOfflnUp.begin();
        {   
            Role::Ptr role = Role::create(id);
            //resOfRarelyUp
            role->m_name = rarely->name;
            role->m_turnLife = (TurnLife)rarely->turnLife;
            role->m_account = rarely->account;
            role->m_sex = (Sex)rarely->sex;
            role->m_job = (Job)rarely->job;
            role->m_curObjId = ObjectManager::me().getCurObjIdByRoleId(id);
            role->m_unlockCellNumOfRole = rarely->unlockCellNumOfRole;
            role->m_unlockCellNumOfHero = rarely->unlockCellNumOfHero;
            role->m_unlockCellNumOfStorage = rarely->unlockCellNumOfStorage;
			role->m_defaultCallHero = (Job)rarely->defaultCallHero;
			//role->m_recallHeroTime = Clock::from_time_t(rarely->recallHeroTime); 
			role->m_guanzhiLevel = rarely->guanzhiLevel;
            role->m_bufferVec = rarely->buffer;
			//resOfOftenUp
            role->m_level = often->level;
            role->m_exp = often->exp;
            role->m_money_1 = often->money_1;
            role->m_money_2 = often->money_2;
            role->m_money_3 = often->money_3;
            role->m_money_4 = often->money_4;
            role->m_money_5 = often->money_5;
            role->m_money_6 = often->money_6;
			role->m_money_7 = often->money_7;
			role->m_money_8 = often->money_8;
			role->m_money_9 = often->money_9;
			role->m_money_10 = often->money_10;
            //resOfOfflnUp
            role->m_mp = offln->mp;
            role->m_hp = offln->hp;
            role->m_dir = offln->dir;
            role->m_sceneId = offln->sceneId;
            role->m_posX = (offln->pos >> 16u);
            role->m_posY = (offln->pos & 0x0000ffff);
            role->m_preSceneId = offln->preSceneId;
            role->m_prePosX = (offln->prePos >> 16u);
            role->m_prePosY = (offln->prePos & 0x0000ffff);
            role->m_dead = offln->dead;
            role->m_deathTime = Clock::from_time_t(offln->deathTime);
			role->m_totalOnlineSec = offln->totalOnlineSec;
            role->m_offlnTime = offln->offlnTime;
            role->m_evilVal = offln->evilVal;
            role->m_attackMode = offln->attackMode;

            //这里插入object表的操作（objectManager中）
            auto objDataVec = ObjectManager::me().getObjDataByRoleId(id);
            for(auto it = objDataVec.begin(); it != objDataVec.end(); it++)
            {
                role->m_objDataByKeyMap.insert({it->objId,*it});
            }

			//这里要加入对skill表的操作(skillManager中)
            auto skillDataVec = SkillManager::me().getSkillDataByRoleId(id);
            for(auto it = skillDataVec.begin(); it != skillDataVec.end(); it++)
            {
                role->m_skillDataBykeyMap.insert({it->skillId,*it});
            }

            auto buffDataVec = BuffManager::me().getBuffDataByRoleId(id);
            for(const auto iter : buffDataVec)
            {
                role->m_buffMap.insert({iter.buffId, iter});
            }

            role->m_counters = CounterManager::me().getCounterInfoByRoleId(id);
            
            auto heroInfoVec = HeroManager::me().getHeroInfoByRoleId(id);
            for(const auto& heroInfo : heroInfoVec)
            {
                role->m_heroMap.insert({uint8_t(heroInfo.job), heroInfo});
            }

            auto mailInfoVec = Mail::me().load(id, role->m_curMailIndex);
            for(const auto& iter : mailInfoVec)
            {
                role->m_mailQueue.insert({iter.mailIndex, iter});
            }

            role->m_horseStr = Horse::me().load(id);
           
            auto titleVec = TitleManager::me().getTitleListByRoleId(id);
            for(const auto& iter : titleVec)
            {
                role->m_titleMap.insert({iter.titleId, iter});
            }
	
			auto washPropVec = WashManager::me().getWashPropList(id, SceneItemType::role);
			for(const auto& iter : washPropVec)
			{
				role->m_WashPropMap[iter.washType].push_back(iter);
			}

            role->m_taskStr = TaskManager::me().getTaskInfoByRoleId(id);

            std::string factionTaskInfo = TaskManager::me().getFactionTaskInfo(id);
            role->m_factionTaskInfo = factionTaskInfo;

			auto dragonVec = DragonBallManager::me().getDragonBallList(id);
			for(const auto& iter : dragonVec)
			{
				role->m_dragonMap.insert({iter.type, iter});
			}

            //sundry
            SundryManager::me().fillsundry(role->m_sundry, id);
            SundryManager::me().fillTimerSundry(role->m_timerSundry, id);

			role->m_autoExpMap = ExpAreaManager::me().getExpAreaSecList(id);

            //摆摊日志
            role->m_stallLog = StallLogMgr::me().getStallLogById(id);
            FactionRoleManager::me().fillRoleFactionInfo(role);

			ret.push_back(role);
        }   
        return ret[0];
    }
    catch(const mysqlpp::Exception& er) 
    {   
        LOG_ERROR("DB:RoleManager::loadById, DB error:{}", er.what());
        return nullptr;
    }
}

bool RoleManager::updateRoleOffln(const PrivateRaw::SaveOffline* rev)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "update roleOfflnUp set pos = ";
        query << sql << (int32_t(rev->posX) << 16) + rev->posY
        << ", sceneId = " << rev->sceneId
        << ", preSceneId = " << rev->preSceneId
        << ", prePos = " << (int32_t(rev->prePosX) << 16) + rev->prePosY
        << ", dir = " << (uint16_t)rev->dir
        << ", mp = " << rev->mp 
        << ", hp = " << rev->hp
        << ", dead = " << rev->dead
        << ", deathTime = " << componet::toUnixTime(rev->deathTime)
        << ", totalOnlineSec = " << rev->totalOnlineSec
        << ", offlnTime = " << rev->offlnTime
        << ", evilVal = " << rev->evilVal
        << ", attackMode = " << uint16_t(rev->attackMode)
        << " where id = " << rev->rid;
        query.execute();
        LOG_DEBUG("DB:RoleManager::updateRoleOffln, 更新下线信息成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:RoleManager::updateRoleOffln, DB error:{}",er.what());
        return false;
    }
}

bool RoleManager::updateRoleLevelExp(RoleId rid,uint32_t level,uint64_t exp)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "update roleOftenUp set level = ";
        query << sql << level 
        << ", exp = " << exp 
        << " where id = " << rid;
        query.execute();
        LOG_DEBUG("DB:RoleManager::updateRoleLevelExp, role表level和exp更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:RoleManager::updateRoleLevelExp, DB error:{}", er.what());
        return false;
    }
}


bool RoleManager::updateRoleGuanzhiLevel(RoleId rid, uint32_t level)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "update roleRarelyUp set guanzhiLevel = ";
        query << sql << level 
        << " where id = " << rid;
        query.execute();
        LOG_DEBUG("DB:RoleManager::updateRoleGuanzhiLevel, role表guanzhiLevel更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:RoleManager::updateRoleGuanzhiLevel, DB error:{}",er.what());
        return false;
    }
}

bool RoleManager::updateRoleBufferData(RoleId roleId, const std::string& bufferVec)
{
	try
	{
		mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
		std::string sql = "update roleRarelyUp set buffer = ";
		query << sql << mysqlpp::quote << bufferVec
		<< " where id = " << roleId;
		query.execute();
		LOG_DEBUG("角色, 更新角色的缓存数据, DB, 成功, roleId={}", roleId);
		return true;
	}
	catch(const mysqlpp::Exception& er)
	{
		LOG_ERROR("角色, 更新角色的缓存数据, DB, error:{}", er.what());
		return false;
	}
}

bool RoleManager::updateRolePackageUnlockCellNum(RoleId rid, uint16_t unlockCellNumOfRole, uint16_t unlockCellNumOfHero, uint16_t unlockCellNumOfStorage)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "update roleRarelyUp set unlockCellNumOfRole = ";
        query << sql << unlockCellNumOfRole
        << ", unlockCellNumOfHero = " << unlockCellNumOfHero 
        << ", unlockCellNumOfStorage = " << unlockCellNumOfStorage
        << " where id = " << rid;
        query.execute();
        LOG_DEBUG("DB:RoleManager::updateRolePackageUnlockCellNum, role表背包格子数更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:RoleManager::updateRolePackageUnlockCellNum, DB error:{}",er.what());
        return false;
    }
}

bool RoleManager::updateRoleMoney(const PrivateRaw::UpdateRoleMoney* updateMoney)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update roleOftenUp set ";
        switch(updateMoney->type)
        {
        case MoneyType::money_1:
             query << "money_1";
             break;
        case MoneyType::money_2:
             query << "money_2";
             break;
        case MoneyType::money_3:
             query << "money_3";
             break;
        case MoneyType::money_4:
             query << "money_4";
             break;
        case MoneyType::money_5:
             query << "money_5";
             break;
        case MoneyType::money_6:
             query << "money_6";
             break;
		case MoneyType::money_7:
			 query << "money_7";
			 break;
		case MoneyType::money_8:
			 query << "money_8";
			 break;
		case MoneyType::money_9:
			 query << "money_9";
			 break;
		case MoneyType::money_10:
			 query << "money_10";
			 break;
        default:
             LOG_ERROR("RoleManager::updateRoleMoney, 收到位置类型的money更新消息");
             return false;
        }
        query << " = " << updateMoney->money
        << " where id = " << updateMoney->roleId;
        query.execute();
        LOG_DEBUG("DB:RoleManager::updateRoleMoney, role表money更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:RoleManager::updateRoleMoney, DB error:{}",er.what());
        return false;
    }
}

bool RoleManager::updateRoleDefaultCallHero(RoleId roleId, Job job)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update roleRarelyUp set defaultCallHero = " 
        << uint16_t(job)
        << " where id = " << roleId;
        query.execute();
        LOG_DEBUG("DB:RoleManager::updateRoleDefaultCallHero, DB role表recallHeroTime更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:RoleManager::updateRoleDefaultCallHero, DB error:{}",er.what());
        return false;
    }
}

bool RoleManager::updateRoleTurnLifeLevel(RoleId roleId, TurnLife turnLifeLevel)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update roleRarelyUp set turnLife = " 
        << static_cast<uint16_t>(turnLifeLevel)
        << " where id = " << roleId;
        query.execute();
        LOG_DEBUG("转生, DB更新成功, roleId={}, level={}", roleId, turnLifeLevel);
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("转生, DB更新失败, roleId={}, level={}, DB error:{}", 
				  roleId, turnLifeLevel, er.what());
        return false;
    }
}

std::vector<Role::Ptr> RoleManager::getByAccount(const std::string& account)
{
    std::vector<Role::Ptr> roleList = m_contrRoles.getByAccount(account);
    if(roleList.empty())
    {
        LOG_DEBUG("RoleManager::getByAccount, 缓存容器中未找到该账户,  account={}",account);
        roleList = loadByAccount(account);
    }
    else
    {
        LOG_DEBUG("RoleManager::getByAccount, 缓存容器中找到该账户,  account={}",account);
        return roleList;
    }
    if(roleList.empty())
    {
        LOG_DEBUG("RoleManager::getByAccount, 数据库中未找到该账户,  account={}",account);
        roleList.clear();
        return roleList;
    }
    return roleList;
}

Role::Ptr RoleManager::getById(RoleId id)
{
    Role::Ptr role = m_contrRoles.getById(id);
    if(role == nullptr)
    {
        LOG_DEBUG("RoleManager:getById, 缓存中未找到该Id, id={}", id);
        role  = loadById(id);
        if(role == nullptr)
        {
            LOG_DEBUG("RoleManager:getById, 数据库中未找到该Id, id={}", id);
            return role;
        }
        //加入ContrRoleContainer
        if(!m_contrRoles.insert(role))
        {
            LOG_ERROR("RoleManager getById, 插入ContrRoleContainer失败, roleId={}", role->id());
        }
    }
    return role;
}

bool RoleManager::insert(Role::Ptr role)
{
    if(!role->save(m_lastRoleIdCounter))
    {
        LOG_ERROR("登录, 建角, 插入数据库失败, account={}", role->m_account); 
        --m_lastRoleIdCounter;
        return false;
    }
    return true;
}

void RoleManager::updateOffln(const PrivateRaw::SaveOffline* rev)
{
    if(!updateRoleOffln(rev))
    {
        LOG_ERROR("RoleManager::updateOffln, 角色下线更新失败, roleId={}, sceneId={}, posX={}, posY={}",
                  rev->rid, rev->sceneId, rev->posX, rev->posY);
        return ;
    }
    if(!m_contrRoles.updateOffln(rev))
    {
        LOG_ERROR("RoleManager::updateOffln, 角色下线更新缓存失败，删除缓存相关角色");
		return;
	}
    LOG_TRACE("RoleManager::updateOffln, 角色下线更新成功, roloeId={}, sceneId={}, posX={}, posY={}",
              rev->rid, rev->sceneId, rev->posX, rev->posY);
    
}

void RoleManager::updateLevelExp(RoleId rid,uint32_t level,uint64_t exp)
{
    if(!updateRoleLevelExp(rid,level,exp))
    {
        LOG_ERROR("RoleManager::updateLevelExp, 角色等级经验更新失败, roleId={}, level={}, exp={}",
                  rid, level, exp);
        return ;
    }
    if(!m_contrRoles.updateLevelExp(rid,level,exp))
    {
        LOG_ERROR("RoleManager::updateLevelExp, 角色等级经验更新缓存失败");
		return;
	}
    LOG_TRACE("RoleManager::updateLevelExp, 角色等级经验更新成功, roleId={}, level={}, exp={}",
              rid, level, exp);
}

void RoleManager::updateGuanzhiLevel(RoleId rid, uint32_t level)
{
    if(!updateRoleGuanzhiLevel(rid, level))
    {
        LOG_ERROR("RoleManager::updateGuanzhiLevel, 角色更新官职等级失败, roleId={}, guanzhiLevel={},",
                  rid, level);
        return ;
    }
    if(!m_contrRoles.updateGuanzhiLevel(rid, level))
    {
        LOG_ERROR("RoleManager::updateGuanzhiLevel, 角色更新官职等级缓存失败");
		return;
    }
	LOG_TRACE("RoleManager::updateGuanzhiLevel, 角色更新官职等级成功, roleId={}, guanzhiLevel={}",
			  rid, level);
}

void RoleManager::updateBufferData(RoleId roleId, const std::string& bufferVec)
{
	if(!updateRoleBufferData(roleId, bufferVec))
		return;
	if(!m_contrRoles.updateBufferData(roleId, bufferVec))
		return;
	LOG_TRACE("角色, 更新角色缓存数据, DB及缓存, 成功, roleId={}", roleId);
}

void RoleManager::updatePackageUnlockCellNum(RoleId rid, uint16_t unlockCellNumOfRole, uint16_t unlockCellNumOfHero, uint16_t unlockCellNumOfStorage)
{
    if(!updateRolePackageUnlockCellNum(rid,unlockCellNumOfRole,unlockCellNumOfHero,unlockCellNumOfStorage))
    {
        LOG_ERROR("RoleManager::updatePackageUnlockCellNum, 角色更新背包解锁格子数失败, roleId={}, ofRole={}, ofHero={}, ofStorage={}",
                  rid, unlockCellNumOfRole, unlockCellNumOfHero, unlockCellNumOfStorage);
        return ;
    }
    if(!m_contrRoles.updatePackageUnlockCellNum(rid,unlockCellNumOfRole,unlockCellNumOfHero,unlockCellNumOfStorage))
    {
        LOG_ERROR("RoleManager::updatePackageUnlockCellNum, 角色更新背包解锁格子数缓存失败");
		return;
	}
	LOG_TRACE("RoleManager::updatePackageUnlockCellNum, 角色更新背包解锁格子数成功, roleId={}, ofRole={}, ofHero={}, ofStorage={}",
			  rid, unlockCellNumOfRole, unlockCellNumOfHero, unlockCellNumOfStorage);
}

void RoleManager::updateDefaultCallHero(RoleId roleId, Job job)
{
    if(!updateRoleDefaultCallHero(roleId, job))
    {
        LOG_ERROR("RoleManager::updateDefaultCallHero, defaultCallHero更新失败, roleId = {}, job = {}", 
                  roleId, job);
        return;
    }
    if(!m_contrRoles.updateDefaultCallHero(roleId, job))
    {
        LOG_ERROR("RoleManager::updateDefaultCallHero, defaultCallHero更新缓存失败, roleId = {}, job = {}",
                  roleId, job);
		return;
	}
    LOG_TRACE("defaultCallHero更新成功, roleId = {}, job = {}",
              roleId, job);
}

void RoleManager::updateTurnLifeLevel(RoleId roleId, TurnLife turnLifeLevel)
{
    if(!updateRoleTurnLifeLevel(roleId, turnLifeLevel))
    {
        LOG_ERROR("转生, 更新数据库失败, roleId={}, level={}",  roleId, turnLifeLevel);
        return;
    }
    if(!m_contrRoles.updateTurnLifeLevel(roleId, turnLifeLevel))
    {
        LOG_ERROR("转生, 更新缓存失败, roleId={}, level={}", roleId, turnLifeLevel);
		return;
	}
    LOG_TRACE("转生, 更新DB及缓存成功, roleId={}, level={}", roleId, turnLifeLevel);
}

void RoleManager::updateMoney(const PrivateRaw::UpdateRoleMoney* updateMoney)
{
    if(!updateRoleMoney(updateMoney))
    {
        LOG_ERROR("RoleManager::updateMoney, 角色更新money失败, roleId={}, monyeType={}, money={}",
                  updateMoney->roleId, updateMoney->type, updateMoney->money);
        return;
    }
    if(!m_contrRoles.updateMoney(updateMoney))
    {
        LOG_ERROR("RoleManager::updateMoney, 角色更新money缓存失败");
		return;
	}
    LOG_TRACE("RoleManager::updateMoney, 角色更新money成功, roleId={}, monyeType={}, money={}", 
              updateMoney->roleId, updateMoney->type, updateMoney->money);
}

Hero::Ptr RoleManager::getHeroByRoleIdAndJob(RoleId roleId, Job job)
{
    auto role = getById(roleId);
    auto it = role->m_heros.find(uint8_t(job));
    Hero::Ptr hero = nullptr;
    if(it == role->m_heros.end())
    {
        LOG_DEBUG("RoleManager::getHeroByRoleIdAndJob, 缓存中没有该英雄, roleId={}, job={}",
                  roleId, job);
        hero = loadHeroData(roleId, job);
        if(hero == nullptr)
        {
            LOG_DEBUG("RoleManager::getHeroByRoleIdAndJob, 数据库中未找到该英雄, roleId={}, job={}",
                      roleId, job);
            return hero;
        }
        //插入缓存
        role->m_heros.insert({(uint8_t)job,hero});
    }
    else
        hero = it->second;
    return hero;
}

bool RoleManager::isExisitName(const std::string& name)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select id from roleRarelyUp where name = ";
        query << sql << mysqlpp::quote << name;
        std::vector<RowOfRoleRarelyUp> res;
        query.storein(res);
        if(res.size() == 0)
        {
            return false;
        }
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB: RoleManager::isExisitName, DB error: {}",er.what());
        return false;
    }
}

Hero::Ptr RoleManager::loadHeroData(RoleId roleId, Job job)
{
    Hero::Ptr hero = Hero::create(job);
    try
    {
        auto objDataVec = ObjectManager::me().getHeroObjData(roleId, job);
        for(auto it = objDataVec.begin(); it != objDataVec.end(); it++)
        {
            hero->m_objDataByKeyMap.insert({it->objId,*it});
        }
        auto skillDataVec = SkillManager::me().getHeroSkillData(roleId, job);
        for(auto it = skillDataVec.begin(); it != skillDataVec.end(); it++)
        {
            hero->m_skillDataBykeyMap.insert({it->skillId,*it});
        }
        auto bufDataVec = BuffManager::me().getHeroBuffData(roleId, job);
        for(auto it = bufDataVec.begin(); it != bufDataVec.end(); it++)
        {
            hero->m_buffMap.insert({it->buffId, *it});
        }
		auto washPropVec = WashManager::me().getWashPropList(roleId, SceneItemType::hero);
		for(auto iter = washPropVec.begin(); iter != washPropVec.end(); ++iter)
		{
			hero->m_WashPropMap[iter->washType].push_back(*iter);
		}

        return hero;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_DEBUG("RoleManager::loadHeroData, DB error:{}", er.what());
        return nullptr;
    }
}

RoleId RoleManager::createRid()
{
    RoleId id = ++m_lastRoleIdCounter;
    id |= (RoleId(DbCached::me().zoneId()) << 32u);
    return id;
}

uint32_t RoleManager::loadLastRoleIdCounter()
{
    //此函数不处理异常
    mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
    std::string sql = "select name from roleRarelyUp where id = 0";
    query << sql;
    mysqlpp::StoreQueryResult ret = query.store();
    if(ret.size() == 0)
    {
        //不存在特殊处理行，添加，id=0,name=0;
        query.reset();
        std::string sql = "INSERT INTO roleRarelyUp VALUES (0,'0',0,'none',0,0,0,0,0,0,0,'')";
        query << sql;
        query.execute();
        LOG_DEBUG("DB:数据库初始化，加入role的特殊行,成功");
        return 0;
    }
    std::string roleId = std::string(ret[0]["name"]);
    uint32_t id =  componet::fromString<uint32_t>(roleId);
    LOG_TRACE("DB: 得到最后的roleIdCounter,成功");
    return id;
}

/*
 void RoleManager::test()
{
    LOG_DEBUG("test 开始");
    PrivateRaw::UpdateRecallTimePoint data;
    data.roleId = 4294967297;
    data.recallTimePoint = water::componet::Clock::now();
    servermsg_UpdateRecallTimePoint((uint8_t *)(&data),sizeof(data),uint64_t(1));
}
*/
}
