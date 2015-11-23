#ifndef FUNC_DBSTORE_H
#define FUNC_DBSTORE_H


#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif
#include "water/dbadaptcher/dbconnection_pool.h"
#include "water/componet/datetime.h"

namespace func{
sql_create_12(roleRarelyUp, 1, 12,
            mysqlpp::sql_bigint, id,
            mysqlpp::sql_char, name,
            uint8_t, turnLife,
            mysqlpp::sql_char, account,
            uint8_t, sex,
            uint8_t, job,
            mysqlpp::sql_bigint, curObjId,
            uint16_t, unlockCellNumOfRole,
            uint16_t, unlockCellNumOfHero,
            uint16_t, unlockCellNumOfStorage,
            uint8_t, defaultCallHero,
			uint8_t, guanzhiLevel)

typedef roleRarelyUp RowOfRoleRarelyUp;

sql_create_10(roleOftenUp, 1, 10,
            mysqlpp::sql_bigint, id,
            mysqlpp::sql_int, level,
            mysqlpp::sql_bigint, exp,
            uint64_t, money_1,
            uint64_t, money_2,
            uint64_t, money_3,
            uint64_t, money_4,
            uint64_t, money_5,
            uint64_t, money_6,
			uint64_t, money_7)

typedef roleOftenUp RowOfRoleOftenUp;

sql_create_13(roleOfflnUp, 1, 13,
            mysqlpp::sql_bigint, id,
            mysqlpp::sql_int, mp,
            mysqlpp::sql_int, hp,
            uint8_t, dir,
            uint64_t, sceneId,
            uint16_t, posX,
            uint16_t, posY,
            bool, dead,
            uint32_t, deathTime,
            uint32_t, totalOnlineSec,
            uint32_t, offlnTime,
            uint16_t, evilVal,
            uint8_t, attackMode)

typedef roleOfflnUp RowOfRoleOfflnUp;

sql_create_5(roleInfo, 1, 5,
             mysqlpp::sql_bigint, id,
             mysqlpp::sql_char, name,
             mysqlpp::sql_int, level,
             uint8_t, job,
             uint8_t, sex)

typedef roleInfo RowOfRoleInfo;

}
#endif
