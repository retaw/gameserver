
#ifndef DBCACHED_SUNDRY_STRUCTURE_H
#define DBCACHED_SUNDRY_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_2(sundry, 1, 2, 
             mysqlpp::sql_bigint, roleId,
             std::string, data)
typedef sundry RowOfSundry;


sql_create_2(timerSundry, 1, 2, 
             mysqlpp::sql_bigint, roleId,
             std::string, data)
typedef timerSundry RowOfTimerSundry;

}


#endif
