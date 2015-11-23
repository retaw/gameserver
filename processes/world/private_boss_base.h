#ifndef PROCESSES_WORLD_PRIVATE_BOSS_BASE_H
#define PROCESSES_WORLD_PRIVATE_BOSS_BASE_H

#include "water/common/roledef.h"
 #include "water/common/scenedef.h"
#include "water/componet/class_helper.h"
#include <unordered_map>
#include <map>

namespace world{

struct PrivateBossTpl
{
    struct Reward
    {
        uint32_t objTplId;
        uint16_t objnum;
        Bind bind;
    };

    TYPEDEF_PTR(PrivateBossTpl)
    CREATE_FUN_NEW(PrivateBossTpl)
    uint32_t bossId;
    uint32_t npcId;
    MapId   npcMapId;
    MapId   transferMapId;
    uint16_t    posx;
    uint16_t    posy;
    std::vector<uint16_t> enterTimes;   //每天可进入次数,下标为vip等级
    std::vector<Reward> reward; //奖励
    uint32_t duration;
};

class PrivateBossBase
{
public:
    ~PrivateBossBase() = default;

    static PrivateBossBase& me();
    void loadConfig(const std::string& cfgdir);
    uint32_t getKeepDuration(uint32_t bossId);

private:
    PrivateBossBase() = default;

public:
    std::unordered_map<uint32_t, PrivateBossTpl::Ptr> m_privateBoss;    //<个人bosstplId, tpl>
    std::map<std::pair<uint32_t, uint32_t>, uint32_t>  m_npcTplIdMapId2BossId;  //<<npctplId, mapId>, bossId>
};

}

#endif
