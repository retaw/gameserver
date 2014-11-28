#ifndef MSG_MSG_H
#define MSG_MSG_H
#include "water/net/packet.h"
#include "water/process_id.h"

#pragma pack(1)

namespace water{
namespace msg{


enum class MsgCode : uint16_t
{
    ProcessIdentityNum,
};


struct MsgHeader
{
    MsgCode code;
};

#define MSG_CODE(MsgName, msgCode) \
MsgName():MsgHeader(msgCode) {}

struct ProcessInendityNum : MsgHeader
{
    MSG_CODE(TestMsg, MsgCode::ProcessIdentityNum);
    int32_t processNum;
};



}}

#pragma pack()


#endif
