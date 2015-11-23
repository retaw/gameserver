#ifndef PROCESSES_WORLD_BOSS_HOME_MANAGER_H
#define PROCESSES_WORLD_BOSS_HOME_MANAGER_H

#include "role_manager.h"

namespace world{

struct BossHomeTpl
{
    TYPEDEF_PTR(BossHomeTpl)
    CREATE_FUN_NEW(BossHomeTpl)

    uint32_t bossId;
    uint32_t npcId;
    MapId mapId;
    MapId   transferMapId;

    uint32_t refreshTime;
    uint32_t deadTime;
};

class BossHomeManager
{
public:
    ~BossHomeManager() = default;
    static BossHomeManager& me();

    void regMsgHandler();
    void loadConfig(const std::string& cfgdir);
    void afterEnterScene(Role::Ptr role);
    void toDie(const uint32_t npcTplId, const MapId mapId, const std::vector<uint32_t>& objIdVec, Role::Ptr role);
    //boss之家的boss复活需要调用
    void refreshbossHome(const uint32_t npcTplId, MapId mapId);
private:
    BossHomeManager() = default;

    void clientmsg_TransFerToBossHome(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_TransFerToNextBossHome(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_LeaveBossHome(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    //模块内函数
    uint32_t getRefreshTime(BossHomeTpl::Ptr bossHomeTpl);
private:
    std::unordered_map<uint32_t, BossHomeTpl::Ptr> m_bossHome;
    std::map<std::pair<uint32_t, uint32_t>, uint32_t>  m_npcTplIdMapId2BossId;  //<<npctplId, mapId>, bossId>
    std::set<MapId> m_mapId;
};


}


#endif
