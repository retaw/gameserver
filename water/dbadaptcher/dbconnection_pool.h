/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-11 10:44 +0800
 *
 * Modified: 2015-03-11 10:44 +0800
 *
 * Description: mysql++简易连接池，可确保每个线程对应一个connection
 */

#ifndef DBADAPTCHER_DBCONNECTION_POOL_H
#define DBADAPTCHER_DBCONNECTION_POOL_H

#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "componet/exception.h"
#include <string>

namespace water{
namespace dbadaptcher{

DEFINE_EXCEPTION(LoadProcessConfigFailed, componet::ExceptionBase)
DEFINE_EXCEPTION(MysqlCfgNotExisit, componet::ExceptionBase)
class MysqlConnectionPool
{
public:
    static MysqlConnectionPool& me();

    MysqlConnectionPool(const MysqlConnectionPool &) = delete;
    MysqlConnectionPool&  operator = (const  MysqlConnectionPool&) = delete;
    void init(const std::string& configDir);//读出数据库配置,检查库是否正常,配置或者库异常都throw出来
    std::shared_ptr<mysqlpp::Connection> getconn();
private:
    MysqlConnectionPool() = default;
    void initmysqlconfig(const std::string& configDir);
    void check();

private:
    std::vector<std::string> dbTable;
    std::string dbName;
    std::string host = "127.0.0.1";
    std::string userName;
    std::string pwd;
    int port = 3306;
    static thread_local std::shared_ptr<mysqlpp::Connection> m_conn; 
};

/* //使用conneciton pool的版本
class MysqlConnectionPool : public mysqlpp::ConnectionPool
{
public:
    MysqlConnectionPool(const std::string& host, int32_t port, 
                        const std::string& userName, const std::string pwd,
                        const std::string& dbName);

    ~MysqlConnectionPool();

private:
    mysqlpp::Connection* create() override;

    void destroy (mysqlpp::Connection *conn) override;

    unsigned int max_idle_time() override;
};
*/



}}

#endif
