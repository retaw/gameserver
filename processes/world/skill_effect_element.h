/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-09 19:50 +0800
 *
 * Modified: 2015-04-09 19:50 +0800
 *
 * Description: 技能效果状态元素
 */

#ifndef PROCESS_WORLD_SKILL_EFFECT_ELEMENT_H
#define PROCESS_WORLD_SKILL_EFFECT_ELEMENT_H

#include "position.h"
#include "pkdef.h"

#include "water/componet/datetime.h"
#include "water/common/roledef.h"

#include <memory>
#include <vector>


namespace world{


class PK;
struct SkillEffectEle
{
    SkillEffectEle()
    : logic_id(0)
    , step(skill_step::start)
    , sec(0)
    , skill_id(0)
    , skill_level(0)
    , kind(skill_kind::normal)
    , hit(0)
    , target(TargetType::all)
    , radius(0)
    , continued(0)
    , param1(0)
    , param2(0)
    , param3(0)
    , param4(0)
    {
    }

    //效果逻辑库id
    uint32_t logic_id;

    //执行步骤
    skill_step step;

    //剩余时间(秒)
    union
    {
        uint32_t sec;
        uint32_t fireAddSec;
    };

    //技能id
    TplId skill_id;

    union
    {
        uint32_t skill_level;
        uint32_t pet_level;
    };

    //技能种类
    skill_kind kind;

    //是否必中
    uint8_t hit;

    //技能生效目标类型
    TargetType target;

    uint16_t radius;

    //方向
    Direction dir;

    //是否持续伤害
    uint8_t continued;

    //攻击者
    std::weak_ptr<PK> attacker;

    //攻击者坐标
    Coord2D att_pos;
    //伤害中心点
    Coord2D pos;

    //效果参数
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint32_t param4;
};



/*
 *
 * 释放技能时临时pk属性缓存集合
 *
 */
class PKValue final
{
public:
    explicit PKValue();
    ~PKValue() = default;

    void clear();

public:
    //技能威力
    uint32_t m_skillPower;

    //技能伤害
    uint32_t m_skillDamage;

    //增伤
    uint32_t m_damageAddPer;

    //固定伤害
    uint32_t m_constDamage;

    //吸血(物理伤害的百分比转换为自身血量)
    uint32_t m_vampirePdamagePer;

    //吸血(魔法伤害伤害的百分比转换为自身血量)
    uint32_t m_vampireMdamagePer;

    //破防(无视物防百分比)
    uint32_t m_ignorePdefencePer;

    //破防(无视魔防百分比)
    uint32_t m_ignoreMdefencePer;

    //对boss固定伤害和加成
    uint32_t m_bossConstDamage;
    uint32_t m_bossDamageAdd;

    //对小怪固定伤害和加成
    uint32_t m_mobConstDamage;
    uint32_t m_mobDamageAdd;

    //对主角固定伤害加成
    uint32_t m_roleConstDamage;
    uint32_t m_roleDamageAdd;

};


}


#endif

