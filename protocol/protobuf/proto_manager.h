/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-17 20:52 +0800
 *
 * Modified: 2015-03-17 20:52 +0800
 *
 * Description: 1，消息号到消息创建器的映射
 *              2，消息号和消息处理器的映射
 *
 *              2，消息类型到消息号的映射 ？？？
 */


#ifndef PROTOCOL_PROTOBUF_PROTO_MANAGER_H
#define PROTOCOL_PROTOBUF_PROTO_MANAGER_H

#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"


#include "water/componet/exception.h"
#include "water/process/tcp_message.h"
#include "water/componet/datetime.h"
#include "water/process/process_id.h"

#include <memory>
#include <unordered_map>

namespace protocol{
namespace protobuf{


using water::process::TcpMsgCode;
using water::componet::TimePoint;

typedef google::protobuf::Message ProtoMsg;
typedef std::shared_ptr<ProtoMsg> ProtoMsgPtr;
typedef std::function<void (const ProtoMsgPtr& proto, uint64_t connId, const TimePoint& now)> ProtoMsgHandler;


class ProtoManager
{
typedef water::process::TcpMsg TcpMsg;
public:
    //消息号配置， 得到号到name的映射
    bool loadConfig(const std::string& configDir);
    void initMsgCode();

    //通过msgCode创建一个ProtoMsg
    ProtoMsgPtr create(TcpMsgCode code);

    //被Process::tcpPacketHandle 调用的函数
    //tcpPacketHandle内部，先判断packet中放的是否是proto协议，是的话，调用此函数来处理
    void dealTcpMsg(const TcpMsg* recv, uint32_t recvSize, uint64_t sender, TimePoint now);

    void regHandler(TcpMsgCode code, ProtoMsgHandler handler);

    //自动异步msg处理
    
    //定时检查，回调注册是否过期
//    void timerExec(TimePoint now);
//    uint64_t regAsyncAutoHandler(ProtoMsgHandler handler, uint32_t beforeExpiry);
//    void unregAsyncAutoHandler(uint64_t asyncCode);


private:
    bool realyLoadConfig(const std::string& configFile);

private:
    //water::componet::Spinlock m_lock;
    //std::map<uint32_t, std::unordered_map<uint32_t, ProtoMsgHandler>> m_callbackHandlers;

    std::unordered_map<TcpMsgCode, ProtoMsgHandler> m_handlers; 

    std::unordered_map<TcpMsgCode, const google::protobuf::Descriptor*> m_protoDiscriptors; 

public:
    static ProtoManager& me();

private:
    static ProtoManager m_me;

};

#ifndef PROTO_CODE_PUBLIC
#define PROTO_CODE_PUBLIC(protoName) (PublicProto::code##protoName)
#endif


#ifndef PROTO_CODE_PRIVATE
#define PROTO_CODE_PRIVATE(protoName) (PrivateProto::code##protoName)
#endif


#define REG_PROTO_PUBLIC(proto, handler) \
protocol::protobuf::ProtoManager::me().regHandler(PROTO_CODE_PUBLIC(proto), handler);

#define REG_PROTO_PRIVATE(proto, handler) \
protocol::protobuf::ProtoManager::me().regHandler(PROTO_CODE_PRIVATE(proto), handler);


}}

#endif
