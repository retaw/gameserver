/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-11 23:02 +0800
 *
 * Description: 
 */

#ifndef WATER_TCP_CONNECTION_MANAGER_H
#define WATER_TCP_CONNECTION_MANAGER_H

#include "net/packet_connection.h"
#include "net/epoller.h"

#include "componet/event.h"
#include "componet/class_helper.h"
#include "componet/spinlock.h"
#include "componet/lock_free_circular_queue_ss.h"

#include "process_id.h"
#include "process_thread.h"
#include "tcp_packet.h"

#include <unordered_map>
#include <list>

namespace water{
namespace process{

class TcpConnectionManager : public ProcessThread
{
public:
    TYPEDEF_PTR(TcpConnectionManager)
    CREATE_FUN_MAKE(TcpConnectionManager)

    //临时
    using ClinetIdentity = int64_t;
    enum class ConnType {privateType, publicType};

    struct ConnectionHolder
    {
        TYPEDEF_PTR(ConnectionHolder);
        ConnType type;
        int64_t id;

        net::PacketConnection::Ptr conn;

        //由于socke太忙而暂时无法发出的包，缓存在这里
        std::list<net::Packet::Ptr> sendQueue; 
        //发包时需要一个锁，因为conn的发送函数和sendQueue都是不可重入的
        componet::Spinlock sendLock;
    };
public:
    TcpConnectionManager() = default;
    ~TcpConnectionManager() = default;

    void setSelfZoneId(ZoneId zoneId);

    bool exec() override;

    void addPrivateConnection(net::PacketConnection::Ptr conn, ProcessIdentity processId);
    bool addPublicConnection(net::PacketConnection::Ptr conn, ClinetIdentity clientId);

    net::PacketConnection::Ptr erasePublicConnection(ClinetIdentity clientId);

    //当前的
    uint32_t totalPrivateConnNum() const;

    //从接收队列中取出一个packet, 并得到与其相关的conn
    bool getPacket(ConnectionHolder::Ptr* conn, net::Packet::Ptr* packet);

    bool sendPacketToPrivate(ProcessIdentity processId, net::Packet::Ptr packet);
    void broadcastPacketToPrivate(ZoneId zoneId, ProcessType processType, net::Packet::Ptr packet);
    void broadcastPacketToPrivate(ProcessType processType, net::Packet::Ptr packet);

    bool sendPacketToPublic(ClinetIdentity clientId, net::Packet::Ptr packet);
    void broadcastPacketToPublic(net::Packet::Ptr packet);


public:
    //添加conn成功的事件
    componet::Event<void (ProcessIdentity id)> e_afterAddPrivateConn;

    //删除conn成功的事件
    componet::Event<void (ProcessIdentity id)> e_afterErasePrivateConn;
    componet::Event<void (ClinetIdentity  id)> e_afterErasePublicConn;

private:
    void epollerEventHandler(int32_t socketFD, net::Epoller::Event event);
    bool sendPacket(ConnectionHolder::Ptr connHolder, net::Packet::Ptr packet);
    void eraseConnection(net::PacketConnection::Ptr conn);

private:
    //本进程所属的zoneId
    ZoneId m_selfZoneId = INVALID_ZONE_ID;

    net::Epoller m_epoller;

    componet::Spinlock m_lock;

    //所有的连接, 以socketFd为索引 {socketFd, conn}
    std::unordered_map<int32_t, ConnectionHolder::Ptr> m_allConns;

    //私网连接, 二层索引, {zoneAndType, {num, conn}}
    std::unordered_map<uint64_t, std::unordered_map<ProcessNum, ConnectionHolder::Ptr>> m_privateConns;
    //公网连接
    std::unordered_map<ClinetIdentity, ConnectionHolder::Ptr> m_publicConns;

    componet::LockFreeCircularQueueSPSC<std::pair<ConnectionHolder::Ptr, net::Packet::Ptr>> m_recvQueue;
};

}}

#endif
