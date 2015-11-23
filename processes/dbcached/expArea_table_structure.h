/*
 * Author: zhupengfei
 *
 * Created: 2015-09-16 15:30 +0800
 *
 * Modified: 2015-09-16 15:30 +0800
 *
 * Description: 经验区
 */

#ifndef DBCACHE_EXP_AREA_STRUCTURE_HPP
#define DBCACHE_EXP_AREA_STRUCTURE_HPP

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_3(expArea, 1, 3, 
             mysqlpp::sql_bigint, roleId,
             uint8_t, expType,
             uint32_t, sec)

typedef expArea RowOfExpArea;

}


#endif
