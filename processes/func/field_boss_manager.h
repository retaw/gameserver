#ifndef PROCESSES_FUNC_FIELD_BOSS_MANAGER_H
#define PROCESSES_FUNC_FIELD_BOSS_MANAGER_H

#include "role_manager.h"

namespace func{

struct FieldBossTpl
{
    TYPEDEF_PTR(FieldBossTpl)
    CREATE_FUN_NEW(FieldBossTpl)

    uint32_t bossId;
    //uint32_t npcId;
    //MapId   NpcMapId;
    //MapId   transferMapId;
    //uint16_t    posx;
    //uint16_t    posy;
    uint32_t refreshTime;

    uint32_t deadTime;
};

class FieldBossManager
{
public:
    ~FieldBossManager() = default;

    void regMsgHandler();
    static FieldBossManager& me();

    void loadConfig(const std::string& cfgdir); 
private:
    FieldBossManager() = default;

    void clientmsg_FieldBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void servermsg_FieldBossToDie(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_RefreshFieldBoss(const uint8_t* msgData, uint32_t msgSize);

    uint32_t getRefreshTime(FieldBossTpl::Ptr filedBossTpl);
public:

private:
    std::map<uint32_t, FieldBossTpl::Ptr> m_fieldBoss;    //<bossId, bosstpl>
};

}
#endif
