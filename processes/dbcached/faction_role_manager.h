#ifndef PROCESS_DBCACHED_FACTION_ROLE_MANAGER_HPP
#define PROCESS_DBCACHED_FACTION_ROLE_MANAGER_HPP

#include "role_manager.h"

namespace dbcached{

class FactionRoleManager
{
    friend class RoleManager; 
public:
    ~FactionRoleManager() = default;
    static FactionRoleManager& me();

    void regMsgHandler();

private:
    FactionRoleManager() = default;

    //同步角色的帮派信息，不用入库只同步缓存即可
    void servermsg_UpdateFaction(const uint8_t* msgData, uint32_t msgSize);

public:
    void fillRoleFactionInfo(Role::Ptr role);

};

}
#endif
