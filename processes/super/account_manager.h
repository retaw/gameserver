/*
 * Author: LiZhaojia
 *
 * Created: 2015-08-17 11:16 +0800
 *
 * Modified: 2015-08-17 11:16 +0800
 *
 * Description: 登录管理器
 */



#include "water/common/commdef.h"
#include "water/common/roledef.h"

#include "water/componet/class_helper.h"
#include "water/componet/spinlock.h"
#include "water/componet/limited_size_unordered_map_cache.h"

#include "water/process/tcp_message.h"

#include "protocol/protobuf/proto_manager.h"

#include <atomic>
#include <list>
#include <mutex>

namespace super{

using namespace water;

class AccountItem
{
public:
    CREATE_FUN_MAKE(AccountItem)
    TYPEDEF_PTR(AccountItem)

public:
    /**操作数据库对应的表**/
    bool replace() const; //不存在插入，存在更新，如果id大于特殊行的最大id则更新特殊行
    //bool erase() const;
    bool loadById(const AccId id);
    bool loadByAccount(const std::string& account);
    bool loadByPhone(const std::string& phone);
    bool loadByEmail(const std::string& email);

    static AccId getLastId();//失败返回0

//data:
    LoginId loginId = 0;
    bool isConfirmed = false;

    AccId m_id = 0;

    std::string m_pwdHash;
    std::string m_device;

    std::string m_nickname;
    std::string m_account;
    std::string m_phone;
    uint64_t m_recore = 0;
    std::string m_email;
    static  AccId m_lastId;
};

class LoginAccount
{
public:
private:
};

using water::process::ProcessIdentity;
using water::process::TcpMsgCode;
using protocol::protobuf::ProtoMsgPtr;

class AccountManager
{
public:
    ~AccountManager() = default;

    void loadConfig();

    void regMsgHandler();

    void onClientDisconect(LoginId loginId);

private:
    AccountManager() = default;

    void timerExec(const water::componet::TimePoint& now);

    uint64_t newToken();

    AccountItem::Ptr getByAccId(AccId accId);
    AccountItem::Ptr getByAccount(const std::string& account);
    AccountItem::Ptr getByLoginId(LoginId loginId);

    //处理登录消息
    void clientmsg_C_FastSignUp(const ProtoMsgPtr& proto, LoginId loginId);
    void clientmsg_C_SignUpWithAccount(const ProtoMsgPtr& proto, LoginId loginId);
    void clientmsg_C_LoginByAccount(const ProtoMsgPtr& proto, LoginId loginId);
    void clientmsg_C_FastLogin(const ProtoMsgPtr& proto, LoginId loginId);
    void clientmsg_C_SelectPlatform(const ProtoMsgPtr& proto, LoginId loginId);

    void servermsg_ZoneGatewayReady(const ProtoMsgPtr& proto, ProcessIdentity pid);

    bool saveNewAccount(AccountItem::Ptr);

private:
    typedef std::lock_guard<water::componet::Spinlock> LockGuard;

    componet::LimitedSizeUnorderedMapCache<AccId, AccountItem::Ptr, 100000> m_accountCache; //数据库的内存缓冲
    componet::LimitedSizeUnorderedMapCache<std::string, AccId, 100000> m_accId2accountMap;  //<account, accId>
    struct VerifiedClients
    {
        mutable water::componet::Spinlock lock;
        std::unordered_map<LoginId, AccId> clients;      // <loginId, account>
    } m_verifiedClients;

public:
    static AccountManager& me();
};


}

