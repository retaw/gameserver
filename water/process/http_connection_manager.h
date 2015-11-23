/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-11 23:02 +0800
 *
 * Description: 
 */

#ifndef WATER_HTTP_CONNECTION_MANAGER_H
#define WATER_HTTP_CONNECTION_MANAGER_H

#include <unordered_map>
#include <atomic>

#include "net/packet_connection.h"
#include "net/epoller.h"

#include "componet/event.h"
#include "componet/class_helper.h"
#include "componet/spinlock.h"
#include "componet/lock_free_circular_queue_ss.h"

#include "process_id.h"
#include "process_thread.h"

namespace water{
namespace process{

class HttpConnectionManager : public ProcessThread
{
public:
    struct ConnectionHolder
    {
        TYPEDEF_PTR(ConnectionHolder);


        int64_t id;
        net::PacketConnection::Ptr conn;
        //由于socke太忙而暂时无法发出的包，缓存在这里
        componet::LockFreeCircularQueueSPSC<net::Packet::Ptr>::Ptr sendQueue; 
    };
public:
    HttpConnectionManager();
    ~HttpConnectionManager() = default;

    bool exec() override;

    void addPrivateConnection(net::PacketConnection::Ptr conn);
    void delConnection(net::PacketConnection::Ptr conn);

    //从接收队列中取出一个packet, 并得到与其相关的conn
    bool sendPacket(ConnectionHolder::Ptr connHolder, net::Packet::Ptr packet);

public:
    componet::Event<void (HttpConnectionManager*)> e_close;
    componet::Event<void (ConnectionHolder::Ptr, net::Packet::CPtr) > e_packetrecv;

private:
    void epollerEventHandler(int32_t socketFD, net::Epoller::Event event);

private:

	int32_t uniqueID = 0;
    componet::Spinlock m_lock;

    net::Epoller m_epoller;
    //所有的连接, {fd, conn}
    std::unordered_map<int32_t, ConnectionHolder::Ptr> m_allConns;
    //私网连接, {processId.value, conn}
    std::unordered_map<int64_t, ConnectionHolder::Ptr> m_privateConns;
    //公网连接, {accountId, conn}, 暂空，需要一个确定账号id的机制配合
    //std::unordered_map<PublicClientIdentity, ConnectionHolder::Ptr>

    componet::LockFreeCircularQueueSPSC<std::pair<ConnectionHolder::Ptr, net::Packet::CPtr>> m_recvQueue;
};

}}

#endif
