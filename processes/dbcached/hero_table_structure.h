#ifndef DBCACHED_HERO_TABLE_STRUCTURE_H
#define DBCACHED_HERO_TABLE_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h"
namespace dbcached{
sql_create_9(hero, 1, 9,
             uint8_t, job,
             mysqlpp::sql_bigint, roleId,
             uint8_t, sex,
             uint32_t, level,
             uint64_t, exp,
             uint32_t, hp,
             uint32_t, mp,
             uint8_t, turnLife,
             uint32_t, clother)

typedef hero RowOfHero;

}

#endif
