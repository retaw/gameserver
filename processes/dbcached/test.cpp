#include "test.h"

#include "water/componet/logger.h"
#include "water/componet/datetime.h"
#include "protocol/protobuf/proto_manager.h"
#include "protocol/protobuf/protomsg_def.h"
#include "water/process/process_id.h"

#include "protocol/rawmsg/private/login.h"
#include "protocol/rawmsg/private/login.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace dbcached{

void proto_GetDbNum(const ProtoMsgPtr& rev, ProcessIdentity remotePid, const componet::TimePoint& now)
{
    static uint32_t num = 120;

    PrivateProto::RetDbNum send;
    send.set_num(num++);
    send.set_msg("hallo");
    const bool ret = DbCached::me().sendToPrivate(remotePid, PROTO_CODE_PRIVATE(RetDbNum), send);
    LOG_DEBUG("send proto to {}, {}", remotePid, ret ? "ok" : "falied");
}

void raw_TestMsg(const uint8_t* msgData, uint32_t msgSize, uint64_t connId, const componet::TimePoint& now)
{
    ProcessIdentity remoteId(connId);
    auto rev = reinterpret_cast<const PrivateRaw::TestMsg*>(msgData);
    LOG_DEBUG("recv TestMsg from {}, rev num={}", remoteId, rev->num);
    PrivateRaw::TestMsg send;
    send.num = rev->num * 100;
    const bool ret = DbCached::me().sendToPrivate(remoteId, RAWMSG_CODE_PRIVATE(TestMsg), &send, sizeof(send));
    LOG_DEBUG("return TestMsg to {}, {}, send num={}", remoteId, ret ? "ok" : "falied", send.num);
}

/******************************/

void test2SecsTimerHandler(const componet::TimePoint& now)
{
}

}
