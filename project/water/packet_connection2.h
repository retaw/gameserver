/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:04 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_PACKET_CONNECTION_HPP
#define WATER_NET_PACKET_CONNECTION_HPP

#include "net/connection.h"
#include "net/net_exception.h"
#include "net/endpoint.h"
#include "componet/lock_free_circular_queue_ss.h"
#include "packet.h"


namespace water{


//异常定义：读取一个已经关闭的连接
DEFINE_EXCEPTION(ReadClosedConnection, componet::ExceptionBase);
DEFINE_EXCEPTION(RecvIllegalData, componet::ExceptionBase);

class PacketConnection : public net::TcpConnection
{
public:
    TYPEDEF_PTR(PacketConnection)
    CREATE_FUN_MAKE(PacketConnection)
    explicit PacketConnection(TcpConnection&& tcpConn, uint32_t recvQueueArg, uint32_t sendQueueArg);

public:
    ~PacketConnection();

//带缓冲队列的发送和接收，以下4个函数互相可以不加锁并发，但单个函数本身不可并发
public:
    //将消息放入发送队列，返回false表明队列已满
    bool putPacket(const Packet::Ptr& packet);
    //将消息从接收队列取出，返回nullptr表明队列已空
    Packet::Ptr getPacket();

    //执行发送，尽可能的把数据写入socket，返回true表示没有尚未写入socket的数据
    bool sendAll();
    //执行接收, 尽可能的从socket读数据, 返回true表示socket上的数据已全部读出
    bool recvAll();

//操作发送与接收缓冲，缓冲中的数据一定属于同一个packet
private:
    //把packet放入发送缓冲, packet的结构为：[4字节包体长度，包体，2个字节的魔数]
    bool setSendPacket(const Packet::Ptr& packet);
    //注册要发送的数据放入发送缓冲
    bool setSendPacket(const void* packetData, int32_t dataLen);
    //将发送缓冲写入socket，返回true表示缓冲中的这个packet已发送完毕
    bool trySendPacket();

    //清除接收缓冲
    void clearRecvPacket();
    //读取socket到接收缓冲，返回true表示收到一个完整的packet
    bool tryRecvPacket();

    //检查packet的结构：[4字节包体长度，包体，2个字节的魔数] 
    bool checkPacket(const Packet::Ptr& packet);

private:
    struct PacketBuffer
    {
        Packet::Ptr buf;
        Packet::size_type doneSize = 0;
    };
    PacketBuffer m_sendBuf;
    PacketBuffer m_recvBuf;

    componet::LockFreeCircularQueueSPSC<Packet::Ptr> m_sendQueue;
    componet::LockFreeCircularQueueSPSC<Packet::Ptr> m_recvQueue;
};

}

#endif

