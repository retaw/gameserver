/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-14 17:35 +0800
 *
 * Modified: 2015-03-14 17:35 +0800
 *
 * Description: 数据库缓冲进程
 */

#ifndef DBCACHED_DBCACHED_H
#define DBCACHED_DBCACHED_H


#include "water/process/process.h"
#include "protocol/protobuf/proto_manager.h"


namespace dbcached{

using namespace water;
using namespace process;

using protocol::protobuf::ProtoMsg;
using protocol::protobuf::ProtoMsgPtr;
using protocol::protobuf::ProtoManager;
using protocol::protobuf::ProtoMsgHandler;

class DbCached : public process::Process
{
public:
    bool sendToPrivate(ProcessIdentity pid, TcpMsgCode code);
    bool sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto);
    bool sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size);

    bool relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto);
    bool relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size);

private:
    DbCached(int32_t num, const std::string& configDir, const std::string& logDir);

    void tcpPacketHandle(TcpPacket::Ptr packet, 
                         TcpConnectionManager::ConnectionHolder::Ptr conn,
                         const componet::TimePoint& now) override;
    void init() override;
    void registerTcpMsgHandler();
    void registerTimerHandler();

public:
    static void init(int32_t num, const std::string& configDir, const std::string& logDir);
    static DbCached& me();
private:
    static DbCached* m_me;
};

}

#endif

