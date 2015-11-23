#include "stall_sell_log.h"
#include "stallLog_table_structure.h"
#include "role_manager.h"

#include "water/componet/logger.h"
#include "water/componet/serialize.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/stall_log.h"
#include "protocol/rawmsg/private/stall_log.codedef.private.h"


namespace dbcached{

using namespace water::componet;

StallLogMgr& StallLogMgr::me()
{
    static StallLogMgr me;
    return me;
}


void StallLogMgr::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(SaveStallLog, std::bind(&StallLogMgr::servermsg_SaveStallLog, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(WorldReqStallLog, std::bind(&StallLogMgr::servermsg_WorldReqStallLog, this, _1, _2, _3));
}

void StallLogMgr::servermsg_SaveStallLog(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::SaveStallLog*>(msgData);
    std::string log("");
    log.append(rev->logs, rev->size);

    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfStallLog logRow(rev->roleId, log);
        query.replace(logRow);
        query.execute();
    }
    catch(const mysqlpp::Exception & er)
    {
        LOG_ERROR("DB StallLogMgr::saveStallLog error:{}", er.what());
        return;
    }

    RoleManager::me().m_contrRoles.updateStallLog(rev->roleId, log);
}

void StallLogMgr::servermsg_WorldReqStallLog(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::WorldReqStallLog*>(msgData);
    auto role = RoleManager::me().m_contrRoles.getById(rev->roleId);
    if(nullptr == role)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(2048);
    std::string log = role->stallLog();
    uint32_t logLen = log.length();

    buf.resize(sizeof(PrivateRaw::DBRetStallLog) + logLen);
    auto msg = reinterpret_cast<PrivateRaw::DBRetStallLog*>(buf.data());
    msg->roleId = rev->roleId;
    msg->size = logLen;
    log.copy(msg->logs, logLen);

    ProcessIdentity pid(remoteProcessId);
    DbCached::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(DBRetStallLog), buf.data(), buf.size());
}

std::string StallLogMgr::getStallLogById(RoleId roleId)
{
    std::string out("");
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select * from stallSellLog where roleId = ";
        query << sql << roleId << " limit 1";
        std::vector<RowOfStallLog> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("DB:StallLogMgr::getStallLogById()空,请求roleId={}",roleId);
            return out;
        }
        else
        {
            out = res[0].blobStr;
        }
    }
    catch(const mysqlpp::Exception & er)
    {
        LOG_DEBUG("DB: StallLogMgr::getStallLogById error:{}", er.what());
    }

    return out;
}

}
