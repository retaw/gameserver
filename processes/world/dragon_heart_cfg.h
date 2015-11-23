#ifndef PROCESS_WORLD_DRAGON_HEART_CFG_H
#define PROCESS_WORLD_DRAGON_HEART_CFG_H

#include "pkdef.h"
#include "water/componet/class_helper.h"
#include <vector>
#include <unordered_map>

namespace world{


struct DragonHeartCfg
{
    TYPEDEF_PTR(DragonHeartCfg)
    CREATE_FUN_MAKE(DragonHeartCfg)

    uint32_t    dragonSkillId;
    uint32_t    dragonSkillLevel;
    uint16_t    costDragonSoul; //消耗的龙魂
    uint32_t    roleLevel;  //主角等级需求
    std::vector<std::pair<DragonSkillProp, uint16_t>> dragonSkillProps;
};


class DragonHeartBase
{
public:
    ~DragonHeartBase() = default;

private:
    DragonHeartBase();

public:
    void loadConfig(const std::string& cfgdir);
    static DragonHeartBase& me();
    DragonHeartCfg::Ptr getCfg(uint32_t dragonSkillId, uint32_t dragonSkillLevel) const;

private:
    std::unordered_map<uint32_t, DragonHeartCfg::Ptr> m_dragonHeartCfg;
};

}

#endif
