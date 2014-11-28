#ifndef WATER_TCP_CLIENT_H
#define WATER_TCP_CLIENT_H

#include <atomic>
#include <map>

#include "net/endpoint.h"
#include "net/connection.h"
#include "componet/datetime.h"
#include "componet/event.h"
#include "componet/class_helper.h"
#include "process_thread.h"

namespace water{

class TcpClient : public ProcessThread
{
public:
    TYPEDEF_PTR(TcpClient)
    CREATE_FUN_MAKE(TcpClient)

    TcpClient();
    void addRemoteEndpoint(net::Endpoint ep, std::chrono::seconds retryInterval);
    bool exec();

public:
    componet::Event<void (net::TcpConnection::Ptr)> e_newConn;
    componet::Event<void (TcpClient*)> e_close;

private:
    struct RemoteEndpointInfo
    {
        componet::TimePoint retryTimepoint;
        std::chrono::seconds retryInterval;
        net::TcpConnection::WPtr conn;
    };

    std::map<net::Endpoint, RemoteEndpointInfo> m_remoteEndpoints;
};

}

#endif
