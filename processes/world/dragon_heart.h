#ifndef PROCESS_WORLD_DRAGON_HEART_H
#define PROCESS_WORLD_DRAGON_HEART_H

#include "pkdef.h"
#include <unordered_map>
#include <vector>
/*
 * 火龙之心
 */

namespace world{


struct DragonPara
{
    uint16_t roleSkillPower         = 0;    //技能威力(角色技能生效)    
    uint16_t roleSkillDamage        = 0;    //技能伤害                  
    uint16_t rolePveConstDamage     = 0;    //pve固定伤害               
    uint16_t rolepveDamageAdd       = 0;    //pve伤害加成               
    uint16_t rolePvpConstDamage     = 0;    //pvp固定伤害               
    uint16_t rolePvpDamageAdd       = 0;    //pvp伤害加成               

    uint16_t heroSkillPower         = 0;    //技能威力(英雄技能生效)    
    uint16_t heroSkillDamage        = 0;    //技能伤害                  
    uint16_t heroPveConstDamage     = 0;    //pve固定伤害               
    uint16_t heropveDamageAdd       = 0;    //pve伤害加成               
    uint16_t heroPvpConstDamage     = 0;    //pvp固定伤害               
    uint16_t heroPvpDamageAdd       = 0;    //pvp伤害加成               

    uint16_t reduceEnergeCost       = 0;    //释放合击技能龙心能量减少数值
    uint16_t addEnergeLimit         = 0;    //提升龙心能量上限
    uint16_t extraAnger             = 0;    //怒气增长速度提高值
    uint16_t extendReadyTime      = 0;    //增加合击技能准备时间(delay合击技能释放)
};


class Role;
class DragonHeart
{
public:
    DragonHeart(Role& me);

public:
    ~DragonHeart() = default;

private:
    bool checkCondition(uint32_t dragonSoul, uint32_t roleLevel) const;
    void setDragonPara(const std::vector<std::pair<DragonSkillProp, uint16_t>>& para);


public:
    //返回炼心技能列表
    void retDragonSkillList() const;

    //刷新单个炼心技能数据
    void refreshDragonSkill(uint16_t id, uint16_t level) const;

    //升级炼心技能
    void upgradeDragonSkill(uint16_t id);


public:
    uint16_t roleSkillPower() const;
    uint16_t roleSkillDamage() const;
    uint16_t rolePveConstDamage() const;
    uint16_t rolepveDamageAdd() const;
    uint16_t rolePvpConstDamage() const;
    uint16_t rolePvpDamageAdd() const;

    uint16_t heroSkillPower() const;
    uint16_t heroSkillDamage() const;
    uint16_t heroPveConstDamage() const;
    uint16_t heropveDamageAdd() const;
    uint16_t heroPvpConstDamage() const;
    uint16_t heroPvpDamageAdd() const;

    uint16_t reduceEnergeCost() const;
    uint16_t addEnergeLimit() const;
    uint16_t extraAnger() const;
    uint16_t extendReadyTime() const;

    uint32_t energe() const;

private:
    Role&   m_owner;
    DragonPara m_dragonPara;
};

}

#endif
