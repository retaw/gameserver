/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-14 16:12 +0800
 *
 * Modified: 2015-03-14 16:12 +0800
 *
 * Description: 一些功能测试代码
 */

#ifndef PROCESS_DBCACHED_TEST_H
#define PROCESS_DBCACHED_TEST_H

#include "dbcached.h"


namespace dbcached{

using protocol::protobuf::ProtoMsgPtr;
using water::process::ProcessIdentity;


/****************proto msg handler******************/
void proto_GetDbNum(const ProtoMsgPtr& rev, ProcessIdentity remotePid, const componet::TimePoint& now);

void raw_TestMsg(const uint8_t* msgData, uint32_t msgSize, uint64_t connId, const componet::TimePoint& now);

/*****************timer*****************/
void test2SecsTimerHandler(const componet::TimePoint& now);



}

#endif
