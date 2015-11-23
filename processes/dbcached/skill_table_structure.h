
#ifndef DBCACHED_SKILL_TABLE_STRUCTURE_H
#define DBCACHED_SKILL_TABLE_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_5(skill, 1, 5, 
             mysqlpp::sql_bigint, roleId,
             uint32_t, skillId,
             uint32_t, skillLv,
             uint32_t, strengthenLv,
             uint32_t, exp)
typedef skill RowOfSkill;

sql_create_6(heroSkill, 1, 6, 
             mysqlpp::sql_bigint, roleId,
             uint8_t, job,
             uint32_t, skillId,
             uint32_t, skillLv,
             uint32_t, strengthenLv,
             uint32_t, exp)
typedef heroSkill RowOfHeroSkill;

}


#endif
