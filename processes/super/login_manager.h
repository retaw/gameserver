/*
 * Author: LiZhaojia
 *
 * Created: 2015-08-17 11:16 +0800
 *
 * Modified: 2015-08-17 11:16 +0800
 *
 * Description: 登录管理器
 */


namespace super{

#include "water/common/commdef.h"
#include "water/process/tcp_message.h"
#include "protocol/protobuf/proto_manager.h"

#include <atomic>
#include <list>
#include <mutex>




class LoginManager
{
public:
    ~LoginManager() = default;

    void loadConfig();

//    void clientConnReady(LoginId loginId);

    void regMsgHandler();

private:
    LoginManager() = default;

 //   void erase(LoginId loginId);

    //处理登录消息
    void clientmsg_UserSignUp(const ProtoMsgPtr& proto, const ProtoMsgPtr& proto, LoginId loginId);
    void clientmsg_UserLogin(const ProtoMsgPtr& proto, const ProtoMsgPtr& proto, LoginId loginId);

    //处理内部的消息
//    void servermsg_(const ProtoMsgPtr& proto);

private:
//    typedef std::lock_guard<water::componet::Spinlock> LockGuard;
//    mutable water::componet::Spinlock m_usersLock;

    std::unordered_map<std::string, std::string> m_accountAndToken; //<account, token>


public:
    static LoginManager& me();
};


}

