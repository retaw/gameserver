#ifndef PROCESS_WORLD_TRIGGER_CFG_H
#define PROCESS_WORLD_TRIGGER_CFG_H

#include "water/componet/class_helper.h"
#include <unordered_map>

namespace world{

#define DYNAMIC_BLOCK 2001

enum class TriggerType : uint8_t
{
    door        = 1, //传送门
    block       = 2, //动态阻挡
    box         = 3, //世界boss宝箱
	bonfire		= 4, //篝火	
};

struct TriggerTpl
{
    TYPEDEF_PTR(TriggerTpl)
    CREATE_FUN_MAKE(TriggerTpl)

    uint32_t    id;
    TriggerType type;
    uint16_t    lifetime;   //生命周期(秒, 0:表示一直存在)
    union
    {
        uint16_t    param1;
        uint16_t    mapId;
		uint16_t	radius;	
    };

    union
    {
        uint16_t    param2;
        uint16_t    posx;
    };

    union
    {
        uint16_t    param3;
        uint16_t    posy;
    };
};


class TriggerCfg
{
private:
    TriggerCfg();

public:
    ~TriggerCfg() = default;

public:
    static TriggerCfg& me();
    void loadConfig(const std::string& cfgDir);

    TriggerTpl::Ptr getById(uint32_t id) const;

private:
    std::unordered_map<uint32_t, TriggerTpl::Ptr> m_triggerTpls;
};

}


#endif
