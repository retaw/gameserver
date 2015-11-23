#include "account_manager.h"

#include "super.h"
#include "account_table_structure.h"
#include "zone_manager.h"

#include "water/componet/random.h"
#include "water/componet/logger.h"
#include "water/dbadaptcher/dbconnection_pool.h"

#include "protocol/protobuf/public/account.codedef.h"
#include "protocol/protobuf/private/super.codedef.h"

namespace super{

AccId AccountItem::m_lastId = INVALID_ACC_ID;

bool AccountItem::replace() const
{
    try
    {
        RowOfAccount rowOfAccount(m_id, m_pwdHash, m_nickname, m_account, m_phone, m_recore, m_email);
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query.replace(rowOfAccount);
        if(m_lastId < m_id)//id大于m_lastId，新用户，需同时更新用户和特殊行
        {
            ++m_lastId;

            //事务
            mysqlpp::Query querySpecialRow = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
            querySpecialRow << "update account_data set recore = " << m_id << " where id = " << INVALID_ACC_ID;
            mysqlpp::Transaction trans(
                                       *(dbadaptcher::MysqlConnectionPool::me().getconn()),
                                       mysqlpp::Transaction::serializable,
                                       mysqlpp::Transaction::session);
            query.execute();
            querySpecialRow.execute();
            trans.commit();
        }
        else
        {
            query.execute();
        }
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("账户存储更新, 数据库操作失败, DB error:{}", er.what());
        return false;
    }
}
/*
bool AccountItem::erase() const
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from account where id = " << m_id;
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("账户删除, 数据库操作失败, DB error:{}", er.what());
        return false;
    }
}
*/
bool AccountItem::loadById(const AccId id)
{
    std::vector<RowOfAccount> rowOfAccount;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from account_data where id = " << id;
        query.storein(rowOfAccount);
        //这里账户为key。。。
        if(!rowOfAccount.empty())
        {
            m_id = rowOfAccount[0].id;
            m_pwdHash = rowOfAccount[0].pwd_hash;
            m_nickname = rowOfAccount[0].nicknam;
            m_account = rowOfAccount[0].account;
            m_phone = rowOfAccount[0].phone;
            m_recore = rowOfAccount[0].recore;
            m_email = rowOfAccount[0].email;
            return true;
        }
        return false;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("账户查询, 数据库操作失败, DB error:{}", er.what());
        return false;
    }
}

bool AccountItem::loadByAccount(const std::string& account)
{
    std::vector<RowOfAccount> rowOfAccount;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from account_data where account = " << mysqlpp::quote << account;
        query.storein(rowOfAccount);
        //这里账户为key。。。
        if(!rowOfAccount.empty())
        {
            m_id = rowOfAccount[0].id;
            m_pwdHash = rowOfAccount[0].pwd_hash;
            m_nickname = rowOfAccount[0].nicknam;
            m_account = rowOfAccount[0].account;
            m_phone = rowOfAccount[0].phone;
            m_recore = rowOfAccount[0].recore;
            m_email = rowOfAccount[0].email;
            return true;
        }
        return false;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("账户查询, 数据库操作失败, DB error:{}", er.what());
        return false;
    }
}

bool AccountItem::loadByEmail(const std::string& email)
{
    std::vector<RowOfAccount> rowOfAccount;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from account_data where id = " << mysqlpp::quote << email;
        query.storein(rowOfAccount);
        //这里账户为key。。。
        if(!rowOfAccount.empty())
        {
            m_id = rowOfAccount[0].id;
            m_pwdHash = rowOfAccount[0].pwd_hash;
            m_nickname = rowOfAccount[0].nicknam;
            m_account = rowOfAccount[0].account;
            m_phone = rowOfAccount[0].phone;
            m_recore = rowOfAccount[0].recore;
            m_email = rowOfAccount[0].email;
            return true;
        }
        return false;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("账户查询, 数据库操作失败, DB error:{}", er.what());
        return false;
    }
}


bool AccountItem::loadByPhone(const std::string& phone)
{
    std::vector<RowOfAccount> rowOfAccount;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from account_data where id = " << mysqlpp::quote << phone;
        query.storein(rowOfAccount);
        //这里账户为key。。。
        if(!rowOfAccount.empty())
        {
            m_id = rowOfAccount[0].id;
            m_pwdHash = rowOfAccount[0].pwd_hash;
            m_nickname = rowOfAccount[0].nicknam;
            m_account = rowOfAccount[0].account;
            m_phone = rowOfAccount[0].phone;
            m_recore = rowOfAccount[0].recore;
            m_email = rowOfAccount[0].email;
            return true;
        }
        return false;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("账户查询, 数据库操作失败, DB error:{}", er.what());
        return false;
    }
}

AccId AccountItem::getLastId()
{
    if(m_lastId != INVALID_ACC_ID)
        return m_lastId;

    //到这里说明m_lastId无效即首次调用，需从库内读取
    //数据库内特殊行(id=0)记录最大id，使用字段recore记录最大id
    std::vector<RowOfAccount> rowOfAccount;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select recore from account_data where id = 0";
        query.storein(rowOfAccount);
        if(!rowOfAccount.empty())
            return rowOfAccount[0].recore;
        else
        {
            LOG_ERROR("得到最大用户id, 数据库特殊行异常, id无效");
            return INVALID_ACC_ID;
        }
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("得到最大用户id, 数据库查询失败, DB error:{}, er.what()");
        return INVALID_ACC_ID;
    }
}

/********************/

AccountManager& AccountManager::me()
{
    static AccountManager me;
    return me;
}

void AccountManager::timerExec(const water::componet::TimePoint& now)
{
}

/*
uint64_t AccountManager::newToken()
{
    static water::componet::Random<uint64_t> random(0, 0xffffffffffffffff);
    return random.get();
}*/

AccountItem::Ptr AccountManager::getByAccId(AccId accId)
{
    auto it = m_accountCache.find(accId);
    if(it != m_accountCache.end())
        return it->second;

    //从数据库中检查accId是否存在, 如果存在, 加入缓存
    AccountItem::Ptr item = AccountItem::create();
    if(!item->loadById(accId))
        return nullptr;

    m_accountCache.insert({item->m_id, item});
    m_accId2accountMap.insert({item->m_account, item->m_id});
    return item;
}

AccountItem::Ptr AccountManager::getByAccount(const std::string& account)
{
    //先从缓存中查找
    auto it = m_accId2accountMap.find(account);
    if(it == m_accId2accountMap.end())
    {
        //从数据库中检查account是否存在, 如果存在, 加入缓存
        //从数据库中检查account是否存在, 如果存在, 加入缓存
        AccountItem::Ptr item = AccountItem::create();
        if(!item->loadByAccount(account))
            return nullptr;

        m_accountCache.insert({item->m_id, item});
        m_accId2accountMap.insert({item->m_account, item->m_id});
        return item;
    }


    return getByAccId(it->second);
}

AccountItem::Ptr AccountManager::getByLoginId(LoginId loginId)
{
    AccId accId = INVALID_ACC_ID;
    {
        LockGuard lock(m_verifiedClients.lock);
        auto it = m_verifiedClients.clients.find(loginId);
        if(it == m_verifiedClients.clients.end())
            return nullptr;

        accId = it->second;
    }

    if(accId == INVALID_ACC_ID)
        return nullptr;

    return getByAccId(accId);
}

void AccountManager::clientmsg_C_FastSignUp(const ProtoMsgPtr& proto, LoginId loginId)
{
    auto rev = std::static_pointer_cast<PublicProto::C_FastSignUp>(proto);

    const std::string device = rev->device();

    PublicProto::S_SignUpRet retMsg;
    retMsg.set_acc_id(0);

    AccountItem::Ptr item = AccountItem::create();
    item->m_device  = rev->device();

    if(saveNewAccount(item))
    {
        retMsg.set_acc_id(item->m_id);

        //保存成功, 即登录成功, 加入loginId 和 accId 的索引
        LockGuard lock(m_verifiedClients.lock);
        m_verifiedClients.clients[loginId] = item->m_id;
    }
    Super::me().sendToClient(loginId, PROTO_CODE_PUBLIC(S_SignUpRet), retMsg);
    return;
}

void AccountManager::clientmsg_C_SignUpWithAccount(const ProtoMsgPtr& proto, LoginId loginId)
{
    auto rev = std::static_pointer_cast<PublicProto::C_SignUpWithAccount>(proto);
    std::string account = rev->account();

    PublicProto::S_SignUpRet retMsg;
    retMsg.set_acc_id(0);

    if(getByAccount(rev->account()) != nullptr)
    {
        Super::me().sendToClient(loginId, PROTO_CODE_PUBLIC(S_SignUpRet), retMsg);
        return;
    }

    AccountItem::Ptr item = AccountItem::create();
    item->m_account = account;
    item->m_pwdHash = rev->password();

    if(saveNewAccount(item))
    {
        retMsg.set_acc_id(item->m_id);

        //保存成功, 即登录成功, 加入loginId 和 accId 的索引
        LockGuard lock(m_verifiedClients.lock);
        m_verifiedClients.clients[loginId] = item->m_id;
    }
    Super::me().sendToClient(loginId, PROTO_CODE_PUBLIC(S_SignUpRet), retMsg);
    return;
}

bool AccountManager::saveNewAccount(AccountItem::Ptr item)
{
    //1 把item插入数据库
    item->m_id = AccountItem::getLastId() + 1;
    if(!item->replace())
        return false;

    //2 插入缓存
    m_accountCache.insert({item->m_id, item});
    if(item->m_account != "")
        m_accId2accountMap.insert({item->m_account, item->m_id});

    return true;
}

void AccountManager::clientmsg_C_FastLogin(const ProtoMsgPtr& proto, LoginId loginId)
{
    auto rev = std::static_pointer_cast<PublicProto::C_FastLogin>(proto);

    auto accId = rev->acc_id();
    auto device = rev->device();

    PublicProto::S_LoginRet retMsg;

    auto item = getByAccId(accId);
    if(item == nullptr || item->m_device != device)
    {
        retMsg.set_acc_id(0);
        Super::me().sendToClient(loginId, PROTO_CODE_PUBLIC(S_LoginRet), retMsg);
        return;
    }

    {//登录成功, 加入loginId 和 accId 的索引
        LockGuard lock(m_verifiedClients.lock);
        m_verifiedClients.clients[loginId] = item->m_id;
    }

    retMsg.set_acc_id(item->m_id);
    Super::me().sendToClient(loginId, PROTO_CODE_PUBLIC(S_LoginRet), retMsg);
    return;
}

void AccountManager::clientmsg_C_LoginByAccount(const ProtoMsgPtr& proto, LoginId loginId)
{
    auto rev = std::static_pointer_cast<PublicProto::C_LoginByAccount>(proto);
    std::string account = rev->account();
    std::string passwd  = rev->password();

    PublicProto::S_LoginRet retMsg;

    auto item = getByAccount(account);
    if(item == nullptr || item->m_pwdHash != rev->password())
    {
        retMsg.set_acc_id(0);
        Super::me().sendToClient(loginId, PROTO_CODE_PUBLIC(S_LoginRet), retMsg);
        return;
    }

    {//登录成功, 加入loginId 和 accId 的索引
        LockGuard lock(m_verifiedClients.lock);
        m_verifiedClients.clients[loginId] = item->m_id;
    }

    retMsg.set_acc_id(item->m_id);
    Super::me().sendToClient(loginId, PROTO_CODE_PUBLIC(S_LoginRet), retMsg);
    return;
}

void AccountManager::clientmsg_C_SelectPlatform(const ProtoMsgPtr& proto, LoginId loginId)
{
    AccountItem::Ptr item = getByLoginId(loginId);
    if(item == nullptr)
        return;

    auto rev = std::static_pointer_cast<PublicProto::C_SelectPlatform>(proto);
    Platform platform = static_cast<Platform>(rev->platform());
    auto zoneId = ZoneManager::me().selectZone(platform); //会发消息到zone, 带gateway注册后返回消息
    if(zoneId == water::process::INVALID_ZONE_ID)
    {
        LOG_TRACE("登录, 选区, 选择网关失败, accont={}, platform={}", item->m_account, platform);
        return;
    }

    PrivateProto::RegVerifiedAccountToGateway msg;
    msg.set_acc_id(item->m_id);
    msg.set_login_id_super(loginId);

    ProcessIdentity zoneGatewayId(zoneId, "gateway", 1);
    Super::me().sendToPrivate(zoneGatewayId, PROTO_CODE_PRIVATE(RegVerifiedAccountToGateway), msg);
}

void AccountManager::servermsg_ZoneGatewayReady(const ProtoMsgPtr& proto, ProcessIdentity pid)
{
    auto rev = std::static_pointer_cast<PrivateProto::ZoneGatewayReady>(proto);
    LoginId loginId = rev->login_id_super();

    auto item = getByLoginId(loginId);
    if(item == nullptr)
    {
        LOG_TRACE("登录, 网关已准备好, 用户已放弃登录, accId={}", rev->acc_id());
        return;
    }

    Endpoint ep = ZoneManager::me().getGatewayEndpintByZoneId(pid.zoneId());
    if(ep.port == 0)
    {
        LOG_ERROR("登录, 网关已准备好, 获取endpoint失败, zoneId={}", pid.zoneId());
        return;
    }

    PublicProto::S_SelectPlatformRet msg;
    msg.set_ip(ep.ip.value);
    msg.set_port(ep.port);
    msg.set_token(rev->token());

    Super::me().sendToClient(loginId, PROTO_CODE_PUBLIC(S_SelectPlatformRet), msg);
}

void AccountManager::regMsgHandler()
{
    using namespace std::placeholders;

    REG_PROTO_PUBLIC(C_FastSignUp, std::bind(&AccountManager::clientmsg_C_FastSignUp, this, _1, _2));
    REG_PROTO_PUBLIC(C_SignUpWithAccount, std::bind(&AccountManager::clientmsg_C_SignUpWithAccount, this, _1, _2));
    REG_PROTO_PUBLIC(C_LoginByAccount, std::bind(&AccountManager::clientmsg_C_LoginByAccount, this, _1, _2));
    REG_PROTO_PUBLIC(C_FastLogin, std::bind(&AccountManager::clientmsg_C_FastLogin, this, _1, _2));
    REG_PROTO_PUBLIC(C_SelectPlatform, std::bind(&AccountManager::clientmsg_C_SelectPlatform, this, _1, _2));


    REG_PROTO_PRIVATE(ZoneGatewayReady, std::bind(&AccountManager::servermsg_ZoneGatewayReady, this, _1, _2));
}

void AccountManager::onClientDisconect(LoginId loginId)
{
    LockGuard lock(m_verifiedClients.lock);
    m_verifiedClients.clients.erase(loginId);
}


}
