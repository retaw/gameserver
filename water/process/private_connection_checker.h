/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-27 16:30 +0800
 *
 * Description: 
 */


#ifndef WATER_PROCESS_PRIVATE_CONNECTION_CHECKET_H 
#define WATER_PROCESS_PRIVATE_CONNECTION_CHECKET_H 

#include "connection_checker.h"
#include "process_id.h"

#include <mutex>
#include <list>
#include <set>
#include <unordered_map>

namespace water{
namespace process{


class PrivateConnectionChecker : public ConnectionChecker
{
public:
    TYPEDEF_PTR(PrivateConnectionChecker)
    CREATE_FUN_MAKE(PrivateConnectionChecker)

    PrivateConnectionChecker(ProcessIdentity processId);
    ~PrivateConnectionChecker() = default;

    void addUncheckedConnection(net::PacketConnection::Ptr conn, ConnType type) override;

private:
    void checkConn() override;

private:
    std::mutex m_mutex;
    using LockGuard = std::lock_guard<std::mutex>;

    const ProcessIdentity m_processId;

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

    std::list<ConnInfo> m_conns;
};

}}

#endif
