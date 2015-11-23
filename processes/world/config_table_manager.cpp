#include "config_table_manager.h"

namespace world{


ConfigTable<SkillBase> skillCT = ConfigTable<SkillBase>::me();
ConfigTable<SkillStrengthenBase> skillStrengthenCT = ConfigTable<SkillStrengthenBase>::me();
ConfigTable<SkillEffectBase> skillEffectCT = ConfigTable<SkillEffectBase>::me();
ConfigTable<BuffBase> buffCT = ConfigTable<BuffBase>::me();
ConfigTable<LevelPropsBase> levelPropCT = ConfigTable<LevelPropsBase>::me();
ConfigTable<KValueBase> kCT = ConfigTable<KValueBase>::me();


ConfigTableMgr& ConfigTableMgr::me()
{
    static ConfigTableMgr me;
    return me;
}


void ConfigTableMgr::loadAllCfgTable(const std::string& cfgdir)
{
    levelPropCT.loadCfgTable(cfgdir + "/level_props.xml");
    skillCT.loadCfgTable(cfgdir + "/skill.xml");
    skillEffectCT.loadCfgTable(cfgdir + "/skill_effect.xml");
    skillStrengthenCT.loadCfgTable(cfgdir + "/skill_strengthen.xml");
    kCT.loadCfgTable(cfgdir + "/k_value.xml");
    buffCT.loadCfgTable(cfgdir + "/buff.xml");
}

}
