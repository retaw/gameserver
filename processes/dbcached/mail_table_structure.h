#ifndef DBCACHE_MAIL_STRUCTURE_H
#define DBCACHE_MAIL_STRUCTURE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
    #define MYSQLPP_SSQLS_NO_STATICS
#endif

#include "water/dbadaptcher/dbconnection_pool.h" 

namespace dbcached{

sql_create_7(mail, 1, 7, 
             mysqlpp::sql_bigint, roleId,
             uint32_t, mailIndex,
             std::string, title,
             std::string, text,
             uint8_t, state,
             uint32_t, time,
             std::string, obj)

typedef mail RowOfMail;

}


#endif
