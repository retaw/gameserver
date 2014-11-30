/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:04 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_PACKET_CONNECTION_HPP
#define WATER_NET_PACKET_CONNECTION_HPP

#include "connection.h"
#include "net_exception.h"
#include "endpoint.h"
#include "packet.h"
#include "componet/lock_free_circular_queue_ss.h"


namespace water{
namespace net{

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
public:
    //把packet放入发送缓冲，用于发送一个完整的packet
    bool setSendBuffer(const Packet::Ptr& packet);
    //用要发送的数据放入发送缓，自动构建一个packet并发送
    bool setSendBuffer(const void* packetData, Packet::SizeType dataLen);
    //将发送缓冲写入socket，返回true表示缓冲已空
    bool trySendPacket();

    //重置接收缓冲, 开始接收一个完整的packet
    void setRecvBuffer();
    //重置接收缓冲并指定要接收的包体长度, 用于接收包体部分
    void setRecvBuffer(Packet::SizeType dataSize);
    //读取socket数据放入接收缓冲，返回true缓冲区内有一个完整的packet
    bool tryRecvPacket();

    //得到接收缓冲的只读句柄，在tryRecvPacket返回true时是一个已接收完整的包
    Packet::CPtr getRecvBuffer() const;

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

}}

#endif

