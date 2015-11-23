#ifndef DBCACHED_OBJECT_TABLE_STRUCTURE_H
#define DBCACHED_OBJECT_TABLE_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_11(object, 1, 11, 
             mysqlpp::sql_bigint, objId,
             mysqlpp::sql_bigint, roleId,
             uint8_t, packageType,
             mysqlpp::sql_bigint, tplId,
             uint16_t, item,
             uint16_t, cell,
             uint32_t, skillId,
             uint8_t, bind,
             uint32_t, sellTime,
             uint8_t, strongLevel,
			 uint8_t, luckyLevel)
typedef object RowOfObject;

}


#endif
