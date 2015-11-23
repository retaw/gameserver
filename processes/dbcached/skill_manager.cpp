#include "skill_manager.h"
#include "role_manager.h"
#include "protocol/rawmsg/private/skill.h"
#include "protocol/rawmsg/private/skill.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "water/process/process_id.h" 
#include "water/componet/logger.h"
#include "skill_table_structure.h"
#include <vector>


namespace dbcached{

SkillManager& SkillManager::me()
{
    static SkillManager me;
    return me;
}


void SkillManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(ModifySkillData, std::bind(&SkillManager::servermsg_ModifySkillData, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(CachePKCdStatus, std::bind(&SkillManager::servermsg_SavePKCdStatus, this, _1, _2, _3));
}

void SkillManager::servermsg_ModifySkillData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto msg = reinterpret_cast<const PrivateRaw::ModifySkillData*>(msgData);
    for(ArraySize i = 0; i < msg->size; ++i)
    {
        switch(msg->modifyType)
        {
        case ModifyType::modify: 
            if(msg->sceneItem == uint8_t(1))
                updateOrInsert(msg->data[i], msg->roleId);
            if(msg->sceneItem == uint8_t(2))
                updateOrInsertHero(msg->data[i], msg->roleId, msg->job);
            break;
        case ModifyType::erase:
            if(msg->sceneItem == uint8_t(1))
                erase(msg->data[i], msg->roleId);
            if(msg->sceneItem == uint8_t(2))
                eraseHero(msg->data[i], msg->roleId, msg->job);
            break;
        default:
            LOG_ERROR("SkillManager::servermsg_ModifySkillData, 错误的ModifyType, 不是insert, erase, update中的一种");
            break;
        }
    }
}

void SkillManager::servermsg_SavePKCdStatus(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::CachePKCdStatus*>(msgData);
    if(rev->sceneItem == uint8_t(1))
    {
        if(!RoleManager::me().m_contrRoles.savePKCdStatus(rev))
        {
            LOG_ERROR("SkillManager::servermsg_SavePKCdStatus, 缓存pkCd状态失败");
            return;
        }
        LOG_DEBUG("SkillManager::servermsg_SavePKCdStatus, 缓存pkCd状态成功");
    }
    if(rev->sceneItem == uint8_t(2))
    {
        if(!RoleManager::me().m_contrRoles.saveHeroPKCdStatus(rev))
        {
            LOG_ERROR("SkillManager::servermsg_SavePKCdStatus, 缓存heropkCd状态失败");
            return;
        }
        LOG_DEBUG("SkillManager::servermsg_SavePKCdStatus, 缓存heropkCd状态成功");
    }
}

void SkillManager:: updateOrInsert(const SkillData& modifySkill,RoleId roleId)
{
    if(updateOrInsertSkill(modifySkill,roleId))
    {
        if(!RoleManager::me().m_contrRoles.updateOrInsertSkill(modifySkill,roleId))
        {
            LOG_ERROR("SkillManager::updateOrInsert, skill缓存update失败");
        }
        /*
        LOG_TRACE("SkillManager::updateOrInsert, skill表update成功, roleId={}, skillId={}, skillLv={}, strengthenLv={}, exp={}",
                  roleId, modifySkill.skillId, modifySkill.skillLv,
                  modifySkill.strengthenLv, modifySkill.exp);
        */
        return;
    }
}

void SkillManager:: updateOrInsertHero(const SkillData& modifySkill, RoleId roleId, Job job)
{
    if(updateOrInsertHeroSkill(modifySkill, roleId, job))
    {
        if(!RoleManager::me().m_contrRoles.updateOrInsertHeroSkill(modifySkill, roleId, job))
        {
            LOG_ERROR("SkillManager::updateOrInsertHero, heroSkill缓存修改失败");
        }
        /*
        LOG_TRACE("SkillManager::updateOrInsertHero, heroSkill表修改成功, roleId={}, job={}, skillId={}, skillLv={}, strengthenLv={}, exp={}",
                  roleId, job, 
                  modifySkill.skillId, modifySkill.skillLv,
                  modifySkill.strengthenLv, modifySkill.exp);
        */
    }
}

void SkillManager::erase(const SkillData& modifySkill,RoleId roleId)
{
    if(eraseSkill(modifySkill,roleId))
    {
        if(!RoleManager::me().m_contrRoles.eraseSkill(modifySkill,roleId))
        {
            LOG_ERROR("SkillManager::erase, skill缓存删除失败");
        }
        return ;
    }
}

void SkillManager::eraseHero(const SkillData& modifySkill, RoleId roleId, Job job)
{
    if(eraseHeroSkill(modifySkill, roleId, job))
    {
        if(!RoleManager::me().m_contrRoles.eraseHeroSkill(modifySkill, roleId, job))
        {
            LOG_ERROR("SkillManager::eraseHero, heroSkill缓存删除失败");
        }
        return ;
    }
}

//基本skill表操作
std::vector<SkillData> SkillManager::getSkillDataByRoleId(RoleId roleId)
{
    std::vector<SkillData> skillDataVec;
    //异常要抛出，有上层处理，这是为了保证role角色的缓存准确，防止此处异常，而role存入缓存的情况出现
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select * from skill where roleId = ";
        query << sql << roleId;
        std::vector<RowOfSkill> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("DB:SkillManager::getSkillDataByRoleId, 请求技能为空, roleId={}", roleId);
            skillDataVec.clear();
            return skillDataVec;
        }
        for(auto it = res.begin(); it != res.end(); it++)
        {
            SkillData skillData;
            skillData.skillId = it->skillId;
            skillData.skillLv = it->skillLv;
            skillData.strengthenLv = it->strengthenLv;
            skillData.exp = it->exp;
            skillDataVec.emplace_back(skillData);
        }
        return skillDataVec;
    }
}

bool SkillManager::updateOrInsertSkill(const SkillData& modifySkill, RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfSkill skillRow(roleId, modifySkill.skillId, modifySkill.skillLv, modifySkill.strengthenLv, modifySkill.exp);
        query.replace(skillRow);
        query.execute();
        LOG_DEBUG("DB:SkillManager::updateOrInsertSkill, skill更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_ERROR("DB:SkillManager::updateOrInsertSkill, DB error:{}", er.what());
         return false;
    }
}

bool SkillManager::eraseSkill(const SkillData& modifySkill,RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from skill where roleId = " << roleId 
        << " and skillId = " << modifySkill.skillId;
        query.execute();
        LOG_DEBUG("DB:SkillManager::eraseSkill, skill更新数据成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:SkillManager::eraseSkill, DB error:{}", er.what());
        return true;
    }
}

std::vector<SkillData> SkillManager::getHeroSkillData(RoleId roleId, Job job)
{
    std::vector<SkillData> skillDataVec;
    //异常给上层处理
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select * from heroSkill where roleId = ";
        query << sql << roleId
        << " and job = " << uint16_t(job);
        std::vector<RowOfHeroSkill> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("DB:SkillManager::getHeroSkillData, 请求技能为空, roleId={}, job={}",
                      roleId, job);
            skillDataVec.clear();
            return skillDataVec;
        }
        for(auto it = res.begin(); it != res.end(); it++)
        {
            SkillData skillData;
            skillData.skillId = it->skillId;
            skillData.skillLv = it->skillLv;
            skillData.strengthenLv = it->strengthenLv;
            skillData.exp = it->exp;
            skillDataVec.emplace_back(skillData);
        }
        return skillDataVec;
    }
}

bool SkillManager::updateOrInsertHeroSkill(const SkillData& modifySkill, RoleId roleId, Job job)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfHeroSkill skillRow(roleId,uint8_t(job),modifySkill.skillId,modifySkill.skillLv,modifySkill.strengthenLv,modifySkill.exp);
        query.replace(skillRow);
        query.execute();
        LOG_DEBUG("DB:SkillManager::updateOrInsertHeroSkill, heroSkill修改成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:SkillManager::updateOrInsertHeroSkill, DB error:{}", er.what());
        return false;
    }
}

bool SkillManager::eraseHeroSkill(const SkillData& modifySkill, RoleId roleId, Job job)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from heroSkill where roleId = " << roleId
        << "and job = " << (uint16_t)job
        << " and skillId = " << modifySkill.skillId;
        query.execute();
        LOG_DEBUG("DB:SkillManager::eraseHeroSkill, skill更新数据成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:SkillManager::eraseHeroSkill, DB error:{}",er.what());
        return true;
    }

}

}
