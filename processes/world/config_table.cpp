#include "config_table.h"
#include "water/componet/string_kit.h"


namespace world{


uint32_t SkillBase::parse(const XmlParseNode& node)
{
    id = node.getChildNodeText<TplId>("id");
    node.getChildNodeText<std::string>("name").copy(name, sizeof(name)-1);
    level = node.getChildNodeText<uint32_t>("level");
    injury = node.getChildNodeText<uint8_t>("injury");
    hit = node.getChildNodeText<uint8_t>("hit");
    kind = static_cast<skill_kind>(node.getChildNodeText<uint8_t>("kind"));
    type = static_cast<SkillType>(node.getChildNodeText<uint8_t>("type"));
    trigger_per = node.getChildNodeText<uint32_t>("trigger_per");
    cdtime = node.getChildNodeText<uint32_t>("cdtime");
    min_distance = node.getChildNodeText<uint16_t>("min_distance");
    max_distance = node.getChildNodeText<uint16_t>("max_distance");
    hero_min_distance = node.getChildNodeText<uint16_t>("hero_min_distance");
    hero_max_distance = node.getChildNodeText<uint16_t>("hero_max_distance");
    center_type = static_cast<CenterType>(node.getChildNodeText<uint8_t>("center_type"));
    costmp = node.getChildNodeText<uint32_t>("costmp");
    add_skillexp = node.getChildNodeText<uint32_t>("add_skillexp");

    std::vector<std::string> subvs;
    std::string str = node.getChildNodeText<std::string>("effects");
    std::vector<std::string> vs = componet::splitString(str, ",");
    for(const auto& iter : vs)
    {
        subvs.clear();
        subvs = componet::splitString(iter, "-");
        if(subvs.size() < 2)
            continue;

        uint32_t effectId = atoi(subvs[0].c_str());
        uint32_t effeceLv = atoi(subvs[1].c_str());
        effects.push_back(std::make_pair(effectId, effeceLv));
    }

    vs.clear();
    vs = componet::splitString(node.getChildNodeText<std::string>("job"), "_");
    if(vs.size() >= 2)
    {
        jobs.first = static_cast<Job>(atoi(vs[0].c_str()));
        jobs.second = static_cast<Job>(atoi(vs[1].c_str()));
    }
    else if(vs.size() < 2 && vs.size() > 0)
    {
        jobs.first = static_cast<Job>(atoi(vs[0].c_str()));
        jobs.second = jobs.first;
    }

    sceneItem = static_cast<SceneItemType>(node.getChildNodeText<uint8_t>("pkmodel"));
    role_level = node.getChildNodeText<uint32_t>("role_level");
    needexp = node.getChildNodeText<uint32_t>("needexp");

    str = node.getChildNodeText<std::string>("consumes");
    vs.clear();
    vs = componet::splitString(str, ",");
    for(const auto& iter : vs)
    {
        subvs.clear();
        subvs = componet::splitString(iter, "-");
        if(subvs.size() < 2)
            continue;

        uint32_t objId = atoi(subvs[0].c_str());
        uint32_t objNum = atoi(subvs[1].c_str());
        consumes.push_back(std::make_pair(objId, objNum));
    }

    return skill_hash(id, level);
}


uint32_t SkillStrengthenBase::parse(const XmlParseNode& node)
{
    skill_id = node.getChildNodeText<TplId>("skill_id");
    strengthen_level = node.getChildNodeText<uint32_t>("strengthen_level");
    role_level = node.getChildNodeText<uint32_t>("role_level");
    skill_level = node.getChildNodeText<uint32_t>("skill_level");
    
    std::vector<std::string> subvs, vs;
    std::string str = node.getChildNodeText<std::string>("effects");
    vs = componet::splitString(str, ",");
    for(const auto& iter : vs)
    {
        subvs.clear();
        subvs = componet::splitString(iter, "-");
        if(subvs.size() < 2)
            continue;

        uint32_t effectId = atoi(subvs[0].c_str());
        uint32_t effectLv = atoi(subvs[1].c_str());
        effects.push_back(std::make_pair(effectId, effectLv));
    }

    vs.clear();
    str = node.getChildNodeText<std::string>("consumes");
    vs = componet::splitString(str, ",");
    for(const auto& iter : vs)
    {
        subvs.clear();
        subvs = componet::splitString(iter, "-");
        if(subvs.size() < 2)
            continue;

        uint32_t objId = atoi(subvs[0].c_str());
        uint32_t objNum = atoi(subvs[1].c_str());
        consumes.push_back(std::make_pair(objId, objNum));
    }
    return skill_strengthen_hash(skill_id, strengthen_level);
}


uint32_t SkillEffectBase::parse(const XmlParseNode& node)
{
    effectid = node.getChildNodeText<uint32_t>("effectid");
    level = node.getChildNodeText<uint32_t>("level");
    trigger_per = node.getChildNodeText<uint32_t>("trigger_per");
    target_type = static_cast<TargetType>(node.getChildNodeText<uint8_t>("target_type"));
    target_num = node.getChildNodeText<uint32_t>("target_num");
    range_type = static_cast<Range>(node.getChildNodeText<uint8_t>("range_type"));
    range_param1 = node.getChildNodeText<uint16_t>("range_param1");
    range_param2 = node.getChildNodeText<uint16_t>("range_param2");
    logic_id = node.getChildNodeText<uint32_t>("logic_id");
    param1 = node.getChildNodeText<uint32_t>("param1");
    param2 = node.getChildNodeText<uint32_t>("param2");
    param3 = node.getChildNodeText<uint32_t>("param3");
    param4 = node.getChildNodeText<uint32_t>("param4");

    return skill_effect_hash(effectid, level);
}


uint32_t BuffBase::parse(const XmlParseNode& node)
{
    buffid = node.getChildNodeText<TplId>("buffid");
    group = node.getChildNodeText<uint16_t>("group");
    status_id = node.getChildNodeText<uint32_t>("status_id");
    action_type = static_cast<buff_action>(node.getChildNodeText<uint8_t>("action_type"));
    priority = node.getChildNodeText<uint8_t>("priority");
    time_merge = node.getChildNodeText<uint8_t>("time_merge");
    die_exist = node.getChildNodeText<uint8_t>("die_exist");
    offline_exist = node.getChildNodeText<uint8_t>("offline_exist");
    dur = node.getChildNodeText<uint32_t>("dur");
    dur_type = node.getChildNodeText<uint8_t>("dur_type");
    interval = node.getChildNodeText<uint32_t>("interval");
    sec = node.getChildNodeText<uint32_t>("sec");
    endtime = node.getChildNodeText<uint32_t>("endtime");
    recovery_hp = node.getChildNodeText<uint32_t>("recovery_hp");
    recovery_hp_percent = node.getChildNodeText<uint16_t>("recovery_hp_percent");
    recovery_mp = node.getChildNodeText<uint32_t>("recovery_mp");
    recovery_mp_percent = node.getChildNodeText<uint16_t>("recovery_mp_percent");
    damage_hp = node.getChildNodeText<uint32_t>("damage_hp");
    damage_hp_percent = node.getChildNodeText<uint16_t>("damage_hp_percent");
    damage_mp = node.getChildNodeText<uint32_t>("damage_mp");
    damage_mp_percent = node.getChildNodeText<uint16_t>("damage_mp_percent");
    role_exp_percent = node.getChildNodeText<uint16_t>("role_exp");
    hero_exp_percent = node.getChildNodeText<uint16_t>("hero_exp");

    std::vector<std::string> subvs;
    std::string str = node.getChildNodeText<std::string>("props");
    std::vector<std::string> vs = componet::splitString(str, ",");
    for(const auto& iter : vs)
    {
        subvs.clear();
        subvs = componet::splitString(iter, ":");
        if(subvs.size() < 2)
            continue;

        PropertyType type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
        int32_t value = atoi(subvs[1].c_str());
        props.push_back(std::make_pair(type, value));
    }

    str = node.getChildNodeText<std::string>("per_props");
    vs.clear();
    vs = componet::splitString(str, ",");
    for(const auto& iter : vs)
    {
        subvs.clear();
        subvs = componet::splitString(iter, ":");
        if(subvs.size() < 2)
            continue;

        PropertyType type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
        int32_t value = atoi(subvs[1].c_str());
        percent_props.push_back(std::make_pair(type, value));
    }
    return buffid;
}


uint32_t LevelPropsBase::parse(const XmlParseNode& node)
{
    level = node.getChildNodeText<uint32_t>("level");
    job = node.getChildNodeText<uint32_t>("job");
    type = node.getChildNodeText<uint32_t>("type");

    std::vector<std::string> subvs;
    std::string str = node.getChildNodeText<std::string>("props");
    std::vector<std::string> vs = componet::splitString(str, ",");
    for(const auto& iter : vs)
    {
        subvs.clear();
        subvs = componet::splitString(iter, "-");
        if(subvs.size() < 2)
            continue;

        PropertyType prop_type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
        uint32_t prop_val = atoi(subvs[1].c_str());
        level_props.push_back(std::make_pair(prop_type, prop_val));
    }

    return  rlp_hash(job, type, level);
}


uint32_t KValueBase::parse(const XmlParseNode& node)
{
    level = node.getChildNodeText<uint32_t>("level");
    shotk = node.getChildNodeText<uint32_t>("shotk");
    critk = node.getChildNodeText<uint32_t>("critk");

    return level;
}


}
