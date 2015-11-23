/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-17 20:52 +0800
 *
 * Modified: 2015-03-17 20:52 +0800
 *
 * Description: 消息号和消息处理器的映射
 *
 */


#ifndef PROTOCOL_RAWMSG_RAWMSG_MANAGER_H
#define PROTOCOL_RAWMSG_RAWMSG_MANAGER_H

#include "water/componet/exception.h"
#include "water/process/tcp_message.h"
#include "water/componet/datetime.h"
#include "water/process/process_id.h"

#include <memory>
#include <unordered_map>

namespace protocol{
namespace rawmsg{

/*****************************************/


typedef water::process::TcpMsgCode TcpMsgCode;
typedef water::process::TcpMsg TcpMsg;
typedef water::componet::TimePoint TimePoint;

typedef std::function<void (const uint8_t*, uint32_t, uint64_t, const TimePoint&)> RawmsgHandler;

class RawmsgManager
{
public:
    //被Process::tcpPacketHandle 调用的函数
    //tcpPacketHandle内部，先判断packet中放的是否是proto协议，是的话，调用此函数来处理
    void dealTcpMsg(const TcpMsg* recv, uint32_t recvSize, uint64_t senderId, TimePoint now); 

    //定时检查，回调注册是否过期
    void timerExec(TimePoint now);
    
    //注册处理msg的handler
    void regHandler(TcpMsgCode code, RawmsgHandler handler);

//    uint64_t regAsyncAutoHandler(RawmsgHandler handler, uint32_t beforeExpiry);
//    void unregAsyncAutoHandler(uint64_t asyncCode);


private:
    //water::componet::Spinlock m_lock;
    
    //std::map<uint32_t, std::unordered_map<uint32_t, RawmsgHandler>> m_callbackHandlers;

    std::unordered_map<TcpMsgCode, RawmsgHandler> m_handlers; 

public:
    static RawmsgManager& me();

private:
    static RawmsgManager m_me;

};

}}


#ifndef RAWMSG_CODE_PUBLIC
#define RAWMSG_CODE_PUBLIC(msgName) (PublicRaw::code##msgName)
#endif

#ifndef RAWMSG_CODE_PRIVATE
#define RAWMSG_CODE_PRIVATE(msgName) (PrivateRaw::code##msgName)
#endif


#define REG_RAWMSG_PUBLIC(raw, handler) \
protocol::rawmsg::RawmsgManager::me().regHandler(RAWMSG_CODE_PUBLIC(raw), handler);

#define REG_RAWMSG_PRIVATE(raw, handler) \
protocol::rawmsg::RawmsgManager::me().regHandler(RAWMSG_CODE_PRIVATE(raw), handler);

#endif
