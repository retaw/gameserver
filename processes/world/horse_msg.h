#ifndef PROCESS_WORLD_HORSE_MSG_H
#define PROCESS_WORLD_HORSE_MSG_H

#include "water/common/roledef.h"

namespace world{

class HorseMsg
{
public:
    ~HorseMsg() = default;

private:
    HorseMsg() = default;

public:
    static HorseMsg& me();

    void regMsgHandler();

private:
    void clientmsg_RequestRaiseInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestRide(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestRaise(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestActiveSkins(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestHuanHuaSkin(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
};

}

#endif
