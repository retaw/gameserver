/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 19:06 +0800
 *
 * Description: 
 */

#include "connection.h"

#include "net_exception.h"

#include <sys/socket.h>

namespace water{
namespace net{

TcpConnection::TcpConnection(const Endpoint& remoteEndpoint)
: m_remoteEndpoint(remoteEndpoint), m_state(ConnState::READ_AND_WRITE), m_sendQueue(0), m_recvQueue(8)
{
}

TcpConnection::TcpConnection(int32_t socketFD, const Endpoint& remoteEndpoint)
: TcpSocket(socketFD), m_remoteEndpoint(remoteEndpoint), m_state(ConnState::READ_AND_WRITE)
{
}

TcpConnection::~TcpConnection()
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

const Endpoint& TcpConnection::getRemoteEndpoint() const
{
    return m_remoteEndpoint;
}

int32_t TcpConnection::send(const void* data, int32_t dataLen)
{
    uint32_t sendLen = ::send(getFD(), data, dataLen, MSG_NOSIGNAL);
    if(sendLen == static_cast<uint32_t>(-1))
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return -1;

        SYS_EXCEPTION(NetException, "::send");
    }

    return sendLen;
}

int32_t TcpConnection::recv(void* data, int32_t dataLen)
{
    uint32_t recvLen = ::recv(getFD(), data, dataLen, 0);
    if(recvLen == static_cast<uint32_t>(-1))
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return -1;
        SYS_EXCEPTION(NetException, "::recv");
    }
    return recvLen;
}

bool TcpConnection::setSendPacket(const Packet::Ptr& packet)
{
    if(m_sendBuf.buf != nullptr)
        return false;

    m_sendBuf.buf = packet;
    m_sendBuf.doneSize = 0;

    return true;
}

bool TcpConnection::setSendPacket(const void* packetData, int32_t dataLen)
{
    if(m_sendBuf.buf != nullptr)
        return false;

    m_sendBuf.buf = std::make_shared<Packet>();
    m_sendBuf.buf->setMsg(packetData, dataLen);

    m_sendBuf.doneSize = 0;

    return true;
}

bool TcpConnection::trySendPacket()
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

bool TcpConnection::tryRecvPacket()
{
    if(m_recvBuf.buf == nullptr)
    {
        m_recvBuf.buf = Packet::create();
        m_recvBuf.buf->resize(sizeof(PacketDataSize));
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

        if(m_recvBuf.doneSize == sizeof(PacketDataSize)) //收到一个包头
        {
            //分配包体的内存
            const int32_t dataSize = m_recvBuf.buf->msgSize();
            m_recvBuf.buf->resize(m_recvBuf.doneSize + dataSize);
        }
    }

    if(!checkPacket(m_recvBuf.buf))
        EXCEPTION(RecvIllegalData, "包格式错误");

    return true;
}

void TcpConnection::clearRecvPacket()
{
    m_recvBuf.buf = nullptr;
}

void TcpConnection::shutdown(ConnState state)
{
    if(!isAvaliable())
        return;

    switch (state)
    {
    case ConnState::READ:
        ::shutdown(getFD(), SHUT_RD);
    case ConnState::WRITE:
        ::shutdown(getFD(), SHUT_WR);
        break;
    case ConnState::READ_AND_WRITE:
        ::shutdown(getFD(), SHUT_RDWR);
        break;
    default:
        break;
    }

    const uint8_t stateValue = (uint8_t)state;
    const uint8_t m_stateValue = (uint8_t)m_state;
    m_state = (ConnState)(m_stateValue & ~stateValue);

    if(m_state == ConnState::CLOSED)
        close();
}

TcpConnection::ConnState TcpConnection::getState() const
{
    return m_state;
}

bool TcpConnection::checkPacket(const Packet::Ptr& packet)
{
    const uint32_t headSize = sizeof(PacketDataSize);

    const uint8_t* buf = packet->data();
    const uint32_t dataSize = *reinterpret_cast<const uint32_t*>(buf);
    return packet->size() == headSize + dataSize;
}

bool TcpConnection::sendAll()
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

bool TcpConnection::recvAll()
{
    while(tryRecvPacket())
    {
        if(!m_recvQueue.push(m_recvBuf.buf))
            return false; //接收队列满，即至少有一个包还没放入接收队列，socket可能还有数据

        clearRecvPacket();
    }
    return true;
}

bool TcpConnection::putPacket(const Packet::Ptr& packet)
{
    return m_sendQueue.push(packet);
}

Packet::Ptr TcpConnection::getPacket()
{
    Packet::Ptr ret;
    m_sendQueue.pop(&ret);
    return ret;
}

}}

