/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-11 23:02 +0800
 *
 * Description: 
 */

#ifndef WATER_PACKET_CONNECTION_MANAGER_H
#define WATER_PACKET_CONNECTION_MANAGER_H

#include <unordered_map>
#include <atomic>

#include "packet_connection.h"
#include "net/epoller.h"
#include "componet/spinlock.h"
#include "componet/event.h"

namespace water{


class PacketConnectionManager
{
    /*
public:
    TYPEDEF_PTR(PacketConnectionManager)
    CREATE_FUN_MAKE(PacketConnectionManager)
*/
private:
    class ConnectionWithEpoller : public net::Epoller
    {
    public:
        bool insert(PacketConnection::Ptr conn);
        void erase(int32_t socketFD);
        PacketConnection::Ptr find(int32_t socketFD);

    private:
        componet::Spinlock m_lock;
        std::unordered_map<int32_t, PacketConnection::Ptr> m_conns;
//        std::unordered_map<ProcessType, std::vector<PacketConnection::Ptr>> m_ProcessConns;
    };

public:
    PacketConnectionManager();
    ~PacketConnectionManager() = default;

    bool addConnection(PacketConnection::Ptr conn);
    void delConnection(PacketConnection::Ptr conn);
    PacketConnection::Ptr getConnection() const;

    //从接收队列中取出一个packet, 并得到与其相关的conn
    bool getPacket(PacketConnection::Ptr* conn, Packet::Ptr* packet);

    void run();
    void stop();

public:
    componet::Event<void (PacketConnectionManager*)> e_close;

private:
    void epollerEventHandler(net::Epoller* epoller, int32_t socketFD, net::Epoller::Event event);

private:
    ConnectionWithEpoller m_connsEpoller;

    componet::LockFreeCircularQueueSPSC<std::pair<PacketConnection::Ptr, Packet::Ptr>> m_recvMsgQueue;

    enum class Switch : uint8_t {on, off};
    std::atomic<Switch> m_switch;
};

}

#endif
