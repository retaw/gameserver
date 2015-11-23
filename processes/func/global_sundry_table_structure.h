#ifndef FUNC_GLOBAL_SUNDRY_TABLE_H
#define FUNC_GLOBAL_SUNDRY_TABLE_H


#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif
#include "water/dbadaptcher/dbconnection_pool.h"

namespace func{
sql_create_2(globalSundry, 1, 2,
            uint16_t, id,
            std::string, blobStr)

typedef globalSundry RowOfGlobalSundry;

}
#endif
