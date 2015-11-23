#include "test.h"

#include "super.h"

#include "water/componet/logger.h"

#include "water/process/process_id.h"

//#include "protocol/protobuf/public/account.codedef.h"
//#include "protocol/protobuf/public/client.codedef.h"

namespace super{

//static PublicProto::C_FastSignUp temp1;
//static PublicProto::C_FastSignUp temp1;

void testMsgSend()
{
    auto proto1 = ProtoManager::me().create(268435478);
    if(proto1 == nullptr)
    {
        LOG_DEBUG("测试, create {} 失败", 268435478);
    }

    auto proto2 = ProtoManager::me().create(268435458);
    if(proto2 == nullptr)
    {
        LOG_DEBUG("测试, create {} 失败", 268435458);
    }
    else
    {
        LOG_DEBUG("测试, create {} 成功", 268435458);
    }
    return;
}

}
