#ifndef FUNC_FIRST_TABLE_H
#define FUNC_FIRST_TABLE_H


#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif
#include "water/dbadaptcher/dbconnection_pool.h"

namespace func{
sql_create_4(first, 1, 4,
            mysqlpp::sql_bigint, roleId,
            std::string, name,
			uint8_t, job,
			uint8_t, sex)

typedef first RowOfFirst;

}
#endif
