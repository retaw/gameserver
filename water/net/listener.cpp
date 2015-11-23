/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 19:06 +0800
 *
 * Description: 
 */

#include "listener.h"
#include "net_exception.h"

#include <sys/types.h>
#include <arpa/inet.h>


namespace water{
namespace net{


void TcpListener::listen(int32_t backlog)
{
    if(-1 == ::listen(getFD(), backlog))
        SYS_EXCEPTION(NetException, "::listen");
}

TcpConnection::Ptr TcpListener::accept()
{
    struct sockaddr_in clientAddrIn;
    socklen_t clientAddrInSize = sizeof(clientAddrIn);

    int32_t fd = ::accept(getFD(), (struct sockaddr*)&clientAddrIn, &clientAddrInSize);
    if(-1 == fd)
    {
        //两种错误不视为错误
        if(errno == EAGAIN || errno == EWOULDBLOCK  //非阻塞的accept暂时没有收到请求
           || errno == EINTR) //被信号打断
            return nullptr;

        SYS_EXCEPTION(NetException, "::accept");
    }

    Endpoint remoteEndpoint;
    remoteEndpoint.ip.value = clientAddrIn.sin_addr.s_addr;     //远端ip
    remoteEndpoint.port     = ntohs(clientAddrIn.sin_port);   //远端port

    TcpConnection::Ptr ret = TcpConnection::create(fd, remoteEndpoint);
    return ret;
}

}}

