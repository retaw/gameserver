#ifndef WATER_TCP_CLIENT_H
#define WATER_TCP_CLIENT_H

#include <atomic>
#include <map>

#include "net/endpoint.h"
#include "net/connection.h"
#include "componet/datetime.h"
#include "componet/event.h"

namespace water{

class TcpClient
{
public:
    TcpClient();

    void addRemoteEndpoint(net::Endpoint ep, std::chrono::seconds retryInterval);

    void run();

    void stop();

public:
    componet::Event<void (net::TcpConnection::Ptr)> e_newConn;
    componet::Event<void (TcpClient*)> e_onClose;

private:
    struct RemoteEndpointInfo
    {
        componet::TimePoint retryTimepoint;
        std::chrono::seconds retryInterval;
        net::TcpConnection::WPtr conn;
    };

    std::map<net::Endpoint, RemoteEndpointInfo> m_remoteEndpoints;

    enum class Switch : uint8_t {on, off};
    std::atomic<Switch> m_switch;
};

}

#endif
