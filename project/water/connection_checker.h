/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-27 16:30 +0800
 *
 * Description: 
 */


#ifndef WATER_CONNECTION_CHECKET_H 
#define WATER_CONNECTION_CHECKET_H 

#include "componet/event.h"
#include "net/connection.h"
#include "process_id.h"
#include "process_thread.h"

#include <mutex>
#include <list>
#include <unordered_map>

namespace water{


class ConnectionChecker : public ProcessThread
{

public:
    ConnectionChecker(const std::set<ProcessIdentity>& acceptWhiteList);
    ~ConnectionChecker() = default;

    enum class ConnType {in, out};
    void addUncheckedPrivateConnection(PacketConnection::Ptr conn, ConnType type);
    void addUncheckedPublicConnection(componet::TcpConnection::Ptr conn, ConnType type);

public:
    Event<void (ConnectionChecker*, componet::TcpConnection::Ptr)> e_checkDone;

private:
    bool exec() override;
    void checkPrivateConn();
    void checkPublicConn();

private:
    std::mutex m_mutex;
    using LockGuard = std::lock_guard<std::mutex>;

    std::set<ProcessIdentity> m_acceptWhiteList;

    enum class ConnState {newIn, newOut, sendId, ok, close};
    struct ConnInfo
    {
        ConnState state;
        PacketConnection::Ptr conn;
    };
    std::list<ConnInfo> m_privateConns;
    std::list<ConnInfo> m_publicConns;
};

}
#endif
