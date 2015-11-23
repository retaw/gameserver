#include "super.h"

#include "water/dbadaptcher/dbconnection_pool.h"


namespace super{

void Super::loadDB()
{
    dbadaptcher::MysqlConnectionPool::me().init(m_cfgDir + "/process.xml");
}

}
