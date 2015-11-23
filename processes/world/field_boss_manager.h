#ifndef PROCESSES_WORLD_FIELD_BOSS_MANAGER_H
#define PROCESSES_WORLD_FIELD_BOSS_MANAGER_H

#include "role_manager.h"

namespace world{

struct FieldBossTpl
{
    TYPEDEF_PTR(FieldBossTpl)
    CREATE_FUN_NEW(FieldBossTpl)

    uint32_t bossId;
    uint32_t npcId;
    MapId   npcMapId;
    MapId   transferMapId;
    uint16_t    posx;
    uint16_t    posy;

    //uint32_t refreshTime;
    //uint32_t deadTime;
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

    void clientmsg_TransFerToFieldBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

public:
    void toDie(const uint32_t npcTplId, const MapId mapId, const std::vector<uint32_t>& objIdVec, Role::Ptr role);
    void refreshFieldBoss(const uint32_t npcTplId, const uint32_t mapId);

private:
    std::unordered_map<uint32_t, FieldBossTpl::Ptr> m_fieldBoss;
    std::map<std::pair<uint32_t, uint32_t>, uint32_t>  m_npcTplIdMapId2BossId;  //<<npctplId, mapId>, bossId>
};

}
#endif
