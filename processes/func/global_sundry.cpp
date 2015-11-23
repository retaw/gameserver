#include "global_sundry.h"
#include "global_sundry_table_structure.h"
#include "func.h"
#include "shabake.h"
#include "water/componet/logger.h"
#include "water/componet/serialize.h"

namespace func{

using namespace water::componet;

GlobalSundry::GlobalSundry()
{
}

GlobalSundry& GlobalSundry::me()
{
    static GlobalSundry me;
    return me;
}

void GlobalSundry::regTimer()
{
    using namespace std::placeholders;
    Func::me().regTimer(std::chrono::minutes(5),
                        std::bind(&GlobalSundry::timerLoop, this));
}

void GlobalSundry::timerLoop()
{
    saveDB();
}

/*
 * 序列化数据
 */
void GlobalSundry::serializeData(std::string& saveStr)
{
    Serialize<std::string> iss(&saveStr);
    iss.reset();
    iss << m_worldBossLv;
    iss << m_recordStoreObjForever;
    iss << m_recordStoreObjDay;
    ShaBaKe::me().serializeData(iss);

    LOG_DEBUG("GlobalSundry: saveDB, m_worldBossLv:{}", m_worldBossLv);
}


/*
 * 反序列化
 */
void GlobalSundry::deserializeData(const std::string& loadStr)
{
    Deserialize<std::string> oss(&loadStr);
    oss >> m_worldBossLv;
    oss >> m_recordStoreObjForever;
    oss >> m_recordStoreObjDay;
    ShaBaKe::me().deserializeData(oss);

    LOG_DEBUG("GlobalSundry: loadDB, m_worldBossLv:{}", m_worldBossLv);
}


void GlobalSundry::saveDB()
{
    std::string saveStr("");
    serializeData(saveStr);

    try
    {
        mysqlpp::Query query = water::dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfGlobalSundry sundryRow(1, saveStr);
        query.replace(sundryRow);
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_DEBUG("FUNC:GlobalSundry::saveDB error:{}",er.what());
    }
}


void GlobalSundry::loadDB()
{
    try
    {
        mysqlpp::Query query = water::dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query <<"select * from globalSundry";
        std::vector<RowOfGlobalSundry> rowGlobalSundry;
        query.storein(rowGlobalSundry);
        if(rowGlobalSundry.size() > 0)
        {
            std::string ss = rowGlobalSundry[0].blobStr;
            deserializeData(ss);
        }
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_DEBUG("FUNC:GlobalSundry::loadDB error:{}",er.what());
    }
}

}
