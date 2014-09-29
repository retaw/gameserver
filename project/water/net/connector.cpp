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
#include <arpa/inet.h>

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
    struct sockaddr_in serverAddrIn;
    std::memset(&serverAddrIn, 0, sizeof(serverAddrIn));

    serverAddrIn.sin_family      = AF_INET;
    serverAddrIn.sin_addr.s_addr = m_remoteEndpoint.ip.value;
    serverAddrIn.sin_port        = ::htons(m_remoteEndpoint.port);

    TcpConnection::Ptr ret = TcpConnection::create(m_remoteEndpoint);
    if(-1 == ::connect(ret->getFD(), (const struct sockaddr*)&serverAddrIn, sizeof(serverAddrIn)))
        SYS_EXCEPTION(NetException, "::connect:");

    return ret;
}

}}
