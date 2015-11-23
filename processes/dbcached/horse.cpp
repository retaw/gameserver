#include "horse.h"
#include "role_manager.h"
#include "horse_table_structure.h"

#include "water/componet/logger.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/horse.h"
#include "protocol/rawmsg/private/horse.codedef.private.h"


namespace dbcached{

using namespace water::componet;

Horse& Horse::me()
{
    static Horse me;
    return me;
}


void Horse::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(ModifyHorseData, std::bind(&Horse::servermsg_ModifyHorseData, this, _1, _2));
}

void Horse::servermsg_ModifyHorseData(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::ModifyHorseData*>(msgData);
    std::string blobstr("");
    blobstr.append((char*)rev->buf, rev->size);
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfHorse horseRow(rev->roleId, blobstr);
        query.replace(horseRow);
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("Horse:: modifyHorseData 产生异常, {}", er.what());
    }

    RoleManager::me().m_contrRoles.updateHorseData(rev->roleId, blobstr);
}

std::string Horse::load(RoleId roleId)
{
    std::string out("");
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from horse where roleId = " << roleId << " limit 1";
        std::vector<RowOfHorse> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("DB:Horse::load()空,请求roleId={}",roleId);
        }
        else
        {
            out = res[0].blobStr;
        }
    }

    return out;
}

}
