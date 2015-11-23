/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-14 16:04 +0800
 *
 * Modified: 2015-08-15 20:15 +0800
 *
 * Description:  客户端连接检查器, 处理连接合法性检查和登录流程, 暂时不做任何验证, 一切连接皆直接视为合法 
 */

#ifndef PROCESS_SUPER_CLIENT_CONNECTION_CHECKER_H
#define PROCESS_SUPER_CLIENT_CONNECTION_CHECKER_H


#include "common/commdef.h"
#include "common/roledef.h"

#include "water/componet/spinlock.h"
#include "water/componet/datetime.h"
#include "water/componet/event.h"
#include "water/process/process_thread.h"
#include "water/process/connection_checker.h"

#include <list>

namespace super{

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
    componet::Event<void (water::net::PacketConnection::Ptr, UserId)> e_clientConfirmed;

private:
    LoginId getLoginId();

private:
    LoginId m_lastLoginIdCounter;

    typedef std::lock_guard<componet::Spinlock> LockGuard;
    componet::Spinlock m_clientsLock;
    std::list<ClientInfo> m_clients; //<loginId, clientInfo>, 消息驱动
};

}


#endif
