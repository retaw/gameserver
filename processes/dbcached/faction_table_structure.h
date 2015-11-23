#ifndef DBCACHED_FACTION_TABLE_STRUCTURE_H
#define DBCACHED_FACTION_TABLE_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h"
namespace dbcached{

sql_create_11(faction, 1, 11,
              uint64_t, factionId,
              mysqlpp::sql_char, name,
              uint32_t, level,
              uint64_t, exp,
              uint64_t, resource,
              uint64_t, leader,
              std::string, viceLeaders,
              uint64_t, warriorLeader,
              uint64_t, magicianLeader,
              uint64_t, taoistLeader,
              std::string, notice)

typedef faction RowOfFaction;

sql_create_3(roleInFaction, 1, 3,
             uint64_t, id,
             uint64_t, factionId,
             uint64_t, banggong)

typedef roleInFaction RowOfRoleInFaction;

}


#endif
