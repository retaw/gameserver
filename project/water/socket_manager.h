/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-08 22:06 +0800
 *
 * Description: 
 */

#ifndef WATER_SOCKET_MANAGER_H
#define WATER_SOCKET_MANAGER_H

#include <functional>

#include "componet/componet.h"
#include "net/connection.h"
#include "net/epoller.h"
#include "componet/spinlock.h"

namespace water{

template <typename SocketType>
class SocketManager final
{
    typedef componet::Spinlock Lock;
public:
    SocketManager() = default;
    ~SocketManager() = default;

    void insert(typename SocketType::Ptr conn, net::Epoller::EventType epollEvent)
    {
        m_lock.lock();
        m_epoller.regSocket(conn.get(), epollEvent);
        m_connections.insert(conn);
        m_lock.unlock();
    }

    void erase(net::TcpConnection* conn)
    {
        m_lock.lock();
        //删除顺序不更改变
        m_epoller.delSocket(conn);
        m_connections.erase(conn);
        m_lock.unlock();
    }

    net::Epoller& epoller()
    {
        return m_epoller;
    }

private:
    Lock m_lock;
    net::Epoller m_epoller;
    std::vector<typename SocketType::Ptr> m_connections;
};



}

#endif
