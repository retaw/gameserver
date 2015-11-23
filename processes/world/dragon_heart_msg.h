#ifndef PROCESS_WORLD_DRAGON_HEART_MSG_H
#define PROCESS_WORLD_DRAGON_HEART_MSG_H

#include "water/common/roledef.h"

namespace world{

class DragonHeartMsg
{
public:
    ~DragonHeartMsg() = default;

private:
    DragonHeartMsg() = default;

public:
    static DragonHeartMsg& me();

    void regMsgHandler();

private:
    void clientmsg_RequestDragonSkills(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestUpgradeDragonSkill(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestAddEnerge(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
};

}

#endif
