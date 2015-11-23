#ifndef DBCACHED_DBSTORE_H
#define DBCACHED_DBSTORE_H


#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif
#include "water/dbadaptcher/dbconnection_pool.h"
#include "water/componet/datetime.h"

namespace dbcached{
sql_create_12(roleRarelyUp, 1, 12,
            mysqlpp::sql_bigint, id,
            mysqlpp::sql_char, name,
            uint8_t, turnLife,
            mysqlpp::sql_char, account,
            uint8_t, sex,
            uint8_t, job,
            uint16_t, unlockCellNumOfRole,
            uint16_t, unlockCellNumOfHero,
            uint16_t, unlockCellNumOfStorage,
            uint8_t, defaultCallHero,
			uint8_t, guanzhiLevel,
			std::string, buffer)

typedef roleRarelyUp RowOfRoleRarelyUp;

sql_create_13(roleOftenUp, 1, 13,
            mysqlpp::sql_bigint, id,
            mysqlpp::sql_int, level,
            mysqlpp::sql_bigint, exp,
            uint64_t, money_1,
            uint64_t, money_2,
            uint64_t, money_3,
            uint64_t, money_4,
            uint64_t, money_5,
            uint64_t, money_6,
			uint64_t, money_7,
			uint64_t, money_8,
			uint64_t, money_9,
			uint64_t, money_10)

typedef roleOftenUp RowOfRoleOftenUp;

sql_create_14(roleOfflnUp, 1, 14,
            mysqlpp::sql_bigint, id,
            mysqlpp::sql_int, mp,
            mysqlpp::sql_int, hp,
            uint8_t, dir,
            uint64_t, sceneId,
            uint32_t, pos,
            uint64_t, preSceneId,
            uint32_t, prePos,
            bool, dead,
            uint32_t, deathTime,
            uint32_t, totalOnlineSec,
            uint32_t, offlnTime,
            uint16_t, evilVal,
            uint8_t, attackMode)

typedef roleOfflnUp RowOfRoleOfflnUp;


}
#endif
