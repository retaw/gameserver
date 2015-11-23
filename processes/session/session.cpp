#include "session.h"

#include "water/process/tcp_message.h"
#include "water/componet/logger.h"
#include "water/componet/scope_guard.h"
#include "water/net/endpoint.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace session{

using protocol::rawmsg::RawmsgManager;


Session* Session::m_me = nullptr;

Session& Session::me()
{
    return *m_me;
}

void Session::init(int32_t num, const std::string& configDir, const std::string& logDir)
{
    m_me = new Session(num, configDir, logDir);
}

Session::Session(int32_t num, const std::string& configDir, const std::string& logDir)
: Process("session", num, configDir, logDir)
{
}

void Session::init()
{
    //先执行基类初始化
    process::Process::init();

    using namespace std::placeholders;

    loadConfig();

    //注册消息处理事件和主定时器事件
    registerTcpMsgHandler();
    registerTimerHandler();
}

bool Session::sendToPrivate(ProcessIdentity pid, TcpMsgCode code)
{
    return sendToPrivate(pid, code, nullptr, 0);
}

bool Session::sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto)
{
    return relayToPrivate(getId().value(), pid, code, proto);
}

bool Session::relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto)
{
    const uint32_t protoBinSize = proto.ByteSize();
    const uint32_t bufSize = sizeof(Envelope) + protoBinSize;
    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf);

    Envelope* envelope = new(buf) Envelope(code);
    envelope->targetPid  = pid.value();
    envelope->sourceId = sourceId;

    if(!proto.SerializeToArray(envelope->msg.data, protoBinSize))
    {
        LOG_ERROR("proto serialize failed, msgCode = {}", code);
        return false;
    }

    TcpPacket::Ptr packet = TcpPacket::create();
    packet->setContent(buf, bufSize);

    const ProcessIdentity routerId("router", 1);
    return m_conns.sendPacketToPrivate(routerId, packet);
}

bool Session::sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size)
{
    return relayToPrivate(getId().value(), pid, code, raw, size);
}

bool Session::relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size)
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

void Session::broadcastToWorlds(TcpMsgCode code, const void* raw, uint32_t size)
{
    //num == 0 表示发广播
    ProcessIdentity receiver("world", 0);
    sendToPrivate(receiver, code, raw, size);
}

void Session::tcpPacketHandle(TcpPacket::Ptr packet, 
                               TcpConnectionManager::ConnectionHolder::Ptr conn,
                               const componet::TimePoint& now)
{
    if(packet == nullptr)
        return;

//    LOG_DEBUG("tcpPacketHandle, packetsize()={}, contentSize={}", 
//              packet->size(), packet->contentSize());

    auto tcpMsg = reinterpret_cast<water::process::TcpMsg*>(packet->content());
    if(water::process::isRawMsgCode(tcpMsg->code))
        RawmsgManager::me().dealTcpMsg(tcpMsg, packet->contentSize(), conn->id, now);
    else if(water::process::isProtobufMsgCode(tcpMsg->code))
        ProtoManager::me().dealTcpMsg(tcpMsg, packet->contentSize(), conn->id, now);
}

}

