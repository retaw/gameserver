#ifndef WATER_TCP_SERVER_H
#define WATER_TCP_SERVER_H

#include <atomic>
#include <set>
#include <unordered_map>

#include "net/epoller.h"
#include "net/endpoint.h"
#include "net/listener.h"
#include "componet/event.h"
#include "net/connection.h"

namespace water{

class TcpServer final
{
public:
    TcpServer();
    ~TcpServer() = default;
    void addLocalEndpoint(const net::Endpoint& ep);
    void run();
    void stop();

public:
    componet::Event<void (TcpServer*)> e_onClose;
    componet::Event<void (net::TcpConnection::Ptr)> e_newConn;

private:
    void epollEventHandler(int32_t socketFD, net::Epoller::Event event);

private:
    std::set<net::Endpoint> m_localEndpoints;
    std::unordered_map<int32_t, net::TcpListener::Ptr> m_listeners;
    net::Epoller m_epoller;

    enum class Switch : uint8_t {on, off};
    std::atomic<Switch> m_switch;
};

}

#endif
