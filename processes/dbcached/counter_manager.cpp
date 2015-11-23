#include "counter_manager.h"
#include "counter_table_structure.h"
#include "role_manager.h"

#include "water/componet/logger.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/counter.h"
#include "protocol/rawmsg/private/counter.codedef.private.h"


namespace dbcached{

using namespace water::componet;

CounterManager& CounterManager::me()
{
    static CounterManager me;
    return me;
}


void CounterManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(ModifyCounterInfo, std::bind(&CounterManager::servermsg_ModifyCounterInfo, this, _1, _2, _3));
}

void CounterManager::servermsg_ModifyCounterInfo(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::ModifyCounterInfo*>(msgData);
    std::string counterStr("");
    counterStr.append(rev->buf, rev->size);
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfRoleCounter counterRow(rev->roleId, counterStr);
        query.replace(counterRow);
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
        //出错只打印底层错误
         LOG_ERROR("DB;CounterManager::insert(),DB error:{}",er.what());
         return;
    }

    RoleManager::me().m_contrRoles.updateCounter(counterStr, rev->roleId);
}

std::string CounterManager::getCounterInfoByRoleId(RoleId roleId)
{
    std::string out("");
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select * from roleCounter where roleId = ";
        query << sql << roleId << " limit 1";
        std::vector<RowOfRoleCounter> res;
        query.storein(res);
        if(res.empty())
            LOG_DEBUG("DB:CounterManager::getCounterInfoByRoleId()空,请求roleId={}",roleId);
        else
            out = res[0].counterStr;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB CounterManager::getCounterInfoByRoleId(), error:{}", er.what());
    }

    return out;
}

}
