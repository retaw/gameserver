#ifndef ROUTER_CONNECTION_MANAGER_HPP
#define ROUTER_CONNECTION_MANAGER_HPP

#include "water/tcp_connection_manager.h"

class ConnectonManager : public water::TcpConnectionManager
{
    using namespace water;
public:
    ConnectonManager();
    ~ConnectonManager();

    virtual void msgHandler(net::TcpConnection::Ptr conn, net::Packet::Ptr packet) override;

private:
};

#endif
