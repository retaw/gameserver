/*
 *
 * pk过程中参与计算的一些系数
 *
 */

#ifndef PROCESS_WORLD_PK_CFG_HPP
#define PROCESS_WORLD_PK_CFG_HPP

#include <stdint.h>
#include <string>

namespace world{

class PKCfg final
{
private:
    PKCfg();

public:
    ~PKCfg() = default;
    static PKCfg& me();

public:
    void loadConfig(const std::string& cfgDir);

    int32_t hpParam1() const { return m_hp_param1; }
    int32_t hpParam2() const { return m_hp_param2; }
    int32_t hpParam3() const { return m_hp_param3; }

    int32_t mpParam1() const { return m_mp_param1; }
    int32_t mpParam2() const { return m_mp_param2; }
    int32_t mpParam3() const { return m_mp_param3; }

    int32_t atkParam() const { return m_atk_param; }

    int32_t pdefParam() const { return m_pdef_param; }
    int32_t mdefParam() const { return m_mdef_param; }

    int32_t shotParam1() const { return m_shot_param1; }
    int32_t shotParam2() const { return m_shot_param2; }
    int32_t shotParam3() const { return m_shot_param3; }
    int32_t shotParam4() const { return m_shot_param4; }
    int32_t shotParam5() const { return m_shot_param5; }

    int32_t critParam1() const { return m_crit_param1; }
    int32_t critParam2() const { return m_crit_param2; }
    int32_t critParam3() const { return m_crit_param3; }
    int32_t critParam4() const { return m_crit_param4; }
    int32_t critParam5() const { return m_crit_param5; }
    int32_t critParam6() const { return m_crit_param6; }

    int32_t breakParam1() const { return m_break_param1; }
    int32_t breakParam2() const  { return m_break_param2; }
    int32_t breakEffectParam1() const  { return m_break_effect_param1; }
    int32_t breakEffectParam2() const { return m_break_effect_param2; }
    int32_t breakEffectParam3() const { return m_break_effect_param3; }

    int32_t vampireParam1() const { return m_vampire_param1; }
    int32_t vampireParam2() const { return m_vampire_param2; }
    int32_t vampireEffectParam1() const { return m_vampire_effect_param1; }
    int32_t vampireEffectParam2() const { return m_vampire_effect_param2; }
    int32_t vampireEffectParam3() const { return m_vampire_effect_param3; }

    int32_t dmgaddParam1() const { return m_dmgadd_param1; }
    int32_t dmgaddParam2() const { return m_dmgadd_param2; }
    int32_t dmgaddParam3() const { return m_dmgadd_param3; }
    int32_t dmgaddParam4() const { return m_dmgadd_param4; }

    int32_t finalParam() const { return m_final_param; }
    int32_t finalWarriorParam() const { return m_final_warrior_param; }
    int32_t finalMagicParam() const { return m_final_magic_param; }
    int32_t finalWitchParam() const { return m_final_witch_param; }


private:
    int32_t m_hp_param1;
    int32_t m_hp_param2;
    int32_t m_hp_param3;

    int32_t m_mp_param1;
    int32_t m_mp_param2;
    int32_t m_mp_param3;

    int32_t m_atk_param;

    int32_t m_pdef_param;
    int32_t m_mdef_param;

    int32_t m_shot_param1;
    int32_t m_shot_param2;
    int32_t m_shot_param3;
    int32_t m_shot_param4;
    int32_t m_shot_param5;

    int32_t m_crit_param1;
    int32_t m_crit_param2;
    int32_t m_crit_param3;
    int32_t m_crit_param4;
    int32_t m_crit_param5;
    int32_t m_crit_param6;

    int32_t m_break_param1;
    int32_t m_break_param2;
    int32_t m_break_effect_param1;
    int32_t m_break_effect_param2;
    int32_t m_break_effect_param3;

    int32_t m_vampire_param1;
    int32_t m_vampire_param2;
    int32_t m_vampire_effect_param1;
    int32_t m_vampire_effect_param2;
    int32_t m_vampire_effect_param3;

    int32_t m_dmgadd_param1;
    int32_t m_dmgadd_param2;
    int32_t m_dmgadd_param3;
    int32_t m_dmgadd_param4;

    int32_t m_final_param;
    int32_t m_final_warrior_param;
    int32_t m_final_magic_param;
    int32_t m_final_witch_param;
    
};


#define PK_PARAM PKCfg::me()

}

#endif

