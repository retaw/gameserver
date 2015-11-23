#ifndef PROCESS_WORLD_TASK_MSG_H
#define PROCESS_WORLD_TASK_MSG_H

#include "water/common/roledef.h"

namespace world{

class TaskMsg
{
public:
    ~TaskMsg() = default;

private:
    TaskMsg() = default;

public:
    static TaskMsg& me();

    void regMsgHandler();

private:
    void clientmsg_RequestAcceptTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_NotifyArriveTaskTargetNpc(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestSubmitTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestCollect(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_FinishCollect(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
};

}

#endif
