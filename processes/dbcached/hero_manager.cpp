#include "role_manager.h"
#include "hero_manager.h"
#include "water/componet/logger.h"
#include "water/componet/datetime.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/hero.codedef.private.h"

#include "hero_table_structure.h"

namespace dbcached{

HeroManager& HeroManager::me()
{
    static HeroManager me;
    return me;
}
void HeroManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(InsertHeroInfo, std::bind(&HeroManager::servermsg_InsertHeroInfo, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(UpdateHeroLevelExp, std::bind(&HeroManager::servermsg_UpdateHeroLevelExp, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(UpdateHeroTurnLifeLevel, std::bind(&HeroManager::servermsg_UpdateHeroTurnLifeLevel, this, _1, _2));
    REG_RAWMSG_PRIVATE(SaveHeroOffline, std::bind(&HeroManager::servermsg_SaveHeroOffline, this, _1, _2));
    REG_RAWMSG_PRIVATE(UpdateHeroClothes, std::bind(&HeroManager::servermsg_UpdateHeroClothes, this, _1, _2));
}

void HeroManager::servermsg_InsertHeroInfo(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::InsertHeroInfo*>(msgData);
    LOG_DEBUG("收到英雄修改请求");
    updateOrInsert(rev);
}

void HeroManager::servermsg_UpdateHeroLevelExp(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateHeroLevelExp*>(msgData);
    LOG_DEBUG("收到英雄等级经验更新");
    updateHeroLevelExp(rev);
}

void HeroManager::servermsg_UpdateHeroTurnLifeLevel(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateHeroTurnLifeLevel*>(msgData);
    LOG_DEBUG("收到英雄转生等级更新");
    updateHeroTurnLifeLevel(rev);
}

void HeroManager::servermsg_SaveHeroOffline(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::SaveHeroOffline*>(msgData);
    LOG_DEBUG("收到英雄下线保存数据");
    saveHeroOffline(rev);
}

void HeroManager::servermsg_UpdateHeroClothes(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateHeroClothes*>(msgData);
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update hero set clother = " << rev->clother
        << " where roleId = "
        << rev->roleId << " and job = "
        << (uint16_t)rev->job;
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:HeroManager::UpdateHeroClothes, DB error:{}", er.what());
        return;
    }
    RoleManager::me().m_contrRoles.updateHeroClother(rev);
}

void HeroManager::updateOrInsert(const PrivateRaw::InsertHeroInfo* rev)
{
    if(insertHero(rev))
    {
        LOG_DEBUG("更新英雄信息, 成功, roleId= {}, job = {}, sex = {}, level = {}",
                  rev->roleId, rev->data.job, rev->data.sex, rev->data.level);
        if(!RoleManager::me().m_contrRoles.insertHero(rev->data, rev->roleId))
            LOG_ERROR("更新英雄缓存信息, 失败, roleId= {}, job = {}, sex = {}, level = {}",
                      rev->roleId, rev->data.job, rev->data.sex, rev->data.level);
            
    }
    else
    {
        LOG_ERROR("更新英雄信息, 失败, roleId= {}, job = {}, sex = {}, level = {}",
                  rev->roleId, rev->data.job, rev->data.sex, rev->data.level);
    }
}

void HeroManager::updateHeroLevelExp(const PrivateRaw::UpdateHeroLevelExp* rev)
{
    if(updateDBHeroLevelExp(rev))
    {
        LOG_DEBUG("更新英雄等级经验, 成功, roleId = {}, job = {}, level = {}, exp = {}",
                  rev->roleId, rev->job, rev->level, rev->exp);
        if(!RoleManager::me().m_contrRoles.updateHeroLevelExp(rev))
            LOG_ERROR("更新英雄缓存等级经验, 失败");
    }
    else
    {
        LOG_ERROR("更新英雄等级经验, 失败, roleId = {}, job = {}, level = {}, exp = {}",
                  rev->roleId, rev->job, rev->level, rev->exp);
        
    }
}

void HeroManager::updateHeroTurnLifeLevel(const PrivateRaw::UpdateHeroTurnLifeLevel* rev)
{
    if(updateDBHeroTurnLifeLevel(rev))
    {
        LOG_DEBUG("更新英雄转生等级, 成功, roleId = {}, job = {}, turnLifelevel = {}",
                  rev->roleId, rev->job, rev->turnLifeLevel);
        if(!RoleManager::me().m_contrRoles.updateHeroTurnLifeLevel(rev))
            LOG_ERROR("更新英雄缓存转生等级, 失败");
    }
    else
    {
        LOG_ERROR("更新英雄转生等级, 失败, roleId = {}, job = {}, turnLifeLevel = {}",
                  rev->roleId, rev->job, rev->turnLifeLevel);
        
    }
}

void HeroManager::saveHeroOffline(const PrivateRaw::SaveHeroOffline* rev)
{
    if(saveDBHeroOffline(rev))
    {
        LOG_DEBUG("保存英雄下线数据, 成功, roleId = {}, job = {}, hp = {}, mp = {}, time = {}",
                  rev->roleId, rev->job, rev->hp, rev->mp, water::componet::timePointToString(rev->recallTimePoint));
        if(!RoleManager::me().m_contrRoles.saveHeroOffline(rev))
            LOG_ERROR("保存英雄缓存下线数据, 失败");
    }
    else
    {
        LOG_ERROR("保存英雄等级经验, 失败, roleId = {}, job = {}, hp = {}, mp = {}, time = {}",
                  rev->roleId, rev->job, rev->hp, rev->mp, water::componet::timePointToString(rev->recallTimePoint));
        
    }
}

bool HeroManager::insertHero(const PrivateRaw::InsertHeroInfo* heroInfo)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfHero row(uint8_t(heroInfo->data.job), heroInfo->roleId, uint8_t(heroInfo->data.sex), heroInfo->data.level, heroInfo->data.exp, heroInfo->data.hp, heroInfo->data.mp, uint8_t(heroInfo->data.turnLife), heroInfo->data.clother);
        query.replace(row);
        query.execute();
        LOG_DEBUG("HeroManager::insertHero, 入库或更新库, 成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("HeroManager::insertHero, DB error:{}", er.what());
        return false;
    }
}

bool HeroManager::updateDBHeroLevelExp(const PrivateRaw::UpdateHeroLevelExp* rev)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update hero set level = " 
        << rev->level << " ,exp = " 
        << rev->exp << " where roleId = "
        << rev->roleId << " and job = "
        << (uint16_t)rev->job;
        query.execute();
        LOG_DEBUG("DB:HeroManager::updateDBHeroLevelExp, hero等级经验更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:HeroManager::updateDBHeroLevelExp, DB error:{}", er.what());
        return false;
    }
}

bool HeroManager::updateDBHeroTurnLifeLevel(const PrivateRaw::UpdateHeroTurnLifeLevel* rev)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update hero set turnLife = " 
        << (uint16_t)rev->turnLifeLevel << " where roleId = "
        << rev->roleId << " and job = "
        << (uint16_t)rev->job;
        query.execute();
        LOG_DEBUG("DB:HeroManager::updateDBHeroTurnLifeLevel, hero转生等级更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:HeroManager::updateDBHeroTurnLifeLevel, DB error:{}", er.what());
        return false;
    }
}

bool HeroManager::saveDBHeroOffline(const PrivateRaw::SaveHeroOffline* rev)
{
    try
    {
        mysqlpp::Query queryF = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        queryF << "update hero set hp = " 
        << rev->hp << " and mp = " 
        << rev->mp << " where roleId = "
        << rev->roleId << " and job = "
        << (uint16_t)rev->job << " and clother = "
        << rev->clother;

        queryF.execute();
        LOG_DEBUG("DB:HeroManager::saveDBHeroOffline, hero转生等级更新成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:HeroManager::saveDBHeroOffline, DB error:{}", er.what());
        return false;
    }
}

std::vector<HeroInfoPra> HeroManager::getHeroInfoByRoleId(RoleId roleId)
{
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::vector<HeroInfoPra> ret;
        std::vector<RowOfHero> rowVec;
        query << "select * from hero where roleId = " << roleId;
        query.storein(rowVec);
        for(auto it = rowVec.begin(); it != rowVec.end(); it++)
        {
            HeroInfoPra info;
            info.job = Job(it->job);
            info.sex = Sex(it->sex);
            info.level = it->level;
			info.exp = it->exp;
            info.hp = it->hp;
            info.mp = it->mp;
			info.turnLife = static_cast<TurnLife>(it->turnLife);
            info.clother = it->clother;
            ret.push_back(info);
        }
        LOG_DEBUG("HeroManager::getHeroInfoByRoleId, 成功");
        return ret;
}


}
