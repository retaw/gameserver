/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-11 23:02 +0800
 *
 * Description: 
 */

#ifndef WATER_CONNECTION_MANAGER_H
#define WATER_CONNECTION_MANAGER_H

#include <unordered_map>
#include <atomic>

#include "net/connection.h"
#include "net/epoller.h"
#include "componet/spinlock.h"
#include "componet/event.h"

namespace water{


class TcpConnectionManager
{
public:
    TYPEDEF_PTR(TcpConnectionManager)
    CREATE_FUN_MAKE(TcpConnectionManager)

private:
    class ConnectionWithEpoller : public net::Epoller
    {
    public:
        bool insert(net::TcpConnection::Ptr conn);
        void erase(int32_t socketFD);
        net::TcpConnection::Ptr find(int32_t socketFD);

    private:
        std::unordered_map<int32_t, net::TcpConnection::Ptr> m_datas;
        componet::Spinlock m_lock;
    };

public:
    TcpConnectionManager();
    ~TcpConnectionManager() = default;

    enum class ConnType {in, out};
    void addConnection(net::TcpConnection::Ptr conn, ConnType ct);
    void delConnection(net::TcpConnection::Ptr conn);

    //从接收队列中取出一个packet, 并得到与其相关的conn
    bool getPacket(net::TcpConnection::Ptr* conn, net::Packet::Ptr* packet);

    void run();
    void stop();

public:
    componet::Event<void (TcpConnectionManager*)> e_onClose;

private:
    void epollerEventHandler(net::Epoller* epoller, int32_t socketFD, net::Epoller::Event event);

private:
    ConnectionWithEpoller m_conns;

    componet::LockFreeCircularQueueSPSC<std::pair<net::TcpConnection::Ptr, net::Packet::Ptr>> m_recvMsgQueue;

    enum class Switch : uint8_t {on, off};
    std::atomic<Switch> m_switch;
};

}

#endif
