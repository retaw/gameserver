#include "proto_manager.h"


#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"

#include "private/login.codedef.h"
#include "private/super.codedef.h"
#include "public/account.codedef.h"
#include "public/client.codedef.h"

//static PublicProto::C_FastSignUp temp1;
//static PublicProto::AccountAndTokenToServer temp2;


namespace protocol{
namespace protobuf{


ProtoManager ProtoManager::m_me;

ProtoManager& ProtoManager::me()
{
    return m_me;
}

void ProtoManager::initMsgCode()
{
}

bool ProtoManager::loadConfig(const std::string& configDir)
{
    //clear
    m_protoDiscriptors.clear();
    //load from all files
    return (realyLoadConfig(configDir + "/protobuf.codedef.private.xml") &&
            realyLoadConfig(configDir + "/protobuf.codedef.public.xml"));
}

bool ProtoManager::realyLoadConfig(const std::string& configFile)
{
    using water::componet::XmlParseDoc;
    using water::componet::XmlParseNode;

    LOG_TRACE("protobuf, configfile: {}", configFile);

    XmlParseDoc doc(configFile);
    XmlParseNode root = doc.getRoot();

    if (!root)
    {
        LOG_ERROR("protobuf, configfile: {}, parse error", configFile);
        return false;
    }

    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        auto msgCode = itemNode.getAttr<TcpMsgCode>("msg_code");
        auto msgName = itemNode.getAttr<std::string>("msg_name");

        const google::protobuf::Descriptor* descriptor = 
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(msgName);
        if (nullptr == descriptor)
        {
            LOG_ERROR("protobuf, unkonwn msg, code={}, name={}", msgCode, msgName);
            continue;
        }
        m_protoDiscriptors[msgCode] = descriptor;
    }
    return true;
}

ProtoMsgPtr ProtoManager::create(const TcpMsgCode code)
{
    auto iter = m_protoDiscriptors.find(code);
    if (iter == m_protoDiscriptors.end())
    {
        return nullptr;
    }
    const google::protobuf::Descriptor* descriptor = iter->second;
    if (descriptor)
    {
        const ProtoMsg* prototype =
        google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype)
        {
            ProtoMsgPtr ProtoManager(prototype->New());
            return ProtoManager;
        }
    }
    return nullptr;
}

//定时检查，回调注册是否过期
//void ProtoManager::timerExec(TimePoint now)
//{
//    std::uint32_t utNow = componet::toUnixTime(now);
//
//    {//加锁，检查各个handler的过期情况
//        std::lock_guard<componet::Spinlock> lock(m_lock);
//        for(auto it = m_msgHandlers.begin(); it != m_msgHandlers.end(); )
//        {
//            if(it->first >> utNow) //再后面的肯定都是没有过期的，不用再检查了
//                break;
//
//            for(auto& item : it->second)
//                expriedHandlers.push_back(item.second);
//
//            it = m_msgHandlers.erase(it); //i love c++11, ^_^
//        }
//    }
//
//    //通知过期
//    for(auto& handler : expriedHandlers)
//        handler(nullptr, now);
//        
//}
    
//注册处理msg的handler
void ProtoManager::regHandler(TcpMsgCode code, ProtoMsgHandler handler)
{
    m_handlers[code] = handler;
}

//uint64_t ProtoManager::regAsyncAutoHandler(ProtoMsgHandler handler, uint32_t beforeExpiry)
//{
//}
//
//void ProtoManager::unregAsyncAutoHandler(uint64_t asyncCode)
//{
//}


void ProtoManager::dealTcpMsg(const TcpMsg* recv, uint32_t recvSize, uint64_t senderId, TimePoint now)
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

    ProtoMsgPtr proto = create(msgCode);
    if(proto == nullptr)
    {
        LOG_DEBUG("protoManager, illegal msgCode {}", msgCode);
        return;
    }

//    LOG_DEBUG("protoManager dispatche msg, code={}, protoSize={}", msgCode, msgDataSize);

    auto iterToHandler = m_handlers.find(msgCode);
    if(iterToHandler == m_handlers.end())
    {   
        LOG_ERROR("handle msg error, missing protoMsg handler, code = {}", msgCode);
        return;
    }
    proto->ParseFromArray(msgData, msgDataSize);
    iterToHandler->second(proto, theSenderId, now);
    return;
}

}}
