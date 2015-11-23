#ifndef SUPER_ACCOUNT_TABLE_STRUCTURE_H
#define SUPER_ACCOUNT_TABLE_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h"
namespace super{

sql_create_7(account_data, 1, 7,
             uint64_t, id,
             std::string, pwd_hash,
             std::string, nicknam,
             std::string, account,
             std::string, phone,
             uint64_t, recore,
             std::string, email)

typedef account_data RowOfAccount;
}


#endif
