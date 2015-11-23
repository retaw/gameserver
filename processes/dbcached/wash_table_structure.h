/*
 * Author: zhupengfei
 *
 * Created: 2015-08-07 16:20 +0800
 *
 * Modified: 2015-08-07 16:20 +0800
 *
 * Description: 洗练
 */

#ifndef DBCACHE_WASH_STRUCTURE_HPP
#define DBCACHE_WASH_STRUCTURE_HPP

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_4(wash, 1, 4, 
             mysqlpp::sql_bigint, roleId,
             uint8_t, sceneItem,
             uint8_t, washType,
             std::string, propStr)

typedef wash RowOfWash;

}


#endif
