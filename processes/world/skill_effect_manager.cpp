#include "skill_effect_manager.h"
#include "pk.h"
#include "scene_manager.h"
#include "pet_manager.h"

namespace world{

using namespace water;
using namespace water::componet;


/*
 *
 * owner:逻辑状态拥有者
 * 逻辑库1,直接伤害
 *
 */
logic_retcode SkillLogic_1(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            auto attacker = see.attacker.lock();
            if(nullptr == attacker)
                return logic_retcode::none;

            attacker->m_pkvalue.m_skillPower = see.param1;
            attacker->m_pkvalue.m_skillDamage = see.param2;
            attacker->m_pkvalue.m_damageAddPer = see.param3;
            attacker->m_pkvalue.m_constDamage = see.param4;

            owner.attackMe(attacker, see.hit, see.kind, see.continued);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none;
}



/*
 *
 * 逻辑库2,直接百分比伤害
 *
 */
logic_retcode SkillLogic_2(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            auto attacker = see.attacker.lock();
            if(nullptr == attacker)
                return logic_retcode::none;
            //attacker->m_pkvalue.m_maxhpBleedPer = see.param1;
            //attacker->m_pkvalue.m_hpBleedPer = see.param2;
            //attacker->m_pkvalue.m_constDamage = see.param3;

            int32_t dmg = 0;
            if(see.param1)
                dmg = owner.getMaxHp() * see.param1 / 1000;
            else if(see.param2)
                dmg = owner.getHp() * see.param2 / 1000;
            else
                dmg = see.param3;

            if(dmg > 0)
                owner.changeHpAndNotify(attacker, -dmg, HPChangeType::normal, 0);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}



/*
 *
 * 逻辑库3,伤害吸血
 *
 */
logic_retcode SkillLogic_3(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            owner.m_pkvalue.m_vampirePdamagePer = see.param1;
            owner.m_pkvalue.m_vampireMdamagePer = see.param2;
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}



/*
 *
 * 逻辑库4,破防
 *
 */
logic_retcode SkillLogic_4(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            owner.m_pkvalue.m_ignorePdefencePer = see.param1;
            owner.m_pkvalue.m_ignoreMdefencePer = see.param2;
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}



/*
 *
 * 逻辑库5,以目标中心点为中心的范围性击退
 *
 */
logic_retcode SkillLogic_5(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            std::shared_ptr<Scene> s = owner.scene();
            if(nullptr == s)
                return logic_retcode::none;
            if(s->isArea(owner.pos(), AreaType::security))
                return logic_retcode::none;
            auto atkRole = getRole(see.attacker.lock());
            auto hitbackExec = [&] (PK::Ptr target)
            {
                if(nullptr == target || target->isDead() || target->needErase())
                    return;

                if(owner.level() <= target->level())
                    return;

                SceneItemType sceneItemType;
                if(target->sceneItemType() == SceneItemType::role)
                    sceneItemType = SceneItemType::role;
                else if(target->sceneItemType() == SceneItemType::hero)
                    sceneItemType = SceneItemType::hero;
                else
                    sceneItemType = SceneItemType::npc;

                if(SceneItemType::npc != sceneItemType 
                   && (nullptr != atkRole && attack_mode::peace == atkRole->m_attackMode.mode()))
                    return;

                Direction dir = (see.pos).direction(target->pos());
                //计算被击退距离
                Coord2D newPos = target->pos();
                for(uint32_t i = 0; i < see.param1; ++i)
                {
                    Grid *routeGrid = s->getGridByGridCoord(newPos.neighbor(dir));
                    if(nullptr == routeGrid)
                        break;

                    if(!routeGrid->enterable(sceneItemType))
                        break;

                    newPos = newPos.neighbor(dir);
                }

                LOG_DEBUG("技能, 被击退, atk:{}, target:{}, pos:({},{}), newPos:({},{})", 
                          owner.name(), target->name(), target->pos().x, target->pos().y, newPos.x, newPos.y);
                if(newPos != target->pos())
                {
                    target->chongfengMsgTo9(see.skill_id, see.skill_level, see.attacker.lock(), reverseDirection(dir), MoveType::hitback, newPos);
                    owner.m_rangeHitBackPK.push_back(target);
                }
            };

            auto callback = [&] (Coord2D pos) -> bool
            {
                if(see.pos == pos)
                    return false;

                Grid* grid = s->getGridByGridCoord(pos);
                if(nullptr == grid)
                    return false;

                grid->execGrid(hitbackExec);
                return false;
            };

            s->tryExecAreaByCircle(see.pos, 1, callback);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}




/*
 *
 * 逻辑库6,冲锋
 *
 */
logic_retcode SkillLogic_6(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            owner.chongfeng(see.skill_id, see.skill_level, see.dir, see.param1);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}




/*
 *
 * 逻辑库7,buff/debuff
 *
 */
logic_retcode SkillLogic_7(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            owner.m_buffM.showBuff(see.param1);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}




/*
 *
 * 逻辑库8,反弹
 *
 */
logic_retcode SkillLogic_8(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            //owner.m_reflectPdamagePer = see.param1;
            //owner.m_reflectMdamagePer = see.param2;
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}



/*
 *
 * 逻辑库9,冲锋伤害效果
 *
 */
logic_retcode SkillLogic_9(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            if(nullptr == see.attacker.lock())
                return logic_retcode::none;
            std::shared_ptr<Scene> s = owner.scene();
            if(nullptr == s)
                return logic_retcode::none;

            //冲锋伤害是以攻击者方向上后一格为单元计算伤害
            Coord2D ownerPos = owner.pos();
            Coord2D dmgPos = ownerPos.neighbor(see.dir);
            Grid* dmgGrid = s->getGridByGridCoord(dmgPos);
            if(nullptr == dmgGrid)
                return logic_retcode::none;

            owner.m_pkvalue.m_skillPower = see.param1;
            owner.m_pkvalue.m_skillDamage = see.param2;
            owner.m_pkvalue.m_damageAddPer = see.param3;
            owner.m_pkvalue.m_constDamage = see.param4;

            auto callback = [&see] (PK::Ptr def)
            {
                if(nullptr == def || see.attacker.lock() == def)
                    return;

                if(!Skill::checkTarget(see.attacker.lock(), def, see.target))
                    return;

                def->attackMe(see.attacker.lock(), see.hit, see.kind, see.continued);
            };
            dmgGrid->execGrid(callback);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}



/*
 *
 * 逻辑库10,刷新技能CD
 *
 */
logic_retcode SkillLogic_10(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            //LOG_DEBUG("技能, 触发刷新技能cd效果, owner:{}", (see.attacker.lock())->name());
            owner.clearSkillCD(see.skill_id);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}



/*
 *
 * 逻辑库11,二次伤害(不重新计算)
 *
 */
logic_retcode SkillLogic_11(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            if(owner.issetDoubleDamageSkillLogic(see.skill_id, see.logic_id))
            {
                for(const auto& iter : owner.m_attackHpChangeList)
                {
                    const PK::Ptr& pk = iter.first;
                    if(pk->isDead() || pk->needErase())
                        continue;

                    pk->changeHpAndNotify(see.attacker.lock(), iter.second.hp, iter.second.type);
                }
            }

            if(owner.m_doubleDamageFlag > 0)
            {
                owner.m_doubleDamageLogic = see.logic_id;
                //前端需播放两次特效
                if(see.param1 > 0)
                    owner.m_doubleDamageFlag = 2;
            }
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}



/*
 *
 * 逻辑库12,对目标中心点周围范围伤害(单次)
 *
 */
logic_retcode SkillLogic_12(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            std::shared_ptr<Scene> s = owner.scene();
            if(nullptr == s)
                return logic_retcode::none;

            owner.m_pkvalue.m_skillPower = see.param1;
            owner.m_pkvalue.m_skillDamage = see.param2;
            owner.m_pkvalue.m_damageAddPer = see.param3;
            owner.m_pkvalue.m_constDamage = see.param4;

            auto attacker = see.attacker.lock();
            if(nullptr == attacker)
                return logic_retcode::none;

            auto callback = [s, attacker, &see] (Coord2D pos) -> bool
            {
                Grid* grid = s->getGridByGridCoord(pos);
                if(nullptr == grid)
                    return false;

                grid->execGrid([&] (PK::Ptr def) {
                               if(nullptr == def || attacker == def)
                               return;

                               if(!Skill::checkTarget(attacker, def, see.target))
                               return;

                               def->attackMe(attacker, see.hit, see.kind, see.continued);
                               }
                               );
                return false;
            };
            s->tryExecAreaByCircle(see.pos, see.radius, callback);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}




/*
 *
 * 逻辑库13,对目标中心点周围范围伤害(持续)
 *
 */
logic_retcode SkillLogic_13(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
    case skill_step::timer:
        {
            std::shared_ptr<Scene> s = owner.scene();
            if(nullptr == s)
                return logic_retcode::none;

            owner.m_pkvalue.m_skillPower = see.param1;
            owner.m_pkvalue.m_skillDamage = see.param2;
            owner.m_pkvalue.m_damageAddPer = see.param3;
            owner.m_pkvalue.m_constDamage = see.param4;

            auto attacker = see.attacker.lock();
            if(nullptr == attacker)
                return logic_retcode::none;

            auto callback = [s, attacker, &see] (Coord2D pos) -> bool
            {
                Grid* grid = s->getGridByGridCoord(pos);
                if(nullptr == grid)
                    return false;

                grid->execGrid([&] (PK::Ptr def) {
                               if(nullptr == def || attacker == def)
                               return;

                               if(!Skill::checkTarget(attacker, def, see.target))
                               return;

                               def->attackMe(attacker, see.hit, see.kind, see.continued);
                               }
                               );
                return false;
            };
            s->tryExecAreaByCircle(see.pos, see.radius, callback);
            return logic_retcode::timer;
        }
        break;
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}





/*
 *
 * 逻辑库14,极品套装区分对象属性被动技能效果
 *
 */
logic_retcode SkillLogic_14(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            switch(see.target)
            {
            case TargetType::role:
                owner.m_pkvalue.m_roleConstDamage += see.param1;
                owner.m_pkvalue.m_roleDamageAdd += see.param2;
                break;
            case TargetType::Boss:
                owner.m_pkvalue.m_bossConstDamage += see.param1;
                owner.m_pkvalue.m_bossDamageAdd += see.param2;
                break;
            case TargetType::mob:
                owner.m_pkvalue.m_mobConstDamage += see.param1;
                owner.m_pkvalue.m_mobDamageAdd += see.param2;
                break;
            case TargetType::allnpc:
                owner.m_pkvalue.m_mobConstDamage += see.param1;
                owner.m_pkvalue.m_mobDamageAdd += see.param2;
                owner.m_pkvalue.m_bossConstDamage += see.param1;
                owner.m_pkvalue.m_bossDamageAdd += see.param2;
                break;
            case TargetType::all:
                owner.m_pkvalue.m_roleConstDamage += see.param1;
                owner.m_pkvalue.m_roleDamageAdd += see.param2;
                owner.m_pkvalue.m_bossConstDamage += see.param1;
                owner.m_pkvalue.m_bossDamageAdd += see.param2;
                owner.m_pkvalue.m_mobConstDamage += see.param1;
                owner.m_pkvalue.m_mobDamageAdd += see.param2;
            default:
                break;
            }
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}




/*
 *
 * 逻辑库15,两次伤害单独计算效果
 *
 */
logic_retcode SkillLogic_15(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            auto attacker = see.attacker.lock();
            if(nullptr == attacker)
                return logic_retcode::none;
            see.continued = 1;
            if(attacker->issetDoubleDamageSkillLogic(see.skill_id, see.logic_id))
            {
                owner.attackMe(attacker, see.hit, see.kind, see.continued);
            }

            if(attacker->m_doubleDamageFlag > 0)
            {
                //LOG_DEBUG("技能, 触发二次单独伤害效果, attacker:{}, logic_id:{}", attacker->name(), see.logic_id);
                attacker->m_doubleDamageLogic = see.logic_id;
                //前端需播放两次特效
                if(see.param1 > 0)
                {
                    LOG_DEBUG("技能, 触发两次特效, skill_id:{}, logic_id:{}", see.skill_id, see.logic_id);
                    attacker->m_doubleDamageFlag = 2;
                }
            }
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none; 
}




/*
 *
 * 逻辑库16, 灼烧魔法值效果
 *
 */
logic_retcode SkillLogic_16(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            int32_t dmg = 0;
            if(see.param1)
                dmg = owner.getMaxMp() * see.param1 / 1000;
            else if(see.param2)
                dmg = owner.getMp() * see.param2 / 1000;
            else
                dmg = see.param3;

            if(dmg > 0)
                owner.changeMpAndNotify(-dmg, true);
        }
        break;
    case skill_step::timer:
    case skill_step::stop:
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 * 逻辑库17, 破除魔法盾效果
 *
 */
logic_retcode SkillLogic_17(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        owner.m_buffM.clearShield();
        break;
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 * 逻辑库18, 道士施毒术效果
 *
 */
logic_retcode SkillLogic_18(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            auto attacker = see.attacker.lock();
            if(nullptr == attacker)
                return logic_retcode::none;

            owner.m_poisonAttackerAttr.attackId = attacker->id();
            owner.m_poisonAttackerAttr.attackJob = attacker->job();
            owner.m_poisonAttackerAttr.sceneItem = static_cast<uint8_t>(attacker->sceneItemType());
            owner.m_poisonAttackerAttr.atkMin = attacker->getTotalWitchMin();
            owner.m_poisonAttackerAttr.atkMax = attacker->getTotalWitchMax();
            owner.m_poisonAttackerAttr.lucky = attacker->getTotalLucky();
            owner.m_poisonAttackerAttr.evil = attacker->getTotalEvil();
            owner.m_poisonAttackerAttr.crit = attacker->getTotalCrit();
            owner.m_poisonAttackerAttr.critRatio = attacker->getTotalCritRatio();
            owner.m_poisonAttackerAttr.critDamage = attacker->getTotalCritDmg();
            owner.m_poisonAttackerAttr.damageAdd = attacker->getTotalDmgAdd();
            owner.m_poisonAttackerAttr.damageAddLv = attacker->getTotalDmgAddLv();

            owner.m_poisonAttackerAttr.critk = attacker->critk();
            owner.m_poisonAttackerAttr.skillPower = attacker->m_pkvalue.m_skillPower;
            owner.m_poisonAttackerAttr.skillDamage = attacker->m_pkvalue.m_skillDamage;
            owner.m_poisonAttackerAttr.skillDamageAddPer = attacker->m_pkvalue.m_damageAddPer;
            owner.m_poisonAttackerAttr.skillConstDamage = attacker->m_pkvalue.m_constDamage;

            LOG_DEBUG("技能, 施毒术添加buffid={}", see.param1);
            owner.m_buffM.showBuff(see.param1);
        }
        break;
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 * 逻辑库19, 召唤术效果
 *
 */
logic_retcode SkillLogic_19(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            auto attacker = see.attacker.lock();
            if(nullptr == attacker)
                return logic_retcode::none;

            PetManager::me().summonPet(see.param1, see.skill_id, see.pet_level, attacker);
        }
        break;
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 * 逻辑库20, 死亡复活类被动技能逻辑效果
 *
 */
logic_retcode SkillLogic_20(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            owner.setFeignDead();
            owner.setFeignDeadRelivePercent(see.param1);
        }
        break;
    default:
        break;
    }
    return logic_retcode::none;
}



/*
 *
 * 逻辑库21, 冲锋后对目标几率眩晕效果
 *
 */
logic_retcode SkillLogic_21(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            if(nullptr == see.attacker.lock())
                return logic_retcode::none;
            auto s = owner.scene();
            if(nullptr == s)
                return logic_retcode::none;

            Coord2D stunPos = (owner.pos()).neighbor(see.dir);
            Grid* stunGrid = s->getGridByGridCoord(stunPos);
            if(nullptr == stunGrid)
                return logic_retcode::none;
            auto callback = [&see] (PK::Ptr def)
            {
                //LOG_DEBUG("技能, 冲锋触发眩晕, buffId:{}", see.param1);
                if( nullptr == def || see.attacker.lock() == def)
                    return;

                if((see.attacker.lock())->level() <= def->level())
                    return;

                if(!Skill::checkTarget(see.attacker.lock(), def, see.target))
                    return;

                def->m_buffM.showBuff(see.param1);
            };
            stunGrid->execGrid(callback);
        }
        break;
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 * 逻辑库22, 以目标中心点为中心对推开的目标几率施加某个buff效果
 *
 */
logic_retcode SkillLogic_22(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            for(const auto& iter : owner.m_rangeHitBackPK)
            {
                LOG_DEBUG("技能, def:{} 被推开时添加buffid={}", iter->name(), see.param1);
                iter->m_buffM.showBuff(see.param1);
            }
        }
        break;
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 *  逻辑库23, 增加buff时间
 *
 */
logic_retcode SkillLogic_23(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            LOG_DEBUG("技能, 增加buff时间, id={}, sec={}", see.param1, see.param2);
            owner.m_buffM.addTime(see.param1, see.param2);
        }
        break;
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 *  逻辑库24, 更改技能作用范围
 *   
 */
logic_retcode SkillLogic_24(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        break;
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 * 逻辑库25, 召唤术强化效果
 *
 */
logic_retcode SkillLogic_25(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        break;
    default:
        break;
    }
    return logic_retcode::none;
}




/*
 *
 * 逻辑库26, 火墙效果
 *
 */
logic_retcode SkillLogic_26(PK& owner, SkillEffectEle& see)
{
    switch(see.step)
    {
    case skill_step::start:
        {
            PKAttr ownerAttr;
            ownerAttr.attackJob = owner.job();
            ownerAttr.atkMin = owner.getTotalMAtkMin();
            ownerAttr.atkMax = owner.getTotalMAtkMax();
            ownerAttr.shot = owner.getTotalShot();
            ownerAttr.shotRatio = owner.getTotalShotRatio();
            ownerAttr.lucky = owner.getTotalLucky();
            ownerAttr.evil = owner.getTotalEvil();
            ownerAttr.crit = owner.getTotalCrit();
            ownerAttr.critRatio = owner.getTotalCritRatio();
            ownerAttr.critDamage = owner.getTotalCritRatio();
            ownerAttr.damageAdd = owner.getTotalDmgAdd();
            ownerAttr.damageAddLv = owner.getTotalDmgAddLv();
            ownerAttr.shotk = owner.shotk();
            ownerAttr.critk = owner.critk();
            ownerAttr.skillPower = see.param1;
            ownerAttr.skillDamage = see.param2;
            ownerAttr.skillDamageAddPer = see.param3;
            ownerAttr.skillConstDamage = see.param4;

            FireManager::me().summonFires(see.attacker.lock(), see.pos, see.radius, see.fireAddSec, ownerAttr);
        }
        break;
    default:
        break;
    }
    return logic_retcode::none;
}



/*
 *
 * 逻辑库27, 火墙强化效果
 *
 */
logic_retcode SkillLogic_27(PK& owner, SkillEffectEle& see)
{
    return logic_retcode::none;
}









SkillEffectMgr::SkillEffectMgr(PK& me)
    :m_owner(me)
    ,clearFlag(false)
    ,inited(false)
    ,tick(0)
{
}


void SkillEffectMgr::initFunc()
{
    inited = true;
    m_logicList[1].func = SkillLogic_1;
    m_logicList[2].func = SkillLogic_2;
    m_logicList[3].func = SkillLogic_3;
    m_logicList[4].func = SkillLogic_4;
    m_logicList[5].func = SkillLogic_5;
    m_logicList[6].func = SkillLogic_6;
    m_logicList[7].func = SkillLogic_7;
    m_logicList[8].func = SkillLogic_8;
    m_logicList[9].func = SkillLogic_9;
    m_logicList[10].func = SkillLogic_10;
    m_logicList[11].func = SkillLogic_11;
    m_logicList[12].func = SkillLogic_12;
    m_logicList[13].func = SkillLogic_13;
    m_logicList[14].func = SkillLogic_14;
    m_logicList[15].func = SkillLogic_15;
    m_logicList[16].func = SkillLogic_16;
    m_logicList[17].func = SkillLogic_17;
    m_logicList[18].func = SkillLogic_18;
    m_logicList[19].func = SkillLogic_19;
    m_logicList[20].func = SkillLogic_20;
    m_logicList[21].func = SkillLogic_21;
    m_logicList[22].func = SkillLogic_22;
    m_logicList[23].func = SkillLogic_23;
    m_logicList[24].func = SkillLogic_24;
    m_logicList[25].func = SkillLogic_25;
    m_logicList[26].func = SkillLogic_26;
    m_logicList[27].func = SkillLogic_27;
}



/*
 * 施加一个技能效果在自己身上
 */
void SkillEffectMgr::addSkillEffect(SkillEffectEle& see)
{
    logic_retcode ret = run(see);
    if(ret == logic_retcode::timer)
        m_timerElements[see.logic_id] = see;
}


logic_retcode SkillEffectMgr::run(SkillEffectEle& see)
{
    if(!inited)
        initFunc();
    if(see.logic_id > MAX_SKILL_LOGIC_NUM || see.logic_id <= 0)
    {
        LOG_ERROR("[技能], 参数logic_id错误, {}", see.logic_id);
        return logic_retcode::none;
    }

    return m_logicList[see.logic_id].func(m_owner, see);
}


void SkillEffectMgr::timerExec()
{
    ++tick;
    std::unordered_map<uint32_t, SkillEffectEle>::iterator iter;
    if(clearFlag)
    {
        for(iter = m_timerElements.begin(); iter != m_timerElements.end(); )
        {
            SkillEffectEle& see = iter->second;
            see.step = skill_step::stop;
            run(see);
            m_timerElements.erase(iter++);
        }
        clearFlag = false;
        tick = 0;
        return;
    }

    if(tick%2 == 0)
    {
        for(iter = m_timerElements.begin(); iter != m_timerElements.end(); )
        {
            SkillEffectEle& see = iter->second;
            if (see.sec < 1)
            {
                see.step = skill_step::stop;
                run(see);

                m_timerElements.erase(iter++);
            }
            else
            {
                see.sec -= 1;
                see.step = skill_step::timer;
                see.continued = 1;
                run(see);
                ++iter;
            }
        }
        tick = 0;
    }
}


void SkillEffectMgr::clear()
{
    clearFlag = true;
}


}
