/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-27 16:30 +0800
 *
 * Description: 
 */


#ifndef WATER_PROCESS_CONNECTION_CHECKET_H 
#define WATER_PROCESS_CONNECTION_CHECKET_H 

#include "process_id.h"
#include "componet/event.h"
#include "net/packet_connection.h"
#include "process_thread.h"

namespace water{
namespace process{


class ConnectionChecker : public ProcessThread
{
public:
    TYPEDEF_PTR(ConnectionChecker)

    ConnectionChecker() = default;
    virtual ~ConnectionChecker() = default;

    enum class ConnType {in, out};
    virtual void addUncheckedConnection(net::PacketConnection::Ptr conn, ConnType type) = 0;

public:
    componet::Event<void (net::PacketConnection::Ptr, ProcessIdentity processId)> e_connConfirmed;

protected:
    virtual void checkConn() = 0;

private:
    bool exec() override;
};

}}

#endif
