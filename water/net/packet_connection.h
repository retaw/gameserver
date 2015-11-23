/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:04 +0800
 *
 * Description: 带缓冲区的connnection, 缓冲区为net::Packet 或 其派生类
                可以靠继承并重写Packet::expendData(size)方法来改变接收行为
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
DEFINE_EXCEPTION(ReadClosedConnection, net::NetException);
DEFINE_EXCEPTION(RecvIllegalData, net::NetException);

class PacketConnection : public net::TcpConnection
{
public:
    TYPEDEF_PTR(PacketConnection)
    CREATE_FUN_MAKE(PacketConnection)
    explicit PacketConnection(TcpConnection&& tcpConn);

public:
    ~PacketConnection();

public:
    //把packet放入发送缓冲，用于发送一个完整的packet
    bool setSendPacket(const Packet::Ptr& packet);

    //重置接收缓冲, 开始接收一个完整的packet
    void setRecvPacket(const Packet::Ptr& packet);
    //得到接收缓冲中的包，只有当tryRecv返回true时才有意义 
    Packet::Ptr getRecvPacket() const;

    //将发送缓冲写入socket，返回true表示缓冲已空
    bool trySend();
    //读取socket数据放入接收缓冲，返回true缓冲区内有一个完整的packet
    bool tryRecv();


private:
    Packet::Ptr m_sendBuf;
    Packet::Ptr m_recvBuf;
};

}}

#endif

