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
: m_remoteEndpoint(remoteEndpoint), m_state(ConnState::read_nad_write)
{
}

TcpConnection::TcpConnection(int32_t socketFD, const Endpoint& remoteEndpoint)
: TcpSocket(socketFD), m_remoteEndpoint(remoteEndpoint), m_state(ConnState::read_nad_write)
{
}

TcpConnection::~TcpConnection()
{
    try
    {
        if(getState() != ConnState::closed)
            close();
    }
    catch(...)
    {
    }
}

TcpConnection::TcpConnection(TcpConnection&& other)
    :TcpSocket(std::move(other))
{
}

TcpConnection& TcpConnection::operator=(TcpConnection&& other)
{
    TcpSocket::operator=(std::move(other));
    m_remoteEndpoint = std::move(other.m_remoteEndpoint);
    m_state = std::move(other.m_state);
    return *this;
}

const Endpoint& TcpConnection::getRemoteEndpoint() const
{
    return m_remoteEndpoint;
}

void TcpConnection::shutdown(ConnState state)
{
    if(!isAvaliable())
        return;

    switch (state)
    {
    case ConnState::read:
        ::shutdown(getFD(), SHUT_RD);
    case ConnState::write:
        ::shutdown(getFD(), SHUT_WR);
        break;
    case ConnState::read_nad_write:
        ::shutdown(getFD(), SHUT_RDWR);
        break;
    default:
        break;
    }

    const uint8_t stateValue = (uint8_t)state;
    const uint8_t m_stateValue = (uint8_t)m_state;
    m_state = (ConnState)(m_stateValue & ~stateValue);

    if(m_state == ConnState::closed)
        close();
}

TcpConnection::ConnState TcpConnection::getState() const
{
    return m_state;
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


}}

