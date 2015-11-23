#include "buff_manager.h"
#include "role_manager.h"
#include "protocol/rawmsg/private/buff.h"
#include "protocol/rawmsg/private/buff.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "water/process/process_id.h" 
#include "water/componet/logger.h"
#include "water/componet/datetime.h"
#include "buff_table_structure.h"
#include <vector>


namespace dbcached{

using namespace water::componet;

BuffManager& BuffManager::me()
{
    static BuffManager me;
    return me;
}

void BuffManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(ModifyBuffData, std::bind(&BuffManager::servermsg_ModifyBuffData, this, _1, _2, _3));
}

void BuffManager::servermsg_ModifyBuffData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto msg = reinterpret_cast<const PrivateRaw::ModifyBuffData*>(msgData);
    for(ArraySize i = 0; i < msg->size; ++i)
    {
        switch(msg->modifyType)
        {
        case ModifyType::modify: 
            if(msg->sceneItem == uint8_t(1))
                updateOrInsert(msg->data[i], msg->roleId);
            if(msg->sceneItem == uint8_t(2))
                updateOrInsertHero(msg->data[i], msg->roleId, msg->job);
            if(msg->sceneItem == 3)
                RoleManager::me().m_contrRoles.updatePetBuff(msg->data[i], msg->roleId, msg->ownerSceneItemType, msg->ownerJob);
            break;
        case ModifyType::erase:
            if(msg->sceneItem == uint8_t(1))
                erase(msg->data[i], msg->roleId);
            if(msg->sceneItem == uint8_t(2))
                eraseHero(msg->data[i], msg->roleId, msg->job);
            if(msg->sceneItem == 3)
                RoleManager::me().m_contrRoles.erasePetBuff(msg->data[i], msg->roleId, msg->ownerSceneItemType, msg->ownerJob);
            break;
        default:
            LOG_ERROR("BuffManager::servermsg_ModifyBuffData, 错误的ModifyType, 不是insert, erase, update中的一种");
            break;
        }
    }
}


void BuffManager::updateOrInsert(const BuffData& data, RoleId roleId)
{
    if(updateOrInsertBuff(data, roleId))
    {
        if(!RoleManager::me().m_contrRoles.updateOrInsertBuff(data, roleId))
        {
            LOG_ERROR("BuffManager::updateOrInsert, buff缓存更新失败");
        }
        LOG_TRACE("BuffManager::updateOrInsert, buff表更新成功, roleId={}, buffId={}, sec={}, endtime={}, dur={}",
                  roleId, data.buffId, data.sec, data.endtime, data.dur);
        return;
    }
    LOG_ERROR("BuffManager::updateOrInsert, buff表更新失败, roleId={}, buffId={}, sec={}, endtime={}, dur={}",
              roleId, data.buffId, data.sec, data.endtime, data.dur);
}

void BuffManager::updateOrInsertHero(const BuffData& data, RoleId roleId, Job job)
{
    if(updateOrInsertHeroBuff(data, roleId, job))
    {
        if(!RoleManager::me().m_contrRoles.updateOrInsertHeroBuff(data, roleId, job))
        {
            LOG_ERROR("BuffManager::updateOrInsertHero, heroBuf缓存更新失败");
        }
        LOG_TRACE("BuffManager::updateOrInsertHero, heroBuf更新成功, roleId={}, job={}, buffId={}, sec={}, endtime={}, dur={}",
                  roleId, job, data.buffId, data.sec, data.endtime, data.dur);
        return;
    }
    LOG_ERROR("BuffManager::updateOrInsertHero, heroBuff表更新失败, roleId={}, job={}, buffId={}, sec={}, endtime={}, dur={}",
              roleId, job, data.buffId, data.sec, data.endtime, data.dur);
}

void BuffManager::erase(const BuffData& data, RoleId roleId)
{
    if(eraseBuff(data, roleId))
    {
        if(!RoleManager::me().m_contrRoles.eraseBuff(data, roleId))
        {
            LOG_ERROR("BuffManager::erase, buf缓存更新失败");
        }
        LOG_TRACE("BuffManager::erase, buff删除成功, roleId={}, buffId={}, sec={}, endtime={}, dur={}",
                  roleId, data.buffId, data.sec, data.endtime, data.dur);
        return;
    }
    LOG_ERROR("BuffManager::erase, buff删除失败, roleId={}, buffId={}, sec={}, endtime={}, dur={}",
              roleId, data.buffId, data.sec, data.endtime, data.dur);
}

void BuffManager::eraseHero(const BuffData& data, RoleId roleId, Job job)
{
    if(eraseHeroBuff(data, roleId, job))
    {
        if(!RoleManager::me().m_contrRoles.eraseHeroBuff(data, roleId, job))
        {
            LOG_ERROR("BuffManager::eraseHero, heroBuf缓存删除失败");
        }
        LOG_TRACE("BuffManager::eraseHero, buff删除成功, roleId={}, buffId={}, sec={}, endtime={}, dur={}",
                  roleId, data.buffId, data.sec, data.endtime, data.dur);
        return;
    }
    LOG_ERROR("BuffManager::eraseHero, HeroBuff删除失败, roleId={}, buffId={}, sec={}, endtime={}, dur={}",
              roleId, data.buffId, data.sec, data.endtime, data.dur);
}

//基本skill表操作
std::vector<BuffData> BuffManager::getBuffDataByRoleId(RoleId roleId)
{
    std::vector<BuffData> buffDataVec;
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select buffId, sec, endtime, dur from buff where roleId = ";
        query << sql << roleId;
        std::vector<RowOfBuff> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("DB:BuffManager::getBuffDataByRoleId, buff表为空roleId={}",roleId);
            buffDataVec.clear();
            return buffDataVec;
        }
        for(auto it = res.begin(); it != res.end(); it++)
        {
            BuffData data;
            data.buffId = it->buffId;
            data.sec = it->sec;
            data.endtime = it->endtime;
            data.dur = it->dur;
            if(it->endtime <= toUnixTime(Clock::now()))
            {
                erase(data, roleId);
                continue;
            }
            buffDataVec.emplace_back(data);
        }
        return buffDataVec;
    }
}

std::vector<BuffData> BuffManager::getHeroBuffData(RoleId roleId, Job job)
{
    std::vector<BuffData> buffDataVec;
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select buffId, sec, endtime, dur from heroBuff where roleId = ";
        query << sql << roleId
        << " and job = " << uint16_t(job);
        std::vector<RowOfHeroBuff> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("DB:BuffManager::getHeroBuffData, buff表为空roleId={}",roleId);
            buffDataVec.clear();
            return buffDataVec;
        }
        for(auto it = res.begin(); it != res.end(); it++)
        {
            BuffData data;
            data.buffId = it->buffId;
            data.sec = it->sec;
            data.endtime = it->endtime;
            data.dur = it->dur;
            if(it->endtime <= toUnixTime(Clock::now()))
            {
                erase(data, roleId);
                continue;
            }
            buffDataVec.emplace_back(data);
        }
        return buffDataVec;
    }
}

/*
bool BuffManager::isExist(const BuffData& data,RoleId roleId)
{
    //异常抛给上层处理
    mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
    std::string sql = "select buffId, sec, endtime, dur from buff where roleId = ";
    query << sql << roleId 
    << " and buffId = " << data.buffId << " limit 1";
    std::vector<RowOfBuff> res;
    query.storein(res);
    return res.size() > 0;
}
*/

bool BuffManager::updateOrInsertBuff(const BuffData& data, RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfBuff buffRow(roleId,data.buffId, data.sec, data.endtime, data.dur);
        query.replace(buffRow);
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_DEBUG("DB:BuffManager::updateOrInsertbuff, DB error:{}", er.what());
         return false;
    }
}

bool BuffManager::updateOrInsertHeroBuff(const BuffData& data, RoleId roleId, Job job)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfHeroBuff buffRow(roleId, uint8_t(job), data.buffId, data.sec, data.endtime, data.dur);
        query.replace(buffRow);
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_DEBUG("DB:BuffManager::updateOrInsertHeroBuff, DB error:{}", er.what());
         return false;
    }
}

bool BuffManager::eraseBuff(const BuffData& data, RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from buff where roleId = " << roleId 
        << " and buffId = " << data.buffId;
        query.execute();
        LOG_TRACE("BuffManager::eraseBuff, 删除buff成功, buffId={}, roleId={}", data.buffId, roleId);
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:BuffManager::eraseBuff, DB error:{}",er.what());
        return false;
    }
}

bool BuffManager::eraseHeroBuff(const BuffData& data, RoleId roleId, Job job)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from heroBuff where roleId = " << roleId 
        << " and buffId = " << data.buffId
        << " and job = " << (uint16_t)job;
        query.execute();
        LOG_DEBUG("BuffManager::eraseHeroBuff, 删除heroBuff成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB:BuffManager::eraseHeroBuff, DB error:{}",er.what());
        return false;
    }
}

}
