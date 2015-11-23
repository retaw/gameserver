/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 19:06 +0800
 *
 * Description: 
 */

#include "connector.h"

#include "net_exception.h"

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h> //htons

namespace water{
namespace net{

TcpConnector::TcpConnector(const std::string& strIp, uint16_t port)
{
    m_remoteEndpoint.ip.fromString(strIp);
    m_remoteEndpoint.port = port;
}

TcpConnector::TcpConnector(const Endpoint& endPoint)
: m_remoteEndpoint(endPoint)
{
}

TcpConnector::~TcpConnector()
{
}

TcpConnection::Ptr TcpConnector::connect()
{
    ::sockaddr_in serverAddrIn;
    std::memset(&serverAddrIn, 0, sizeof(serverAddrIn));

    serverAddrIn.sin_family      = AF_INET;
    serverAddrIn.sin_addr.s_addr = m_remoteEndpoint.ip.value;
    serverAddrIn.sin_port        = htons(m_remoteEndpoint.port);

    TcpConnection::Ptr ret = TcpConnection::create(m_remoteEndpoint);
    if(-1 == ::connect(ret->getFD(), (const struct sockaddr*)&serverAddrIn, sizeof(serverAddrIn)))
        SYS_EXCEPTION(NetException, "::connect:");

    return ret;
}

TcpConnection::Ptr TcpConnector::connect(const std::chrono::milliseconds& timeout)
{
    ::sockaddr_in serverAddrIn;
    std::memset(&serverAddrIn, 0, sizeof(serverAddrIn));

    serverAddrIn.sin_family      = AF_INET;
    serverAddrIn.sin_addr.s_addr = m_remoteEndpoint.ip.value;
    serverAddrIn.sin_port        = htons(m_remoteEndpoint.port);

    TcpConnection::Ptr ret = TcpConnection::create(m_remoteEndpoint);

    //用非阻塞方式来连接
    ret->setNonBlocking();
    const int32_t connectRet = ::connect(ret->getFD(), (const ::sockaddr*)&serverAddrIn, sizeof(serverAddrIn));

    //直接成功了
    if(connectRet == 0)
    {
        ret->setBlocking();
        return ret;
    }

    //确认是否正在连接中
    if(errno != EINPROGRESS)
        SYS_EXCEPTION(NetException, "::connect:");

    //用select实现超时等待
    const int32_t fd = ret->getFD();
    ::timeval t;
    t.tv_sec = timeout.count() / 1000;
    t.tv_usec = timeout.count() % 1000;

    fd_set writeSet, errSet;
    FD_ZERO(&writeSet);
    FD_ZERO(&errSet);
    FD_SET(fd, &writeSet);
    FD_SET(fd, &errSet);

    const int selectRet = ::select(fd + 1, nullptr, &writeSet, &errSet, &t);
    if(selectRet > 0) //正常返回，检查返回原因，确定是否连接成功
    {
        int32_t connectErrno = 0;
        socklen_t connectErrnoLen = sizeof(connectErrnoLen);
        if(-1 == ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &connectErrno, &connectErrnoLen))
            SYS_EXCEPTION(NetException, "::getsocketopt");

        if(connectErrno != 0)//连接失败
        {
            errno = connectErrno;
            SYS_EXCEPTION(NetException, "::connect");
        }
    }
    else if(selectRet == 0) //超时返回
    {
        return nullptr;
    }
    else //select 出错
    {
        SYS_EXCEPTION(NetException, "::select");
    }

    //select非超时返回，检查socket是否出错

    ret->setBlocking();
    return ret;
}

}}
