#ifndef DBCACHE_TASK_STRUCTURE_H
#define DBCACHE_TASK_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_2(task, 1, 2, 
             mysqlpp::sql_bigint, roleId,
             std::string, blobStr)

typedef task RowOfTask;

sql_create_2(factionTask, 1, 2,
             mysqlpp::sql_bigint, roleId,
             std::string, data)

typedef factionTask RowOfFactionTask;

}


#endif
