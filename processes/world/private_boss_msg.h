#ifndef PROCESSES_WORLD_PRIVATE_BOSS_MSG_H
#define PROCESSES_WORLD_PRIVATE_BOSS_MSG_H

#include "private_boss.h"
#include "role_manager.h"


namespace world{

class Role;
class PrivateBossMsg
{
public:
    ~PrivateBossMsg() = default;
    static PrivateBossMsg& me();
    void regMsgHandler();

private:
    PrivateBossMsg() = default;

    void clientmsg_PrivateBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_TransFerToPrivateBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_LeavePrivateBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ReqRefreshPrivateBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

};


}
#endif
