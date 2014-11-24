/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 19:07 +0800
 *
 * Description: 
 */

#include "socket.h"

#include "net_exception.h"

#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

namespace water{
namespace net{
TcpSocket::TcpSocket()
: m_fd(::socket(PF_INET, SOCK_STREAM, 0))
{
    if(m_fd == -1)
        SYS_EXCEPTION(NetException, "::socket");

}

TcpSocket::TcpSocket(int32_t fd)
: m_fd(fd)
{
}

TcpSocket::~TcpSocket()
{
    try
    {
        close();
    }
    catch(...)
    {
    }
}

void TcpSocket::close()
{
    if(!isAvaliable())
        return;

    ::close(m_fd);
    e_onClose(this);
    m_fd = -1;
}

int32_t TcpSocket::getFD() const
{
    return m_fd;
}

bool TcpSocket::isAvaliable() const
{
    return m_fd != -1;
}

bool TcpSocket::isNonBlocking() const
{
    return TcpSocket::isNonBlockingFD(m_fd);
}

void TcpSocket::setNonBlocking()
{
    if(isNonBlocking())
        return;

    int32_t flags = ::fcntl(m_fd, F_GETFL, 0);
    if(-1 == flags)
        SYS_EXCEPTION(NetException, "::fcntl");
    if(-1 == ::fcntl(m_fd, F_SETFL, flags | O_NONBLOCK))
        SYS_EXCEPTION(NetException, "::fcntl");
}

void TcpSocket::setBlocking()
{
    if(!isNonBlocking())
        return;

    int32_t flags = ::fcntl(m_fd, F_GETFL, 0);
    if(-1 == flags)
        SYS_EXCEPTION(NetException, "::fcntl");
    if(-1 == ::fcntl(m_fd, F_SETFL, flags & ~O_NONBLOCK))
        SYS_EXCEPTION(NetException, "::fcntl");
}

void TcpSocket::bind(uint16_t port)
{
    Endpoint endpoint;
    endpoint.ip.value = ::htonl(INADDR_ANY);
    endpoint.port     = port;

    bind(endpoint);
}

void TcpSocket::bind(const std::string& strIp, uint16_t port)
{
    Endpoint endpoint;
    endpoint.ip.fromString(strIp);
    endpoint.port = port;

    bind(endpoint);
}

void TcpSocket::bind(const Endpoint& endpoint)
{
    int32_t flag = 1;
    if(-1 == ::setsockopt(getFD(), SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)))
        SYS_EXCEPTION(NetException, "::setsockopt");

    struct sockaddr_in serverAddrIn;
    std::memset(&serverAddrIn, 0, sizeof(serverAddrIn));

    serverAddrIn.sin_family = AF_INET;
    serverAddrIn.sin_addr.s_addr = endpoint.ip.value;
    serverAddrIn.sin_port = ::htons(endpoint.port);

    if(-1 == ::bind(getFD(), (const struct sockaddr*)&serverAddrIn, sizeof(serverAddrIn)))
        SYS_EXCEPTION(NetException, "::bind");
}

bool TcpSocket::isNonBlockingFD(int32_t fd)
{
    int32_t flags = ::fcntl(fd, F_GETFL, 0);
    if(-1 == flags)
        SYS_EXCEPTION(NetException, "::fcntl");

    return (flags & O_NONBLOCK) > 0;
}
/*
void TcpSocket::setEpollReadCallback(EpollCallback cb)
{
    m_epollReadCallback = cb;
}

void TcpSocket::setEpollWriteCallback(EpollCallback cb)
{
    m_epollWriteCallback = cb;
}

void TcpSocket::setEpollErrorCallback(EpollCallback cb)
{
    m_epollErrorCallback = cb;
}
*/
}}

