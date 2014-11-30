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

#include "net/packet_connection.h"
#include "net/epoller.h"
#include "componet/spinlock.h"
#include "componet/event.h"

namespace water{


class PacketConnectionManager
{
private:
    struct ConnectonHolder
    {
        net::PacketConnection::Ptr conn;
        std::vector<net::Packet::Ptr> sendQueue;
    };

    class ConnectionWithEpoller : public net::Epoller
    {
    public:
        bool insert(net::PacketConnection::Ptr conn);
        void erase(int32_t socketFD);
        net::PacketConnection::Ptr find(int32_t socketFD);

    private:
        componet::Spinlock m_lock;
        std::unordered_map<int32_t, net::PacketConnection::Ptr> m_conns;
//        std::unordered_map<ProcessType, std::vector<net::PacketConnection::Ptr>> m_ProcessConns;
    };

public:
    PacketConnectionManager();
    ~PacketConnectionManager() = default;

    bool addConnection(net::PacketConnection::Ptr conn);
    void delConnection(net::PacketConnection::Ptr conn);
    net::PacketConnection::Ptr getConnection() const;

    //从接收队列中取出一个packet, 并得到与其相关的conn
    bool getPacket(net::PacketConnection::Ptr* conn, Packet::Ptr* packet);

    void run();
    void stop();

public:
    componet::Event<void (PacketConnectionManager*)> e_close;

private:
    void epollerEventHandler(net::Epoller* epoller, int32_t socketFD, net::Epoller::Event event);

private:
    ConnectionWithEpoller m_connsEpoller;

    componet::LockFreeCircularQueueSPSC<std::pair<net::PacketConnection::Ptr, Packet::Ptr>> m_recvMsgQueue;

    enum class Switch : uint8_t {on, off};
    std::atomic<Switch> m_switch;
};

}

#endif
