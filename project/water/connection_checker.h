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
#include "net/packet_connection.h"
#include "process_id.h"
#include "process_thread.h"

#include <mutex>
#include <list>
#include <set>
#include <unordered_map>

namespace water{


class ConnectionChecker : public ProcessThread
{

public:
    ConnectionChecker(ProcessIdentity processId);
    ~ConnectionChecker() = default;

    void setPrivateWhiteList(const std::set<ProcessIdentity>& acceptWhiteList);

    enum class ConnType {in, out};
    void addUncheckedPrivateConnection(net::PacketConnection::Ptr conn, ConnType type);
    void addUncheckedPublicConnection(net::PacketConnection::Ptr conn, ConnType type);

public:
    componet::Event<void (net::PacketConnection::Ptr, ProcessIdentity processId)> e_connConfirmed;;

private:
    bool exec() override;
    void checkPrivateConn();
    void checkPublicConn();

private:
    std::mutex m_mutex;
    using LockGuard = std::lock_guard<std::mutex>;

    const ProcessIdentity m_processId;

    //接入白名单，为空时表示不过滤
    std::set<ProcessIdentity> m_acceptWhiteList;

    //链接状态, check函数是个状态机
    enum class ConnState 
    {
        recvId,  //in,  等待对方发送身份信息
        sendRet, //in,  发送验证结果给对方
        sendId,  //out, 发送自己的信息给验证方
        recvRet, //out, 接收验证结果
    };
    struct ConnInfo
    {
        ConnState state;
        net::PacketConnection::Ptr conn;
        ProcessIdentity remoteId;
    };
    std::list<ConnInfo> m_privateConns;
    std::list<ConnInfo> m_publicConns;
};

}
#endif
