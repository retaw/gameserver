
#ifndef DBCACHED_BUFF_TABLE_STRUCTURE_H
#define DBCACHED_BUFF_TABLE_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_5(buff, 1, 5, 
             mysqlpp::sql_bigint, roleId,
             uint32_t, buffId,
             uint32_t, sec,
             uint32_t, endtime,
             uint32_t, dur)
typedef buff RowOfBuff;

sql_create_6(heroBuff, 1, 6, 
             mysqlpp::sql_bigint, roleId,
             uint8_t, job,
             uint32_t, buffId,
             uint32_t, sec,
             uint32_t, endtime,
             uint32_t, dur)
typedef heroBuff RowOfHeroBuff;
}


#endif
