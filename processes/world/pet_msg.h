#ifndef PROCESS_WORLD_PET_MSG_H
#define PROCESS_WORLD_PET_MSG_H

#include "water/common/roledef.h"

namespace world{

class PetMsg
{
public:
    ~PetMsg() = default;

private:
    PetMsg() = default;

public:
    static PetMsg& me();

    void regMsgHandler();

private:
    void clientmsg_PetRequestMoveToPos(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    //宠物ai相关的消息
    void clientmsg_PetAIMode(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
};

}

#endif
