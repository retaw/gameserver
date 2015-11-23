#ifndef DBCACHED_FRIEND_TABLE_STRUCTURE_H
#define DBCACHED_FRIEND_TABLE_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h"
namespace func{
    
sql_create_2(friends, 1, 2,
             mysqlpp::sql_bigint, roleId,
             std::string, friendStr)
typedef friends RowOfFriend;
    
sql_create_2(blacklist, 1, 2,
             mysqlpp::sql_bigint, roleId,
             mysqlpp::sql_bigint, blackId)
typedef blacklist RowOfBlacklist;

sql_create_2(enemy, 1, 2,
             mysqlpp::sql_bigint, roleId,
             std::string, enemyId)
typedef enemy RowOfEnemy;

}

#endif
