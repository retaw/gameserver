#include "super.h"

#include "zone_manager.h"

#include "water/process/tcp_message.h"
#include "water/componet/logger.h"
#include "water/componet/scope_guard.h"
#include "water/net/endpoint.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace super{

using protocol::rawmsg::RawmsgManager;


Super* Super::m_me = nullptr;

Super& Super::me()
{
    return *m_me;
}

void Super::init(int32_t num, const std::string& configDir, const std::string& logDir)
{
    m_me = new Super(num, configDir, logDir);
}

Super::Super(int32_t num, const std::string& configDir, const std::string& logDir)
: Process("super", num, configDir, logDir)
{
}

void Super::init()
{
    //先执行基类初始化
    process::Process::init();

    loadConfig();
    loadDB();

    using namespace std::placeholders;

    if(m_publicNetServer == nullptr)
        EXCEPTION(componet::ExceptionBase, "无公网监听，请检查配置")

    //create checker 和 conn manager
    m_clientChecker = ClientConnectionChecker::create();
    //新的链接加入checker
    m_publicNetServer->e_newConn.reg(std::bind(&ClientConnectionChecker::addUncheckedConnection,
                                               m_clientChecker, std::placeholders::_1));
    //checker初步处理过的连接加入TcpConnManager
    m_clientChecker->e_clientConfirmed.reg(std::bind(&TcpConnectionManager::addPublicConnection, &m_conns, _1, _2));
    m_timer.regEventHandler(std::chrono::milliseconds(50), std::bind(&ClientConnectionChecker::timerExec, m_clientChecker, _1));

    //区服连接断开时的处理
    m_conns.e_afterErasePrivateConn.reg(std::bind(&ZoneManager::onZoneDisconnect, &ZoneManager::me(), _1));

    //注册消息处理事件和主定时器事件
    registerTcpMsgHandler();
    registerTimerHandler();
}

void Super::lanchThreads()
{
    Process::lanchThreads();
}

bool Super::sendToPrivate(ProcessIdentity pid, TcpMsgCode code)
{
    return sendToPrivate(pid, code, nullptr, 0);
}

bool Super::sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto)
{
    return relayToPrivate(getId().value(), pid, code, proto);
}

bool Super::relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto)
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

    const ProcessIdentity routerId(pid.zoneId(), "router", 1);
    return m_conns.sendPacketToPrivate(routerId, packet);
}

bool Super::sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size)
{
    return relayToPrivate(getId().value(), pid, code, raw, size);
}

bool Super::relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size)
{
    const uint32_t bufSize = sizeof(Envelope) + size;
    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf);

    Envelope* envelope  = new(buf) Envelope(code);
    envelope->targetPid = pid.value();
    envelope->sourceId  = sourceId;
    std::memcpy(envelope->msg.data, raw, size);

    TcpPacket::Ptr packet = TcpPacket::create();
    packet->setContent(buf, bufSize);

    const ProcessIdentity routerId(pid.zoneId(), "router", 1);
    return m_conns.sendPacketToPrivate(routerId, packet);
}

bool Super::sendToClient(LoginId loginId, TcpMsgCode code, const void* raw, uint32_t size)
{
    const uint32_t bufSize = sizeof(TcpMsg) + size;
    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf);

    TcpMsg* msg = new(buf) TcpMsg(code);
    std::memcpy(msg->data, raw, size);

    TcpPacket::Ptr packet = TcpPacket::create();
    packet->setContent(buf, bufSize);

    return m_conns.sendPacketToPublic(loginId, packet);
}

bool Super::sendToClient(LoginId loginId, TcpMsgCode code, const ProtoMsg& proto)
{
    const uint32_t protoBinSize = proto.ByteSize();
    const uint32_t bufSize = sizeof(TcpMsg) + protoBinSize;
    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf);

    TcpMsg* msg = new(buf) TcpMsg(code);
    if(!proto.SerializeToArray(msg->data, protoBinSize))
    {
        LOG_ERROR("proto serialize failed, msgCode = {}", code);
        return false;
    }

    TcpPacket::Ptr packet = TcpPacket::create();
    packet->setContent(buf, bufSize);

    return m_conns.sendPacketToPublic(loginId, packet);
}

net::PacketConnection::Ptr Super::eraseClientConn(LoginId loginId)
{
    return m_conns.erasePublicConnection(loginId);
}

void Super::tcpPacketHandle(TcpPacket::Ptr packet, 
                               TcpConnectionManager::ConnectionHolder::Ptr conn,
                               const componet::TimePoint& now)
{
    if(packet == nullptr || packet->contentSize() < sizeof(water::process::TcpMsg))
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

