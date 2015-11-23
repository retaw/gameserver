
#ifndef DBCACHED_STALL_LOG_TABLE_STRUCTURE_H
#define DBCACHED_STALL_LOG_TABLE_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_2(stallSellLog, 1, 2, 
             mysqlpp::sql_bigint, roleId,
             std::string, blobStr)
typedef stallSellLog RowOfStallLog;

}


#endif
