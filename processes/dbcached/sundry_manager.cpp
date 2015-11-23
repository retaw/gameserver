#include "sundry_manager.h"
#include "role_manager.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "water/process/process_id.h" 
#include "water/componet/logger.h"
#include "sundry_table_structure.h"
#include <vector>


namespace dbcached{

using namespace water::componet;

SundryManager& SundryManager::me()
{
    static SundryManager me;
    return me;
}


void SundryManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(SundryToDB, std::bind(&SundryManager::servermsg_SundryToDB, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(TimerSundryToDB, std::bind(&SundryManager::servermsg_TimerSundryToDB, this, _1, _2, _3));
}

void SundryManager::servermsg_SundryToDB(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到world的杂项角色数据");
    auto rev = reinterpret_cast<const PrivateRaw::SundryToDB*>(msgData);
    std::string data;
    data.append(rev->data, rev->size);
    modify(rev->roleId, data);
}

void SundryManager::servermsg_TimerSundryToDB(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    LOG_DEBUG("收到world的频繁杂项角色数据");
    auto rev = reinterpret_cast<const PrivateRaw::TimerSundryToDB*>(msgData);
    std::string data;
    data.append(rev->data, rev->size);
    modifyTimer(rev->roleId, data);
}

void SundryManager::modify(RoleId roleId, std::string& sundry)
{
    try
    {
        RowOfSundry row;
        row.roleId = roleId;
        row.data= sundry;
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query.replace(row);
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("数据杂项入库失败, DB error:{}", er.what());
    }

    RoleManager::me().m_contrRoles.updateSundry(roleId, sundry);
}

void SundryManager::modifyTimer(RoleId roleId, std::string& sundry)
{
    try
    {
        RowOfTimerSundry row;
        row.roleId = roleId;
        row.data= sundry;
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query.replace(row);
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("频繁数据杂项入库失败, DB error:{}", er.what());
    }

    RoleManager::me().m_contrRoles.updateTimerSundry(roleId, sundry);
}

void SundryManager::fillsundry(std::string& sundry, const RoleId roleId)
{
    try
    {
        std::vector<RowOfSundry> ret;
        ret.clear();
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select data from sundry where roleId = " << roleId;
        query.storein(ret);
        if(ret.empty())
        {
            sundry.clear();
            return;
        }
        else
        {
            sundry = ret[0].data;
            return;
        }
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("数据库杂项读取失败");
    }
}

void SundryManager::fillTimerSundry(std::string& sundry, const RoleId roleId)
{
    try
    {
        std::vector<RowOfTimerSundry> ret;
        ret.clear();
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select data from timerSundry where roleId = " << roleId;
        query.storein(ret);
        if(ret.empty())
        {
            sundry.clear();
            return;
        }
        else
        {
            sundry = ret[0].data;
            return;
        }
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("数据库频繁杂项读取失败");
    }
}

}
