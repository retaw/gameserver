#ifndef PROCESS_WORLD_STALL_MSG_H
#define PROCESS_WORLD_STALL_MSG_H

#include "water/common/roledef.h"

namespace world{

class StallMsg
{
public:
    static StallMsg& me();

    void regMsgHandler();

private:
    void servermsg_DBRetStallLog(const uint8_t* msgData, uint32_t size);

    void clientmsg_ReqStallSellLog(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqOpenStall(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqCloseStall(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqBuyStallObj(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqWatchOthersStall(const uint8_t* msgData, uint32_t size, RoleId roleId);
};

}

#endif

