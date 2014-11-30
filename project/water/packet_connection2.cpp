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

PacketConnection::PacketConnection(TcpConnection&& tcpConn, uint32_t recvQueueArg, uint32_t sendQueueArg)
:TcpConnection(std::forward<TcpConnection&&>(tcpConn)), m_sendQueue(recvQueueArg), m_recvQueue(sendQueueArg)
{
}

PacketConnection::~PacketConnection()
{
    try
    {
        if(getState() != ConnState::CLOSED)
            close();
    }
    catch(...)
    {
    }
}

bool PacketConnection::setSendPacket(const Packet::Ptr& packet)
{
    if(m_sendBuf.buf != nullptr)
        return false;

    m_sendBuf.buf = packet;
    m_sendBuf.doneSize = 0;

    return true;
}

bool PacketConnection::setSendPacket(const void* packetData, int32_t dataLen)
{
    if(m_sendBuf.buf != nullptr)
        return false;

    m_sendBuf.buf = std::make_shared<Packet>();
    m_sendBuf.buf->setContent(packetData, dataLen);

    m_sendBuf.doneSize = 0;

    return true;
}

bool PacketConnection::trySendPacket()
{
    if(m_sendBuf.buf == nullptr)
        return true;

    const int32_t len = m_sendBuf.buf->size() - m_sendBuf.doneSize;
    const int32_t sendSize = send(m_sendBuf.buf->data() + m_sendBuf.doneSize, len);
    if(sendSize == -1)
        return false;

    m_sendBuf.doneSize += sendSize;
    if(m_sendBuf.doneSize < m_sendBuf.buf->size())
        return false;
    
    m_sendBuf.buf = nullptr;
    return true;
}

bool PacketConnection::tryRecvPacket()
{
    if(m_recvBuf.buf == nullptr)
    {
        m_recvBuf.buf = Packet::create();
        m_recvBuf.buf->resize(sizeof(Packet::SizeType));
        m_recvBuf.doneSize = 0;
    }

    while(m_recvBuf.doneSize != m_recvBuf.buf->size())
    {
        const int32_t len = m_recvBuf.buf->size() - m_recvBuf.doneSize;

        if(len < 0)
            EXCEPTION(RecvIllegalData, "分包错误");

        const int32_t recvSize = recv(m_recvBuf.buf->data() + m_recvBuf.doneSize, len);

        if(recvSize == 0)
            EXCEPTION(ReadClosedConnection, "");

        if(recvSize == -1) //暂时无数据
            return false;

        m_recvBuf.doneSize += recvSize;

        if(m_recvBuf.doneSize == sizeof(Packet::SizeType)) //收到一个包头
        {
            //分配包体的内存
            const int32_t contentSize = m_recvBuf.buf->contentSize();
            m_recvBuf.buf->resize(m_recvBuf.doneSize + contentSize);
        }
    }

    if(!checkPacket(m_recvBuf.buf))
        EXCEPTION(RecvIllegalData, "包格式错误");

    return true;
}

void PacketConnection::clearRecvPacket()
{
    m_recvBuf.buf = nullptr;
}

bool PacketConnection::checkPacket(const Packet::Ptr& packet)
{
    const uint32_t headSize = sizeof(Packet::SizeType);

    const uint8_t* buf = packet->data();
    const uint32_t contentSize = *reinterpret_cast<const uint32_t*>(buf);
    return packet->size() == headSize + contentSize;
}

bool PacketConnection::sendAll()
{
    while(trySendPacket())
    {
        Packet::Ptr packet;
        if(!m_sendQueue.pop(&packet))
            return true; //发送队列已空，即所有要发送的包已全部发出

        setSendPacket(packet); //while被执行说明当前发送缓冲已空，故这里一定会成功
    }
    return false;
}

bool PacketConnection::recvAll()
{
    while(tryRecvPacket())
    {
        if(!m_recvQueue.push(m_recvBuf.buf))
            return false; //接收队列满，即至少有一个包还没放入接收队列，socket可能还有数据

        clearRecvPacket();
    }
    return true;
}

bool PacketConnection::putPacket(const Packet::Ptr& packet)
{
    return m_sendQueue.push(packet);
}

Packet::Ptr PacketConnection::getPacket()
{
    Packet::Ptr ret;
    m_sendQueue.pop(&ret);
    return ret;
}

}

