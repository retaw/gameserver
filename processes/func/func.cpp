#include "func.h"

#include "water/process/tcp_message.h"
#include "water/componet/logger.h"
#include "water/componet/scope_guard.h"
#include "water/dbadaptcher/dbconnection_pool.h"
#include "protocol/protobuf/proto_manager.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "friend_manager.h"
#include "faction_manager.h"


namespace func{

using protocol::rawmsg::RawmsgManager;
using protocol::protobuf::ProtoManager;

Func* Func::m_me = nullptr;

Func& Func::me()
{
    return *m_me;
}

void Func::init(int32_t num, const std::string& configDir, const std::string& logDir)
{
    m_me = new Func(num, configDir, logDir);
}

Func::Func(int32_t num, const std::string& configDir, const std::string& logDir)
: Process("func", num, configDir, logDir)
{
}

void Func::init()
{
    dbadaptcher::MysqlConnectionPool::me().init("config/process.xml");
    process::Process::init();

    loadConfigAndDB();
    registerTcpMsgHandler();
    registerTimerHandler();
}

void Func::stop()
{
	//进程停止, 执行保存数据操作
	//
	
	Process::stop();
}

bool Func::sendToPrivate(ProcessIdentity pid, TcpMsgCode code)
{
    return sendToPrivate(pid, code, nullptr, 0);
}

bool Func::sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size)
{
    return relayToPrivate(getId().value(), pid, code, raw, size);
}

bool Func::relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size)
{
    const uint32_t bufSize = sizeof(Envelope) + size;
    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf);

    Envelope* envelope = new(buf) Envelope(code);
    envelope->targetPid  = pid.value();
    envelope->sourceId = sourceId;
    std::memcpy(envelope->msg.data, raw, size);

    TcpPacket::Ptr packet = TcpPacket::create();
    packet->setContent(buf, bufSize);

//    LOG_DEBUG("sendToPrivate, rawSize={}, tcpMsgSize={}, packetSize={}, contentSize={}", 
//              size, bufSize, packet->size(), *(uint32_t*)(packet->data()));

    const ProcessIdentity routerId("router", 1);
    return m_conns.sendPacketToPrivate(routerId, packet);
}

void Func::broadcastToWorlds(TcpMsgCode code, const void* raw, uint32_t size)
{
	//编号0表示发广播, 发到所有world类型进程
	ProcessIdentity receiver("world", 0); 
	sendToPrivate(receiver, code, raw, size);
}

void Func::tcpPacketHandle(TcpPacket::Ptr packet, 
                               TcpConnectionManager::ConnectionHolder::Ptr conn,
                               const componet::TimePoint& now)
{
    if(packet == nullptr)
        return;


    auto tcpMsg = reinterpret_cast<water::process::TcpMsg*>(packet->content());
    if(water::process::isRawMsgCode(tcpMsg->code))
        RawmsgManager::me().dealTcpMsg(tcpMsg, packet->contentSize(), conn->id, now);
    else if(water::process::isProtobufMsgCode(tcpMsg->code))
        ProtoManager::me().dealTcpMsg(tcpMsg, packet->contentSize(), conn->id, now);
}

}
