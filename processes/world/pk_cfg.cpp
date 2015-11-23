#include "pk_cfg.h"
#include "water/componet/xmlparse.h"
#include "water/componet/exception.h"
#include "water/componet/logger.h"

namespace world{

using namespace water;       
using componet::XmlParseDoc; 
using componet::XmlParseNode;

PKCfg::PKCfg()
    :m_hp_param1(0)
    ,m_hp_param2(0)
    ,m_hp_param3(0)
    ,m_mp_param1(0)
    ,m_mp_param2(0)
    ,m_mp_param3(0)
    ,m_atk_param(0)
    ,m_pdef_param(0)
    ,m_mdef_param(0)
    ,m_shot_param1(0)
    ,m_shot_param2(0)
    ,m_shot_param3(0)
    ,m_shot_param4(0)
    ,m_shot_param5(0)
    ,m_crit_param1(0)
    ,m_crit_param2(0)
    ,m_crit_param3(0)
    ,m_crit_param4(0)
    ,m_crit_param5(0)
    ,m_break_param1(0)
    ,m_break_param2(0)
    ,m_break_effect_param1(0)
    ,m_break_effect_param2(0)
    ,m_break_effect_param3(0)
    ,m_vampire_param1(0)
    ,m_vampire_param2(0)
    ,m_vampire_effect_param1(0)
    ,m_vampire_effect_param2(0)
    ,m_vampire_effect_param3(0)
    ,m_dmgadd_param1(0)
    ,m_dmgadd_param2(0)
    ,m_dmgadd_param3(0)
    ,m_dmgadd_param4(0)
    ,m_final_param(0)
    ,m_final_warrior_param(0)
    ,m_final_magic_param(0)
    ,m_final_witch_param(0)
{
}


PKCfg& PKCfg::me()
{
    static PKCfg me;
    return me;
}


void PKCfg::loadConfig(const std::string& cfgDir)
{
    const std::string filename = cfgDir + "/pk_cfg.xml";
    XmlParseDoc doc(filename);                                                          
    XmlParseNode root = doc.getRoot();                                                  
    if(!root)                                                                           
    {                                                                                   
        EXCEPTION(componet::ExceptionBase, filename + " parse root node failed"); 
        return;                                                                         
    }                                                                                   

    m_hp_param1 = root.getChildNodeText<int32_t>("hp_param1");
    m_hp_param2 = root.getChildNodeText<int32_t>("hp_param2");
    m_hp_param3 = root.getChildNodeText<int32_t>("hp_param3");
    m_mp_param1 = root.getChildNodeText<int32_t>("mp_param1");
    m_mp_param2 = root.getChildNodeText<int32_t>("mp_param2");
    m_mp_param3 = root.getChildNodeText<int32_t>("mp_param3");
    m_atk_param = root.getChildNodeText<int32_t>("atk_param");
    m_pdef_param = root.getChildNodeText<int32_t>("pdef_param");
    m_mdef_param = root.getChildNodeText<int32_t>("mdef_param");
    m_shot_param1 = root.getChildNodeText<int32_t>("shot_param1");
    m_shot_param2 = root.getChildNodeText<int32_t>("shot_param2");
    m_shot_param3 = root.getChildNodeText<int32_t>("shot_param3");
    m_shot_param4 = root.getChildNodeText<int32_t>("shot_param4");
    m_shot_param5 = root.getChildNodeText<int32_t>("shot_param5");
    m_crit_param1 = root.getChildNodeText<int32_t>("crit_param1");
    m_crit_param2 = root.getChildNodeText<int32_t>("crit_param2");
    m_crit_param3 = root.getChildNodeText<int32_t>("crit_param3");
    m_crit_param4 = root.getChildNodeText<int32_t>("crit_param4");
    m_crit_param5 = root.getChildNodeText<int32_t>("crit_param5");
    m_crit_param6 = root.getChildNodeText<int32_t>("crit_param6");
    m_break_param1 = root.getChildNodeText<int32_t>("break_param1");
    m_break_param2 = root.getChildNodeText<int32_t>("break_param2");
    m_break_effect_param1 = root.getChildNodeText<int32_t>("break_effect_param1");
    m_break_effect_param2 = root.getChildNodeText<int32_t>("break_effect_param2");
    m_break_effect_param3 = root.getChildNodeText<int32_t>("break_effect_param3");
    m_vampire_param1 = root.getChildNodeText<int32_t>("vampire_param1");
    m_vampire_param2 = root.getChildNodeText<int32_t>("vampire_param2");
    m_vampire_effect_param1 = root.getChildNodeText<int32_t>("vampire_effect_param1");
    m_vampire_effect_param2 = root.getChildNodeText<int32_t>("vampire_effect_param2");
    m_vampire_effect_param3 = root.getChildNodeText<int32_t>("vampire_effect_param3");
    m_dmgadd_param1 = root.getChildNodeText<int32_t>("dmgadd_param1");
    m_dmgadd_param2 = root.getChildNodeText<int32_t>("dmgadd_param2");
    m_dmgadd_param3 = root.getChildNodeText<int32_t>("dmgadd_param3");
    m_final_param = root.getChildNodeText<int32_t>("final_param");
    m_final_warrior_param = root.getChildNodeText<int32_t>("final_warrior_param");
    m_final_magic_param = root.getChildNodeText<int32_t>("final_magic_param");
    m_final_witch_param = root.getChildNodeText<int32_t>("final_witch_param");

    //LOG_DEBUG("hp_param1:{}, hp_param2:{}, atk_param:{}, pdef_param:{}, shot_param1:{}, crit_param1:{}, break_param1:{}, break_effect_param1:{}, vampire_param1:{}, vampire_effect_param1:{}, dmgadd_param1:{}, final_param:{}, final_warrior_param:{}, final_magic_param:{}, final_witch_param:{}", m_hp_param1, m_hp_param2, m_atk_param, m_pdef_param, m_shot_param1, m_crit_param1, m_break_param1, m_break_effect_param1, m_vampire_param1, m_vampire_effect_param1, m_dmgadd_param1, m_final_param, m_final_warrior_param, m_final_magic_param, m_final_witch_param);
}

}
