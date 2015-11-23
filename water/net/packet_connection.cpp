/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 19:06 +0800
 *
 * Description: 
 */

#include "packet_connection.h"

#include "net_exception.h"

#include <sys/socket.h>

namespace water{
namespace net{

PacketConnection::PacketConnection(TcpConnection&& tcpConn)
:TcpConnection(std::forward<TcpConnection&&>(tcpConn))
{
}

PacketConnection::~PacketConnection()
{
}

bool PacketConnection::setSendPacket(const Packet::Ptr& packet)
{
    if(m_sendBuf != nullptr)
        return false;

    m_sendBuf = packet;
    m_sendBuf->initBuffType(Packet::BuffType::send);
    return true;
}

void PacketConnection::setRecvPacket(const Packet::Ptr& packet)
{
    m_recvBuf = packet;
    m_recvBuf->initBuffType(Packet::BuffType::recv);
}

bool PacketConnection::trySend()
{
    if(m_sendBuf == nullptr)
        return true;

    const Packet::SizeType len = m_sendBuf->size() - m_sendBuf->getCursor();
    const Packet::SizeType sendSize = send(m_sendBuf->data() + m_sendBuf->getCursor(), len);
    if(sendSize == Packet::SizeType(-1))
        return false;

    m_sendBuf->addCursor(sendSize);
    if(m_sendBuf->getCursor() < m_sendBuf->size())
        return false;
    
    m_sendBuf = nullptr;
    return true;
}

bool PacketConnection::tryRecv()
{
    while(m_recvBuf->getCursor() < m_recvBuf->size())
    {
        const Packet::SizeType len = m_recvBuf->size() - m_recvBuf->getCursor();

        if(len < 0)
            EXCEPTION(RecvIllegalData, "分包错误");

        const Packet::SizeType recvSize = recv(m_recvBuf->data() + m_recvBuf->getCursor(), len);

        if(recvSize == 0)  //socket不可读
            SYS_EXCEPTION(ReadClosedConnection, "connection::recv");

        if(recvSize == Packet::SizeType(-1)) //暂时无数据
            return false;

        m_recvBuf->addCursor(recvSize);
    }

    return true;
}

Packet::Ptr PacketConnection::getRecvPacket() const
{
    return m_recvBuf;
}

}}

