/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-06 17:23 +0800
 *
 * Modified: 2015-03-06 17:23 +0800
 *
 * Description:  Tcp消息结构定义
 */


#ifndef WATER_PROCESS_MESSAGE_H
#define WATER_PROCESS_MESSAGE_H

#include <cstdint>
#include <cstring>

#include "process_id.h"

namespace water{
namespace process{


using TcpMsgCode = uint32_t;

/****************消息号的结构*****************/
//最高1bit表示可访问性,  第2bit表示是否为转发, 第3-4bits为协议类型
//把对外的raw和public位都留成0, 这样自动累加消息号时端感觉不出来有协议和可见性区分
//0为特殊消息号, 代表Envelop, 用与转发消息
struct MsgCodeTypeMask
{
    static const TcpMsgCode accessType       = 0x80000000;  //只看最高1bit, 可访问性
    static const TcpMsgCode accessPublic     = 0x00000000;  //0表示public
    static const TcpMsgCode accessPrivate    = 0x80000000;  //1表示purivate

    static const TcpMsgCode isEnvelopeTpye   = 0x40000000; //高2bit, 信封标志
    static const TcpMsgCode envelopeTrue     = 0x40000000; //1是信封
    static const TcpMsgCode envelopeFalse    = 0x00000000; //0不是信封

    static const TcpMsgCode protocolType     = 0x30000000; //高3-4bits, 能有8种表示，足够了……
    static const TcpMsgCode protocolRaw      = 0x00000000; //0表示raw
    static const TcpMsgCode protocolProtobuf = 0x10000000; //1表示googleprotobuf
};

const TcpMsgCode minPublicRawMsgCode = MsgCodeTypeMask::protocolRaw | MsgCodeTypeMask::accessPublic;
const TcpMsgCode minPrivateRawMsgCode = MsgCodeTypeMask::protocolRaw | MsgCodeTypeMask::accessPrivate;

const TcpMsgCode minPublicProtoMsgCode = MsgCodeTypeMask::protocolProtobuf | MsgCodeTypeMask::accessPublic;
const TcpMsgCode mminPrivateProtoMsgCode = MsgCodeTypeMask::protocolProtobuf |  MsgCodeTypeMask::accessPrivate;

/********************消息号的判定*************************/
inline bool isEnvelopeMsgCode(TcpMsgCode code)
{
    return (code & MsgCodeTypeMask::isEnvelopeTpye) == MsgCodeTypeMask::envelopeTrue;
}

inline bool isPrivateMsgCode(TcpMsgCode code)
{
    return (code & MsgCodeTypeMask::accessType) == MsgCodeTypeMask::accessPrivate;
}

inline bool isPublicMsgCode(TcpMsgCode code)
{
    return (code & MsgCodeTypeMask::accessType) == MsgCodeTypeMask::accessPublic;
}

inline bool isRawMsgCode(TcpMsgCode code)
{
    return (code & MsgCodeTypeMask::protocolType) == MsgCodeTypeMask::protocolRaw;
}

inline bool isProtobufMsgCode(TcpMsgCode code)
{
    return (code & MsgCodeTypeMask::protocolType) == MsgCodeTypeMask::protocolProtobuf;
}


/*********************消息结构的定义*********************/
#pragma pack(1)


struct TcpMsg
{
    TcpMsg() = default;
    TcpMsg(TcpMsgCode code_) : code(code_) {}

    TcpMsgCode code;       //由消息号生成机制保证privateCode <= 1000;
    uint8_t    data[0];     //实际的消息
};
struct Envelope : public TcpMsg
{
    Envelope(TcpMsgCode code)
    : TcpMsg(MsgCodeTypeMask::envelopeTrue | 
             MsgCodeTypeMask::accessPrivate |  //信封消息一律为内部消息
             (code & MsgCodeTypeMask::protocolType)) //信封的协议类型, 同其携带的消息
      , targetPid(0)
      , msg(code)
    {
    }

    void fill(const void* buf, uint32_t size)
    {
        if(buf != nullptr && size > 0)
            std::memcpy(data, buf, size);
    }

    ProcessIdentityValue    targetPid;   //对方的进程Id
    uint64_t   sourceId;    //实际的msg来源Id
    TcpMsg     msg;
};
#pragma pack()

}}


#endif

