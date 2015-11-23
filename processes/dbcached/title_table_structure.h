/*
 * Author: zhupengfei
 *
 * Created: 2015-07-27 14:53 +0800
 *
 * Modified: 2015-07-27 14:53 +0800
 *
 * Description: 称号
 */

#ifndef DBCACHE_TITLE_STRUCTURE_HPP
#define DBCACHE_TITLE_STRUCTURE_HPP

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_6(title, 1, 6, 
             mysqlpp::sql_bigint, roleId,
             uint32_t, titleId,
             uint8_t, titleType,
             uint32_t, createTime,
             uint32_t, disableTime,
             bool, used)

typedef title RowOfTitle;

}


#endif
