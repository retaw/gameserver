/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-14 16:04 +0800
 *
 * Modified: 2015-03-14 16:04 +0800
 *
 * Description:  客户端登录处理, 处理登录流程
 */

#ifndef PROCESS_GATEWAY_LOGIN_PROCESSER_H
#define PROCESS_GATEWAY_LOGIN_PROCESSER_H

#include "def.h"

#include "water/componet/spinlock.h"
#include "water/componet/datetime.h"
#include "water/componet/event.h"
#include "water/process/process_thread.h"
#include "water/process/connection_checker.h"

#include <list>

namespace gateway{


using namespace water;

class LoginProcessor
{
    struct ClientInfo
    {
        TYPEDEF_PTR(ClientInfo)

        LoginId loginId;
        std::string account;

        std::map<RoleId, std::string> roleList;
    };
    using LockGuard = std::lock_guard<componet::Spinlock>;

    LoginProcessor();
public:
    NON_COPYABLE(LoginProcessor)

    ~LoginProcessor() = default;

    void newClient(LoginId loginId, const std::string& account);
    void clientConnReady(LoginId loginId);

    void delClient(LoginId loginId);

    void timerExec(const componet::TimePoint& now);
    void regMsgHandler();
void test();
private:
    LoginId getLoginId();


private:
    ClientInfo::Ptr getClientByLoginId(LoginId loginId);

    //处理client消息
    void clientmsg_CreateRole(const uint8_t* msgData, uint32_t msgSize, LoginId loginId);
    void clientmsg_SelectRole(const uint8_t* msgData, uint32_t msgSize, LoginId loginId);
    void clientmsg_GetRandName(const uint8_t* msgData, uint32_t msgSize, LoginId loginId);

    //处理db的消息
    void servermsg_RetRoleList(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_RetCreateRole(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_RetRandName(const uint8_t* msgData, uint32_t msgSize);

private:
    uint32_t m_lastLoginId;

    componet::Spinlock m_disconnectedClientsLock;
    std::list<LoginId> m_disconnectedClients;

    componet::Spinlock m_clientsLock;
    std::map<LoginId, ClientInfo::Ptr> m_clients; //<loginId, clientInfo>, 消息驱动

public:
    static LoginProcessor& me();
};

}


#endif
