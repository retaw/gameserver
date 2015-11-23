/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  game server 的进程定义
 */

#ifndef PROCESS_GATEWAY_GATEWAY_H
#define PROCESS_GATEWAY_GATEWAY_H


#include "water/process/process.h"
#include "protocol/protobuf/proto_manager.h"

namespace session{

using namespace water;
using namespace process;

using protocol::protobuf::ProtoMsg;
using protocol::protobuf::ProtoMsgPtr;
using protocol::protobuf::ProtoManager;
using protocol::protobuf::ProtoMsgHandler;

class Session : public process::Process
{
public:
    bool sendToPrivate(ProcessIdentity pid, TcpMsgCode code);
    bool sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto);
    bool sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size);

    bool relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto);
    bool relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size);

    void broadcastToWorlds(TcpMsgCode code, const void* raw, uint32_t size);

private:
    Session(int32_t num, const std::string& configDir, const std::string& logDir);

    void tcpPacketHandle(TcpPacket::Ptr packet, 
                         TcpConnectionManager::ConnectionHolder::Ptr conn,
                         const componet::TimePoint& now) override;

    void init() override;

    void registerTcpMsgHandler();
    void registerTimerHandler();

    void loadConfig();

public:
    static void init(int32_t num, const std::string& configDir, const std::string& logDir);
    static Session& me();
private:
    static Session* m_me;
};

}

#endif
