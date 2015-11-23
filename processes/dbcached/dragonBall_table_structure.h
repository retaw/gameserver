/*
 * Author: zhupengfei
 *
 * Created: 2015-08-26 10:00 +0800
 *
 * Modified: 2015-08-26 10:00 +0800
 *
 * Description: 龙珠
 */

#ifndef DBCACHE_DRAGON_BALL_STRUCTURE_HPP
#define DBCACHE_DRAGONBALL_STRUCTURE_HPP

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_3(dragonBall, 1, 3, 
             mysqlpp::sql_bigint, roleId,
             uint8_t, dragonType,
             uint32_t, exp)

typedef dragonBall RowOfDragonBall;

}


#endif
