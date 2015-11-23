
#ifndef DBCACHED_COUNTER_STRUCTURE_H
#define DBCACHED_COUNTER_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_2(roleCounter, 1, 2, 
             mysqlpp::sql_bigint, roleId,
             std::string, counterStr)
typedef roleCounter RowOfRoleCounter;

}


#endif
