#include "dbcached.h"

#include "water/process/tcp_message.h"
#include "water/componet/logger.h"
#include "water/componet/scope_guard.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "water/dbadaptcher/dbconnection_pool.h"
#include "role_manager.h"
#include "randname_manager.h"

namespace dbcached{

using protocol::rawmsg::RawmsgManager;

DbCached* DbCached::m_me = nullptr;

DbCached& DbCached::me()
{
    return *m_me;
}

void DbCached::init(int32_t num, const std::string& configDir, const std::string& logDir)
{
    m_me = new DbCached(num, configDir, logDir);
}

DbCached::DbCached(int32_t num, const std::string& configDir, const std::string& logDir)
: Process("dbcached", num, configDir, logDir)
{
}

void DbCached::init()
{
    {
        //数据库加载检查,...?这里的文件路径将做修改，待处理
        dbadaptcher::MysqlConnectionPool::me().init("config/process.xml");
        RoleManager::me().init();
        RandNameManager::me().init("config/pname.xml");
    }

    Process::init();

    registerTcpMsgHandler();
    registerTimerHandler();
}

bool DbCached::sendToPrivate(ProcessIdentity pid, TcpMsgCode code)
{
    return sendToPrivate(pid, code, nullptr, 0);
}

bool DbCached::sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto)
{
    return relayToPrivate(getId().value(), pid, code, proto);
}

bool DbCached::relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const ProtoMsg& proto)
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

bool DbCached::sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size)
{
    return relayToPrivate(getId().value(), pid, code, raw, size);
}

bool DbCached::relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size)
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

void DbCached::tcpPacketHandle(TcpPacket::Ptr packet, 
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
