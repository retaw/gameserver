#include "rawmsg_manager.h"

//#include "rawmsg.codedef.h"

#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"

namespace protocol{
namespace rawmsg{


RawmsgManager RawmsgManager::m_me;

RawmsgManager& RawmsgManager::me()
{
    return m_me;
}

//定时检查，回调注册是否过期
void RawmsgManager::timerExec(TimePoint now)
{/*
    std::uint32_t utNow = componet::toUnixTime(now);

    {//加锁，检查各个handler的过期情况
        std::lock_guard<componet::Spinlock> lock(m_lock);
        for(auto it = m_msgHandlers.begin(); it != m_msgHandlers.end(); )
        {
            if(it->first >> utNow) //再后面的肯定都是没有过期的，不用再检查了
                break;

            for(auto& item : it->second)
                expriedHandlers.push_back(item.second);

            it = m_msgHandlers.erase(it); //i love c++11, ^_^
        }
    }

    //通知过期
    for(auto& handler : expriedHandlers)
        handler(nullptr, now);
        */
}
    
//注册处理msg的handler
void RawmsgManager::regHandler(TcpMsgCode code, RawmsgHandler handler)
{
    m_handlers[code] = handler;
}
/*
uint64_t RawmsgManager::regAsyncAutoHandler(RawmsgHandler handler, uint32_t beforeExpiry)
{
}

void RawmsgManager::unregAsyncAutoHandler(uint64_t asyncCode)
{
}
*/

void RawmsgManager::dealTcpMsg(const TcpMsg* recv, uint32_t recvSize, uint64_t senderId, TimePoint now)
{
    if(recvSize < sizeof(water::process::TcpMsg))
    {   
        LOG_ERROR("handle msg error, msgSize < sizeof(TcpMsg), recvSize={}, senderId={}", recvSize, senderId);
        return;
    }

    TcpMsgCode msgCode     = recv->code;;
    const uint8_t* msgData = recv->data;
    uint32_t msgDataSize   = recvSize - sizeof(water::process::TcpMsg);
    uint64_t theSenderId   = senderId;

    //如果是信封, 需要另作处理
    if(water::process::isEnvelopeMsgCode(recv->code))
    {
        auto envelope = reinterpret_cast<const water::process::Envelope*>(recv);
        if(recvSize < sizeof(water::process::Envelope))
        {   
            LOG_ERROR("handle msg error, recvSize < sizeof(Envelope), code={}", recv->code);
            return;
        }
        msgCode = envelope->msg.code;
        msgData = envelope->msg.data;
        msgDataSize  = recvSize - sizeof(water::process::Envelope);
        theSenderId  = envelope->sourceId;
    }

//    LOG_DEBUG("rawmsgManager dispatche msg, code={}, rawSize={}", msgCode, msgDataSize);

    auto iterToHandler = m_handlers.find(msgCode);
    if(iterToHandler == m_handlers.end())
    {   
        LOG_ERROR("handle msg error, missing rawmsg handler, code = {}", msgCode);
        return;
    }   

    iterToHandler->second(msgData, msgDataSize, theSenderId, now);
    return;
}

}}


