#ifndef PROCESSES_FUNC_BOSS_HOME_MANAGER_H
#define PROCESSES_FUNC_BOSS_HOME_MANAGER_H

#include "role_manager.h"

namespace func{

struct BossHomeTpl
{
    TYPEDEF_PTR(BossHomeTpl)
    CREATE_FUN_NEW(BossHomeTpl)

    uint32_t bossId;
    uint32_t npcId;
    MapId mapId;

    uint32_t refreshTime;
    uint32_t deadTime;
};

class BossHomeManager
{
public:
    ~BossHomeManager() = default;

    void regMsgHandler();
    static BossHomeManager& me();

    void loadConfig(const std::string& cfgdir);

private:
    BossHomeManager() = default;

    void clientmsg_BossHome(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void servermsg_BossHomeToDie(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_RefreshBossHome(const uint8_t* msgData, uint32_t msgSize);
    //void servermsg_BossHomeInCurrentScene(const uint8_t* msgData, uint32_t msgSize);

    uint32_t getRefreshTime(BossHomeTpl::Ptr bossHomeTpl);
private:
    std::map<uint32_t, BossHomeTpl::Ptr> m_bossHome;        //<bossId, bossTpl>

};


}


#endif
