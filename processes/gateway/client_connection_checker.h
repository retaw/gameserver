/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-14 16:04 +0800
 *
 * Modified: 2015-03-14 16:04 +0800
 *
 * Description:  客户端连接检查器, 处理连接合法性检查和登录流程
 */

#ifndef PROCESS_GATEWAY_CLIENT_CONNECTION_CHECKER_H
#define PROCESS_GATEWAY_CLIENT_CONNECTION_CHECKER_H


#include "def.h"

#include "water/componet/spinlock.h"
#include "water/componet/datetime.h"
#include "water/componet/event.h"
#include "water/process/process_thread.h"
#include "water/process/connection_checker.h"

#include <list>

namespace gateway{

typedef uint64_t LoginId;

using namespace water;

class ClientConnectionChecker
{
    enum class ClientState
    {
        recvingToken,
    };
    struct ClientInfo
    {
        ClientState state;
        water::net::PacketConnection::Ptr conn;
    };

public:
    TYPEDEF_PTR(ClientConnectionChecker)
    CREATE_FUN_MAKE(ClientConnectionChecker)
    
    NON_COPYABLE(ClientConnectionChecker)


    ClientConnectionChecker();
    ~ClientConnectionChecker() = default;

    //添加一个链接
    void addUncheckedConnection(water::net::PacketConnection::Ptr conn);

    void timerExec(const componet::TimePoint& now);

public:
    componet::Event<void (water::net::PacketConnection::Ptr, RoleId)> e_clientConfirmed;

private:
    LoginId getLoginId();

private:

    uint32_t m_lastLoginIdCounter;

    typedef std::lock_guard<componet::Spinlock> LockGuard;
    componet::Spinlock m_clientsLock;
    std::list<ClientInfo> m_clients; //<loginId, clientInfo>, 消息驱动
};

}


#endif
