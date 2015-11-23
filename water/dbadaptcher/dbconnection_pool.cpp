#include"dbconnection_pool.h"

#include <thread>
#include "componet/logger.h"
#include "componet/xmlparse.h"
namespace water{
namespace dbadaptcher{


thread_local std::shared_ptr<mysqlpp::Connection> MysqlConnectionPool::m_conn = std::make_shared<mysqlpp::Connection>(); 

MysqlConnectionPool& MysqlConnectionPool::me()
{
    static MysqlConnectionPool me;
    return me;
}

void MysqlConnectionPool::init(const std::string& configDir)
{
    initmysqlconfig(configDir);
    check();
    
}

std::shared_ptr<mysqlpp::Connection> MysqlConnectionPool::getconn()
{
    if(!m_conn->connected())
    {
        LOG_DEBUG("DB: 开始生成本线程mysql连接");
        m_conn->set_option(new mysqlpp::ReconnectOption(true)); //自动重连
        m_conn->set_option(new mysqlpp::MultiStatementsOption(true));  //支持多语句查询
        m_conn->set_option(new mysqlpp::SetCharsetNameOption("utf8"));//字符集设置
        m_conn->connect(dbName.c_str(),host.c_str(),userName.c_str(),pwd.c_str(),port);
        //m_conn->connect("game","localhost","test","test",3306);
        LOG_DEBUG("DB: 生成本线程mysql连接成功");
    }
    return m_conn;
}

void MysqlConnectionPool::initmysqlconfig(const std::string& configFile)
{
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    LOG_DEBUG("开始读取数据库配置 {}",configFile);

    XmlParseDoc doc(configFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(LoadProcessConfigFailed, configFile + " parse error");
    XmlParseNode mysqlNode = root.getChild("mysql");
    if(!mysqlNode)
        EXCEPTION(MysqlCfgNotExisit, "mysql node not exisit");
    dbName = mysqlNode.getAttr<std::string>("dbName");
    host =  mysqlNode.getAttr<std::string>("host");
    userName = mysqlNode.getAttr<std::string>("userName");
    pwd = mysqlNode.getAttr<std::string>("pwd");
    port = mysqlNode.getAttr<int>("port");
}

void MysqlConnectionPool::check()
{
    mysqlpp::Query query = MysqlConnectionPool::me().getconn()->query();
    //这里数据库应有的表检查，其中表名写死还是配置读取，总之这里从一个vector读取
    for(auto it = dbTable.begin(); it != dbTable.end(); it++)
    {
        query.reset();
        std::string sql = "select TABLE_NAME from INFORMATION_SCHEMA.TABLES where TABLE_NAME =";
        query<< sql << mysqlpp::quote << *it;
        mysqlpp::StoreQueryResult res = query.store();
        if(!res)
        {
            //throw biao bu cun zai
        }
    }
    LOG_DEBUG("DB: 数据库检查通过");
}
/*
MysqlConnectionPool::MysqlConnectionPool(const std::string& host, int32_t port, 
                                         const std::string& userName, const std::string pwd,
                                         const std::string& dbName)
: m_usedCounter(0)
  , m_host(host), m_port(port)
  , m_userName(userName), m_pwd(pwd)
  , m_dbName(dbName)
{
}

MysqlConnectionPool::~MysqlConnectionPool()
{
    clear();
}

mysqlpp::Connection* MysqlConnectionPool::create() override
{
    auto conn = new mysqlpp::Connection();
    conn->set_option(new mysqlpp::ReconnectOption()); //自动重连
    conn->set_option(new MultiStatementsOption(true));  //支持多语句查询
    conn->connect(m_dbName.c_str(),
                  m_host.c_str(),
                  m_userName.c_str(),
                  m_pwd.c_str(),
                  m_port);
}

void MysqlConnectionPool::destroy(mysqlpp::Connection *conn) override
{
    delete conn;
}

unsigned int MysqlConnectionPool::max_idle_time() override
{
    return 3;
}
*/

}}
