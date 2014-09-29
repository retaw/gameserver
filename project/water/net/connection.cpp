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
: m_remoteEndpoint(remoteEndpoint), m_state(ConnState::READ_AND_WRITE)
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

uint32_t TcpConnection::send(const void* buf, int32_t bufLen)
{
    uint32_t sendLen = ::send(getFD(), buf, bufLen, MSG_NOSIGNAL);
    if(sendLen == static_cast<uint32_t>(-1))
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return uint32_t(-1);

        SYS_EXCEPTION(NetException, "::send");
    }

    return sendLen;
}

uint32_t TcpConnection::recv(void* buf, int32_t bufLen)
{
    uint32_t recvLen = ::recv(getFD(), buf, bufLen, 0);
    if(recvLen == static_cast<uint32_t>(-1))
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return uint32_t(-1);
        SYS_EXCEPTION(NetException, "::recv");
    }
    return recvLen;
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

}}
