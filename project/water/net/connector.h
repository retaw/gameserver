/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:05 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_CONNECTOR_HPP
#define WATER_NET_CONNECTOR_HPP

#include "../componet/macro.h"
#include "endpoint.h"
#include "connection.h"

namespace water{
namespace net{
class TcpConnector
{

public:
    TYPEDEF_PTR(TcpConnector)
public:
    CREATE_FUN_MAKE(TcpConnector)
    TcpConnector(const std::string& strIp, uint16_t port);
    TcpConnector(const Endpoint& endPoint);

public:
    ~TcpConnector();

    TcpConnection::Ptr connect();

private:
    Endpoint m_remoteEndpoint;
};
}}

#endif
