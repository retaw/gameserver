#include "skill_manager.h"

#include "pk.h"
#include "world.h"
#include "scene.h"

#include "water/componet/random.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/skill.h"
#include "protocol/rawmsg/private/skill.codedef.private.h"



namespace world{


using namespace water;
using namespace water::componet;
using namespace std::chrono;


/*
 * 获取某个方向的两垂直方向(很笨的方法)
 */
std::vector<Direction> verticalDir(Direction dir)
{
    std::vector<Direction> out;
    if(Direction::none == dir)
        return out;

    switch(dir)
    {
    case Direction::down:
        {
            out.push_back(Direction::left);
            out.push_back(Direction::right);
        }
        break;
    case Direction::rightdown:
        {
            out.push_back(Direction::leftdown);
            out.push_back(Direction::rightup);
        }
        break;
    case Direction::right:
        {
            out.push_back(Direction::down);
            out.push_back(Direction::up);
        }
        break;
    case Direction::rightup:
        {
            out.push_back(Direction::rightdown);
            out.push_back(Direction::leftup);
        }
        break;
    case Direction::up:
        {
            out.push_back(Direction::right);
            out.push_back(Direction::left);
        }
        break;
    case Direction::leftup:
        {
            out.push_back(Direction::leftdown);
            out.push_back(Direction::rightup);
        }
        break;
    case Direction::left:
        {
            out.push_back(Direction::up);
            out.push_back(Direction::down);
        }
        break;
    case Direction::leftdown:
        {
            out.push_back(Direction::leftup);
            out.push_back(Direction::rightdown);
        }
        break;
    default:
        break;
    }

    return out;
}


//检查两点的距离(不是实际距离,只用于范围内的判断)
bool checkDistance(const Coord2D& pos1, const Coord2D& pos2, uint16_t distance)
{
    return std::abs(pos1.x-pos2.x) <= distance && std::abs(pos1.y-pos2.y) <= distance;
}


/*
 *
 * 判断一个点是否在直线格子上
 * pos1:开始坐标  pos2:终点坐标  dst:待判断的坐标点 cell:攻击方向上的格数
 *
 */
bool inLine(const Coord2D& pos1, Direction dir, const Coord2D& dst, uint16_t cell)
{
    LOG_DEBUG("技能, 直线伤害, dir:{}, attpos({},{}), dst({},{}), cell:{}", dir, pos1.x, pos1.y, dst.x, dst.y, cell);
    Coord2D tmp = pos1;
    for(uint16_t i = 0; i < cell; ++i)
    {
        if(tmp.neighbor(dir) == dst)
            return true;
        tmp = tmp.neighbor(dir);
    }
    return false;
}


/*
 *
 * 判断点是否在圆形范围内
 * r:半径   center:中心点  dst:待判断的坐标点
 *
 */
bool inCircle(const Coord2D& center, const Coord2D& dst, uint16_t r)
{
    if(std::abs(center.x - dst.x) <= r && std::abs(center.y - dst.y) <= r)
        return true;
    else
        return false;
}


/*
 *
 * 施加一个技能效果到格子上所有目标
 *
 */
void putSkillEffectInGrid(Grid* grid, SkillEffectEle& see)
{
    if(nullptr == grid)
        return;
    auto atk = see.attacker.lock();
    if(nullptr == atk)
        return;
    auto callback = [&] (PK::Ptr def)
    {
        if(nullptr == def)
            return;

        //LOG_DEBUG("技能, 单体攻击, 收到坐标:{}, defPos:{}", see.pos, def->pos());

        if(!Skill::checkTarget(atk, def, see.target))
            return;
        def->m_skillEffectM.addSkillEffect(see);
    };
    grid->execGrid(callback);
}


/*
 *
 * 范围内所有格子上目标施加一个技能效果
 *
 */
void putSkillEffectInCircle(Scene::Ptr s, SkillEffectEle& see)
{
    auto atk = see.attacker.lock();
    if(nullptr == atk)
        return;
    auto callback = [&] (Coord2D pos) -> bool
    {
        Grid* grid = s->getGridByGridCoord(pos);
        if(nullptr == grid)
            return false;

        grid->execGrid([&] (PK::Ptr def) {
                       if(nullptr == def)
                       return;

                       if(!Skill::checkTarget(atk, def, see.target))
                       return;

                       def->m_skillEffectM.addSkillEffect(see);
                       }
                      );
        return false;
    };
    s->tryExecAreaByCircle(see.pos, see.radius, callback);
}


//*******************技能单元开始********************
Skill::Skill()
: m_strengthenLevel(0)
, m_exp(0)
, m_lastUseTime(EPOCH)
, m_changedRangeType(Range::none)
, m_changedPetId(0)
, m_fireAddSec(0)
{
}


Skill::Ptr Skill::create(TplId id, uint32_t level/*=1*/)
{
    auto s = std::make_shared<Skill>();
    if(nullptr == s)
    {
        LOG_ERROR("技能创建,申请内存失败,id={}", id);
        return nullptr;
    }

    if(!s->initCTData(id, level))
    {
        LOG_ERROR("技能数据初始化时,找不到配置表数据");
        return nullptr;
    }

    return s;
}


bool Skill::initCTData(TplId id, uint32_t level)
{
    m_skillB = skillCT.get(skill_hash(id, level));
    if(nullptr == m_skillB)
        return false;

    m_id = id;
    return true;
}



void Skill::initEffect()
{
    for(const auto& iter : m_skillB->effects)
    {
        SkillEffectBase::Ptr effect_ptr = skillEffectCT.get(skill_effect_hash(iter.first, iter.second));
        insertEffect(effect_ptr);
    }
}


void Skill::initStrengthenEffect()
{
    if(0 == m_strengthenLevel)
        return;

    SkillStrengthenBase::Ptr ssp = skillStrengthenCT.get(skill_strengthen_hash(m_id, m_strengthenLevel));
    if(nullptr == ssp)
        return;
    for(const auto& iter : ssp->effects)
    {
        SkillEffectBase::Ptr effect_ptr = skillEffectCT.get(skill_effect_hash(iter.first, iter.second));
        if(nullptr != effect_ptr)
        {
            if(24 == effect_ptr->logic_id)
            {
                //该效果为更改技能作用范围
                m_changedRangeType = static_cast<Range>(effect_ptr->param1);
                m_changedRangeParam1 = effect_ptr->param2;
                m_changedRangeParam2 = effect_ptr->param3;
                continue;
            }
            if(25 == effect_ptr->logic_id)
            {
                //召唤兽强化效果
                m_changedPetId = effect_ptr->param1;
                continue;
            }
            if(27 == effect_ptr->logic_id)
            {
                //火墙强化效果
                m_fireAddSec = effect_ptr->param1;
                continue;
            }
        }
        insertEffect(effect_ptr);
    }
}


void Skill::deleteStrengthenEffect()
{
    if(0 == m_strengthenLevel)
        return;
    SkillStrengthenBase::Ptr ssp = skillStrengthenCT.get(skill_strengthen_hash(m_id, m_strengthenLevel));
    if(nullptr == ssp)
        return;
    for(const auto& iter : ssp->effects)
    {
        SkillEffectBase::Ptr effect_ptr = skillEffectCT.get(skill_effect_hash(iter.first, iter.second));
        if(nullptr != effect_ptr)
            deleteEffect(effect_ptr);
    }
}


bool Skill::checkUpgradeSkill(PK::Ptr owner, const SkillBase::Ptr nextptr)
{
    Role::Ptr role = getRole(owner);
    if(nullptr == role)
        return false;

    if(getKind() == skill_kind::joint)
    {
        //合击技能不能升级
        return false;
    }

    if(nextptr->jobs.first != owner->job())
    {
        role->sendSysChat("技能职业不对");
        return false;
    }

    if(owner->level() < nextptr->role_level)
    {
        if(owner->sceneItemType() == SceneItemType::hero)
            role->sendSysChat("英雄等级不够");
        else
            role->sendSysChat("主角等级不够");
        return false;
    }

    if(m_exp < nextptr->needexp)
    {
        role->sendSysChat("技能经验不足");
        return false;
    }

    for(const auto& iter : nextptr->consumes)
    {
        if(role->m_packageSet.getObjNum(iter.first, PackageType::role) < iter.second)
        {
            role->sendSysChat("技能升级所需材料不足");
            return false;
        }
    }
    return true;
}



bool Skill::checkStrengthenSkill(PK::Ptr owner, const SkillStrengthenBase::Ptr nextptr)
{
    Role::Ptr role = getRole(owner);
    if(nullptr == role)
        return false;
    if(owner->level() < nextptr->role_level)
    {
        if(owner->sceneItemType() == SceneItemType::hero)
            role->sendSysChat("英雄等级不够");
        else
            role->sendSysChat("主角等级不够");
        return false;
    }
    if(getLevel() < nextptr->skill_level)
    {
        role->sendSysChat("该技能等级不够");
        return false;
    }

    for(const auto& iter : nextptr->consumes)
    {
        if(role->m_packageSet.getObjNum(iter.first, PackageType::role) < iter.second)
        {
            role->sendSysChat("技能强化所需材料不足");
            return false;
        }
    }
    return true;
}



TplId Skill::getID() const
{
    return m_id;
}

std::string Skill::getName() const
{
    return m_skillB->name;
}

uint32_t Skill::getLevel() const
{
    return m_skillB->level;
}

std::pair<Job, Job> Skill::job() const
{
    return m_skillB->jobs;
}

void Skill::setStrengthenLv(uint32_t level)
{
    m_strengthenLevel = level;
}

uint32_t Skill::getStrengthenLv() const
{
    return m_strengthenLevel;
}

void Skill::addExp(uint32_t exp)
{
    m_exp += exp;
}

void Skill::setExp(uint32_t exp)
{
    m_exp = exp;
}

uint32_t Skill::getExp() const
{
    return m_exp;
}

SkillType Skill::getType() const
{
    return m_skillB->type;
}


bool Skill::isInjury() const
{
    return m_skillB->injury > 0;
}


CenterType Skill::getCenter() const
{
    return m_skillB->center_type;
}


uint16_t Skill::getMaxDistance() const
{
    return m_skillB->max_distance;
}

uint16_t Skill::getMaxHeroDistance() const
{
    return m_skillB->hero_max_distance;
}


skill_kind Skill::getKind() const
{
    return m_skillB->kind;
}


uint32_t Skill::getCDTime() const
{
    return m_skillB->cdtime;
}


bool Skill::checkCostMp(PK::Ptr atk)
{
    if(m_skillB->kind == skill_kind::joint)
    {
        //合击技能攻击者必定为Role
        Role::Ptr atkRole = std::static_pointer_cast<Role>(atk);
        return atkRole->checkEnerge(m_skillB->costmp);
    }
    return atk->getMp() >= m_skillB->costmp;
}


void Skill::costMp(PK::Ptr atk)
{
    int32_t mp = m_skillB->costmp;
    if(0 == mp)
        return;
    if(m_skillB->kind == skill_kind::joint)
    {
        Role::Ptr atkRole = std::static_pointer_cast<Role>(atk);
        atkRole->subEnerge(mp);
    }
    else
    {
        atk->changeMpAndNotify(-mp);
    }
}


//检查主动技能cd
bool Skill::checkCDTime() const
{
    TimePoint nowtp = Clock::now();
    if(nowtp < m_lastUseTime + milliseconds {m_skillB->cdtime})
    {
        LOG_DEBUG("cdtime中, skillId:{}, now-last={}, cdtime={} ~~~~~~~~~", 
                  getID(), duration_cast<milliseconds>(nowtp-m_lastUseTime).count(), m_skillB->cdtime);
        return false;
    }
    return true;
}


void Skill::clearCD(PK::Ptr owner)
{
    if(EPOCH == m_lastUseTime)
        return;

    m_lastUseTime = EPOCH;
    PublicRaw::RefreshSkillCD send;
    send.id = owner->id();
    send.sceneItem = static_cast<uint8_t>(owner->sceneItemType());
    send.skillId = m_id;
    owner->sendToMe(RAWMSG_CODE_PUBLIC(RefreshSkillCD), &send, sizeof(send));
}


bool Skill::checkReleaseJointSkill(PK::Ptr atk, Coord2D pos)
{
    if(!atk->isJointReadyState())
    {
        LOG_DEBUG("合击, 非法状态");
        return false;
    }
    Role::Ptr atkRole = getRole(atk);
    if(nullptr == atkRole)
        return false;
    Hero::Ptr hero = atkRole->m_heroManager.getSummonHero();
    if(nullptr == hero)
        return false;

    if(!checkDistance(atk->pos(), pos, getMaxDistance())
       || !checkDistance(hero->pos(), pos, getMaxHeroDistance()))
    {
        LOG_DEBUG("合击, 不在攻击范围");
        return false;
    }

    if(!hero->checkPublicCDTime())
    {
        LOG_DEBUG("合击, 英雄技能公共cd中");
        return false;
    }

    atk->clearJointReadyState();
    hero->retAttackCmdTo9(hero->pos().direction(pos), getID(), getLevel(), pos);
    return true;
}


void Skill::insertEffect(SkillEffectBase::Ptr sptr)
{
    if(nullptr == sptr)
    {
        LOG_ERROR("插入效果fail,insertEffect");
        return;
    }

    //deleteEffect(sptr);
    m_effectList.push_back(sptr);
}


void Skill::deleteEffect(SkillEffectBase::Ptr sptr)
{
    for(auto iter = m_effectList.begin(); iter != m_effectList.end(); ++iter)
    {
        SkillEffectBase::Ptr& ptr = *iter;
        if(nullptr == ptr)
            continue;
        if(ptr->effectid == sptr->effectid)
        {
            m_effectList.erase(iter);
            break;
        }
    }
}


AttackRetcode Skill::processActive(PK::Ptr atk, const PublicRaw::RoleRequestAttack* cmd)
{
    if(!(cmd->step == 1 && atk->lastAttackSuccessSkill() == cmd->skillId) 
       && !checkCDTime())
        return AttackRetcode::cdtime;

    if(!checkCostMp(atk))
        return AttackRetcode::mp;

    PK::Ptr target = PK::getPkptr(cmd->beAtkId, static_cast<SceneItemType>(cmd->beAtkModel));
    //LOG_DEBUG("技能, 单体攻击, atk:{}, 攻击者坐标:{}, 被攻击者坐标:{} 收到坐标:{},{}", 
    //          atk->name(), atk->pos(), nullptr != target ? target->pos() : Coord2D(0,0), cmd->posX, cmd->posY);
    SkillEffectEle see;
    see.att_pos = atk->pos();
    CenterType center = getCenter();
    switch(center)
    {
    case CenterType::self:
        see.pos = atk->pos();
        break;
    case CenterType::target:
        {
            if(nullptr != target)
            {
                see.pos = target->pos();
            }
            else
            {
                see.pos.x = cmd->posX;
                see.pos.y = cmd->posY;
            }
        }
        break;
    case CenterType::remote:
        {
            see.pos.x = cmd->posX;
            see.pos.y = cmd->posY;
        }
        break;
    default:
        LOG_DEBUG("技能, 无效技能落点");
        return AttackRetcode::unknown;
    }

    if(atk->sceneItemType() == SceneItemType::npc 
       && !checkDistance(see.att_pos, see.pos, getMaxDistance()))
        return AttackRetcode::range;

    if(0 == cmd->step)//只播放攻击动作
    {
        m_lastUseTime = Clock::now();
        atk->setLastAttackSuccessSkill(cmd->skillId);
        atk->retAttackCmdTo9(static_cast<Direction>(cmd->dir), getID(), getLevel(), see.pos);
        return AttackRetcode::success;
    }
    else if(2 == cmd->step)//攻击动作和伤害计算并行
    {
        if(getKind() == skill_kind::joint && !checkReleaseJointSkill(atk, see.pos))
            return AttackRetcode::unknown;
        m_lastUseTime = Clock::now();
        atk->retAttackCmdTo9(static_cast<Direction>(cmd->dir), getID(), getLevel(), see.pos);
    }
    else if(3 == cmd->step) //合击进入攻击准备状态
    {
        if(getKind() != skill_kind::joint || !atk->canJointSkillReady(job()))
            return AttackRetcode::unknown;
        return AttackRetcode::success;
    }

    see.attacker = atk;
    see.dir = static_cast<Direction>(cmd->dir);
    see.skill_id = cmd->skillId;
    see.skill_level = m_skillB->level;
    see.kind = m_skillB->kind;
    see.hit = m_skillB->hit;

    //攻击者发起攻击前一些需要清理的东东
    {
        costMp(atk);
        atk->beforeSkillActionClear();
    }

    atk->m_skillM.processPassiveSkill(skill_kind::passiveDamage);
    for(const auto& iter : m_effectList)
    {
        doEffect(atk, iter, see);
    }

    //if(isInjury())
    //{
    //    atk->m_skillM.processPassiveSkill(skill_kind::passiveBuff);
    //    atk->m_skillM.processPassiveSkill(skill_kind::passiveDebuff);
    //    atk->m_skillM.processPassiveSkill(skill_kind::passiveStatus);
    //}

    {//技能释放完后,清除一些攻击者身上的数据或状态
        atk->afterSkillActionClear(getID());
    }

    if(m_skillB->kind != skill_kind::normal)
        addExp(m_skillB->add_skillexp);
    return AttackRetcode::success;

}


/*
 * 
 */
bool Skill::processPassive(PK::Ptr atk)
{
    //被动技能概率触发
    Random<uint32_t> r(1,1000);
    uint32_t rand = r.get();
    if(m_skillB->trigger_per < rand)
        return false;

    LOG_DEBUG("被动技能,触发 id:{}, kind:{}~~~~~~~~~~~~~", getID(), getKind());
    SkillEffectEle see;
    see.attacker = atk;
    see.kind = m_skillB->kind;
    for(auto& iter : m_effectList)
    {
        doEffect(atk, iter, see);
    }

    return true;
}


AttackRetcode Skill::action(PK::Ptr atk, const PublicRaw::RoleRequestAttack* cmd)
{
    if(nullptr == atk || nullptr == m_skillB)
        return AttackRetcode::invalidTarget;

    if(m_skillB->kind == skill_kind::joint && atk->sceneItemType() != SceneItemType::role)
        return AttackRetcode::unknown;

    return processActive(atk, cmd);
}


void Skill::doEffect(PK::Ptr atk, SkillEffectBase::Ptr sptr, SkillEffectEle& see)
{
    if(nullptr == sptr)
    {
        LOG_ERROR("doEffect, 技能id={} 效果列表为空", getID());
        return;
    }

    //计算概率
    Random<uint32_t> r(1,1000);
    uint32_t rand = r.get();
    if(sptr->trigger_per < rand && !atk->issetDoubleDamageSkillLogic(getID(), sptr->logic_id))
    {
        return;
    }
    else if(sptr->trigger_per >= rand && 
            (11 == sptr->logic_id || 15 == sptr->logic_id)) //计算下一次二次伤害触发的概率
    {
        LOG_DEBUG("技能, 效果触发概率, rand:{}, sptr->trigger_per:{}", rand, sptr->trigger_per);
        atk->m_doubleDamageFlag = 1;
    }

    see.logic_id = sptr->logic_id;
    see.target = sptr->target_type;
    see.sec = sptr->param1;

    see.param1 = sptr->param1;
    see.param2 = sptr->param2;
    see.param3 = sptr->param3;
    see.param4 = sptr->param4;

    if(see.kind == skill_kind::summon && m_changedPetId > 0)
        see.param1 = m_changedPetId;
    if(see.kind == skill_kind::firewall)
        see.fireAddSec = m_fireAddSec;

    //这里处理改变技能效果的作用范围
    Range rangetype = sptr->range_type;
    uint16_t finalRangeParam1 = sptr->range_param1;
    uint16_t finalRangeParam2 = sptr->range_param2;
    if(Range::none != m_changedRangeType)
    {
        rangetype = m_changedRangeType;
        finalRangeParam1 = m_changedRangeParam1;
        finalRangeParam2 = m_changedRangeParam2;
    }
    see.radius = finalRangeParam1;

    if(see.kind == skill_kind::chongfeng 
       || see.kind == skill_kind::passiveDamage
       || see.kind == skill_kind::resistfire
       || see.kind == skill_kind::firewall
       || see.target == TargetType::self)
    {
        atk->m_skillEffectM.addSkillEffect(see);
        return;
    }

    Scene::Ptr s = atk->scene();
    if(nullptr == s)
        return;

    switch(rangetype)
    {
    case Range::one:
        {
            if(!checkDistance(see.att_pos, see.pos, getMaxDistance()))
            {
                LOG_DEBUG("技能, id:{} 超出射程, 攻击者坐标:{}, 目标坐标:{}, 射程:{}", 
                          getID(), see.att_pos, see.pos, getMaxDistance());
                return;
            }
            putSkillEffectInGrid(s->getGridByGridCoord(see.pos), see);
        }
        break;
    case Range::circle:
        {
            if(!checkDistance(see.att_pos, see.pos, getMaxDistance()))
            {
                LOG_DEBUG("技能, id:{} 超出射程, 攻击者坐标:{}, 目标坐标:{}, 射程:{}", 
                          getID(), see.att_pos, see.pos, getMaxDistance());
                return;
            }
            putSkillEffectInCircle(s, see);
        }
        break;
    case Range::line:
        {
            Coord2D pos(see.att_pos);
            for(uint32_t i = 0; i < finalRangeParam1; ++i)
            {
                pos = pos.neighbor(see.dir);
                putSkillEffectInGrid(s->getGridByGridCoord(pos), see);
                LOG_DEBUG("技能, line range, att_pos:{}, pos:{}", see.att_pos, pos);
            }
        }
        break;
    case Range::rect:
        {//暂时定义攻击者为技能落点
            Coord2D pos(see.att_pos);
            std::vector<Direction> verticaldir = verticalDir(see.dir);
            for(uint32_t i = 0; i <= finalRangeParam1; ++i)
            {
                LOG_DEBUG("技能, rect range, att_pos:{}, pos:{}", see.att_pos, pos);
                putSkillEffectInGrid(s->getGridByGridCoord(pos), see);
                for(const auto& it : verticaldir)
                {
                    Coord2D tmp(pos);
                    for(uint32_t j = 0; j < finalRangeParam2; ++j)
                    {
                        tmp = tmp.neighbor(it);
                        putSkillEffectInGrid(s->getGridByGridCoord(tmp), see);
                        LOG_DEBUG("技能, rect range, att_pos:{}, pos:{}", see.att_pos, tmp);
                    }
                }
                pos = pos.neighbor(see.dir);
            }
        }
        break;
    case Range::order:
        {
            Coord2D pos(see.att_pos);
            for(uint32_t i = 0; i < finalRangeParam1; ++i)
                pos = pos.neighbor(see.dir);

            putSkillEffectInGrid(s->getGridByGridCoord(pos), see);
        }
        break;
    default:
        break;
    }
}


bool Skill::checkTarget(PK::Ptr atk, PK::Ptr def, const TargetType& target_type)
{
    if(target_type != TargetType::friendly)
    {
        if(!PK::isEnemy(atk, def))
            return false;
    }

    switch(target_type)
    {
    case TargetType::all:
        {
            return true;
        }
        break;
    case TargetType::friendly:
        {
            return def->isFriend(atk);
        }
        break;
    case TargetType::mob:
        {
            if(def->sceneItemType() == SceneItemType::npc)
            {
                Npc::Ptr npc = std::static_pointer_cast<Npc>(def);
                return  npc->type() != NpcType::boss;
            }
        }
        break;
    case TargetType::Boss:
        {
            if(def->sceneItemType() == SceneItemType::npc)
            {
                Npc::Ptr npc = std::static_pointer_cast<Npc>(def);
                return npc->type() == NpcType::boss;
            }
        }
        break;
    case TargetType::allnpc:
        {
            return def->sceneItemType() == SceneItemType::npc;
        }
        break;
    case TargetType::role: //非npc
        {
            return def->sceneItemType() != SceneItemType::npc;
        }
        break;
    default:
        break;
    }

    return false;
}







//*********************技能管理开始**********************
SkillManager::SkillManager(PK& me)
    :m_owner(me)
{
}


void SkillManager::loadFromDB(std::vector<SkillData>& data)
{
    for(const auto& iter : data)
    {
        LOG_DEBUG("技能加载, id={}, level={}, strength_level={}", iter.skillId, iter.skillLv, iter.strengthenLv);
        initSkill(iter);
    }
}


void SkillManager::updateToDB(Skill::Ptr sp) const
{
    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PrivateRaw::ModifySkillData));
    auto send = reinterpret_cast<PrivateRaw::ModifySkillData*>(buf.data());
    send->modifyType = ModifyType::modify;
    send->roleId = m_owner.getOwnerId();
    send->sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    send->job = m_owner.job();
    buf.resize(buf.size() + sizeof(SkillData));
    send->data[0].skillId = sp->getID();
    send->data[0].skillLv = sp->getLevel();
    send->data[0].strengthenLv = sp->getStrengthenLv();
    send->data[0].exp = sp->getExp();
    send->size = 1;

	ProcessIdentity dbcachedId("dbcached", 1);
	World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(ModifySkillData), buf.data(), buf.size());
}



bool SkillManager::initSkill(TplId id, uint32_t level/*=1*/)
{
    if(0 == id || m_activeSkill.find(id) != m_activeSkill.end())
        return false;
    Skill::Ptr sp = Skill::create(id, level);
    if(nullptr == sp)
    {
        LOG_TRACE("初始化技能失败{id}", id);
        return false;
    }

    if(!addSkill(sp))
    {
        LOG_TRACE("添加技能失败{id}", id);
        return false;
    }

    initSkillEffect(sp);
    return true;
}


bool SkillManager::initSkill(const SkillData& data)
{
    if(m_activeSkill.find(data.skillId) != m_activeSkill.end())
        return false;
    Skill::Ptr sp = Skill::create(data.skillId, data.skillLv);
    if(nullptr == sp)
    {
        LOG_TRACE("初始化技能失败{id}", data.skillId);
        return false;
    }
    sp->setExp(data.exp);
    sp->setStrengthenLv(data.strengthenLv);

    if(!addSkill(sp))
    {
        LOG_TRACE("添加技能失败{id}", data.skillId);
        return false;
    }

    initSkillEffect(sp);
    return true;
}


bool SkillManager::addSkill(Skill::Ptr s)
{
    removeSkill(s);
    return m_activeSkill.insert(std::make_pair(s->getID(), s)).second;
}


void SkillManager::removeSkill(Skill::Ptr s)
{
    if(nullptr == s)
        return;
    removeSkill(s->getID());
}



void SkillManager::removeSkill(TplId id)
{
    m_activeSkill.erase(id);
}



Skill::Ptr SkillManager::find(TplId id)
{
    if(m_activeSkill.find(id) == m_activeSkill.end())
        return nullptr;

    return m_activeSkill[id];
}



/*
 * 初始化技能效果
 */
void SkillManager::initSkillEffect(Skill::Ptr sp)
{
    if(sp->getKind() == skill_kind::summon)
    {
        m_owner.setPetSkillId(sp->getID());
        m_owner.setPetLevel(sp->getLevel());
    }

    sp->initEffect();
    sp->initStrengthenEffect();
}


/*
 * 开放合击技能
 */
void SkillManager::openJointSkill()
{
    auto openJointExec = [&, this] (SkillBase::Ptr skillptr)
    {
        if(nullptr == skillptr)
            return;
        if(skillptr->kind != skill_kind::joint)
            return;
        if(m_owner.job() == skillptr->jobs.first)
        {
            if(initSkill(skillptr->id))
            {
                LOG_DEBUG("合击, 开放合击技能, id:{}", skillptr->id);
                refreshSkill(skillptr->id);
            }
        }
    };

    skillCT.execAll(openJointExec);
}


/*
 * 解锁技能
 */
void SkillManager::unlockSkill(bool refresh/*=true*/)
{
    auto unlockExec = [&, this] (SkillBase::Ptr skillptr)
    {
        if(nullptr == skillptr || m_owner.sceneItemType() != skillptr->sceneItem)
            return;
        if(m_owner.level() < skillptr->role_level)
            return;
        if(m_owner.job() == skillptr->jobs.first)
        {
            if(initSkill(skillptr->id))
            {
                LOG_DEBUG("技能, 解锁技能, id:{}", skillptr->id);
                if(refresh)
                    refreshSkill(skillptr->id);
            }
        }
    };

    skillCT.execAll(unlockExec);
}


void SkillManager::sendSkillListToMe() const
{
    std::vector<uint8_t> buf;
    buf.reserve(512);
    buf.resize(sizeof(PublicRaw::SkillListToClient));
    auto msg = reinterpret_cast<PublicRaw::SkillListToClient*>(buf.data());
    msg->sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    msg->job = m_owner.job();
    for(const auto& iter : m_activeSkill)
    {
        if(iter.second->getKind() == skill_kind::normal)
            continue;
        buf.resize(buf.size()+sizeof(SkillData));
        auto msg = reinterpret_cast<PublicRaw::SkillListToClient*>(buf.data());
        msg->data[msg->size].skillId = iter.second->getID();
        msg->data[msg->size].skillLv = iter.second->getLevel();
        msg->data[msg->size].strengthenLv = iter.second->getStrengthenLv();
        msg->data[msg->size].exp = iter.second->getExp();
        ++msg->size;
    }

    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(SkillListToClient), buf.data(), buf.size());
}


void SkillManager::refreshSkill(Skill::Ptr sp) const
{
    PublicRaw::RefreshSkill refresh;
    refresh.data.skillId = sp->getID();
    refresh.data.skillLv = sp->getLevel();
    refresh.data.strengthenLv = sp->getStrengthenLv();
    refresh.data.exp = sp->getExp();
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshSkill), &refresh, sizeof(refresh));
}


void SkillManager::refreshSkill(TplId id) const
{
    auto iter = m_activeSkill.find(id);
    if(iter == m_activeSkill.end())
        return;

    const auto skill = iter->second;
    refreshSkill(skill);
}


void SkillManager::upgradeSkill(TplId id, PK::Ptr owner, uint32_t upNum/*=1*/, bool GmFlag/*=false*/)
{
    Role::Ptr role = getRole(owner);
    if(nullptr == role)
        return;
    Skill::Ptr newSkill = nullptr;
    if(m_activeSkill.find(id) == m_activeSkill.end())
    {
        return;
        /* 预留, 防止策划YY又要学习技能
        //LOG_ERROR("技能升级, 找不到该技能,id={}", id);
        //新技能学习
        const SkillBase::Ptr newPtr = skillCT.get(skill_hash(id, upNum));
        if(nullptr == newPtr)
        {
            LOG_ERROR("技能升级, 找不到技能配置, id={}, level=1", id);
            return;
        }

        newSkill = Skill::create(id, upNum);
        if(nullptr == newSkill)
            return;

        if(!GmFlag && !newSkill->checkUpgradeSkill(owner, newPtr))
            return;
        if(!addSkill(newSkill))
        {
            LOG_ERROR("技能升级, addSkill失败, id={},level=1", id);
            return;
        }

        if(!GmFlag)
        {
            for(const auto& iter : newPtr->consumes)
                role->m_packageSet.eraseObj(iter.first, iter.second, PackageType::role, "技能升级");
        }
        initSkillEffect(newSkill);
        updateToDB(newSkill);
        if(owner->sceneItemType() == SceneItemType::hero)
            role->sendSysChat("英雄成功学习技能{}", newSkill->getName());
        else
            role->sendSysChat("主角成功学习技能{}", newSkill->getName());
        */
    }
    else
    {
        const Skill::Ptr oldSkill = m_activeSkill[id];
        const SkillBase::Ptr nextptr = skillCT.get(skill_hash(id, oldSkill->getLevel()+upNum));
        if(nullptr == nextptr)
        {
            role->sendSysChat("技能已经升到满级");
            return;
        }
        if(!GmFlag && !oldSkill->checkUpgradeSkill(owner, nextptr))
            return;
        newSkill = Skill::create(id, oldSkill->getLevel()+upNum);
        if(nullptr == newSkill)
            return;
        uint32_t oldExp = oldSkill->getExp();
        uint32_t oldStrengthenLv = oldSkill->getStrengthenLv();
        if(!addSkill(newSkill))
        {
            LOG_ERROR("技能升级, addSkill失败, id={},level={}", id, newSkill->getLevel());
            return;
        }

        if(!GmFlag)
        {
            for(const auto& iter : nextptr->consumes)
                role->m_packageSet.eraseObj(iter.first, iter.second, PackageType::role, "技能升级");
        }
        if(!GmFlag)
            newSkill->setExp(oldExp-nextptr->needexp);
        newSkill->setStrengthenLv(oldStrengthenLv);
        initSkillEffect(newSkill);
        updateToDB(newSkill);
        if(owner->sceneItemType() == SceneItemType::hero)
        {
            role->sendSysChat("英雄成功将技能{}提升至{}级", newSkill->getName(), newSkill->getLevel());
            if(newSkill->getLevel() >= 4)
                role->sendSysChat(ChannelType::screen_middle, "恭喜玩家{}的英雄经过不懈努力将{}(技能)提升到{}级", 
                                  role->name(), newSkill->getName(), newSkill->getLevel());
        }
        else
        {
            role->sendSysChat("主角成功将技能{}提升至{}级", newSkill->getName(), newSkill->getLevel());
            if(newSkill->getLevel() >= 4)
                role->sendSysChat(ChannelType::screen_middle, "恭喜玩家{}经过不懈努力将{}(技能)提升到{}级", 
                                  role->name(), newSkill->getName(), newSkill->getLevel());
        }
    }

    refreshSkill(newSkill);
}



void SkillManager::strengthenSkill(TplId id, PK::Ptr owner)
{
    Role::Ptr role = getRole(owner);
    if(nullptr == role)
        return;
    if(m_activeSkill.find(id) == m_activeSkill.end())
    {
        LOG_ERROR("技能强化, 找不到该技能id={}", id);
        return;
    }

    const Skill::Ptr skill = m_activeSkill[id];
    const uint32_t oldStrengthenLv = skill->getStrengthenLv();
    const SkillStrengthenBase::Ptr nextptr = skillStrengthenCT.get(skill_strengthen_hash(id, oldStrengthenLv+1));
    if(nullptr == nextptr)
    {
        role->sendSysChat("该技能已强化到最高星级");
        return;
    }
    if(!skill->checkStrengthenSkill(owner, nextptr))
        return;

    for(const auto& iter : nextptr->consumes)
        role->m_packageSet.eraseObj(iter.first, iter.second, PackageType::role, "技能强化");
    skill->deleteStrengthenEffect();
    skill->setStrengthenLv(oldStrengthenLv+1);
    skill->initStrengthenEffect();
    updateToDB(skill);

    if(owner->sceneItemType() == SceneItemType::hero)
    {
        role->sendSysChat("英雄成功将技能{}强化到{}星", skill->getName(), skill->getStrengthenLv());
        if(skill->getStrengthenLv() >= 4)
            role->sendSysChat(ChannelType::screen_middle, "恭喜玩家{}的英雄经过不懈努力将{}(技能)强化到+{}", 
                              role->name(), skill->getName(), skill->getStrengthenLv());
    }
    else
    {
        role->sendSysChat("主角成功将技能{}强化到{}星", skill->getName(), skill->getStrengthenLv());
        if(skill->getStrengthenLv() >= 4)
            role->sendSysChat(ChannelType::screen_middle, "恭喜玩家{}经过不懈努力将{}(技能)强化到+{}", 
                              role->name(), skill->getName(), skill->getStrengthenLv());
    }
    refreshSkill(skill);
}


bool SkillManager::processPassiveSkill(const skill_kind& kind)
{
    PK::Ptr atk = PK::getPkptr(m_owner.id(), m_owner.sceneItemType());
    if(nullptr == atk)
        return false;
    auto callback = [&, this] (Skill::Ptr s) -> bool
    {
        if(nullptr == s || s->getKind() != kind)
            return false;

        if(!checkPassiveSkillCD(s->getID(), s->getCDTime()))
        {
            //LOG_DEBUG("被动技能CD中, 伤害类,id={}, cdtime={} ~~~~~~~~~~~~~~~", s->getID(), s->getCDTime());
            return false;
        }

        if(s->processPassive(atk))
        {
            updatePassiveSkillCD(s->getID());
            return true;
        }

        return false;
    };

    for(auto& iter : m_passiveSkill)
    {
        if(callback(iter.second))
            return true;
    }
    return false;
}

bool SkillManager::checkPassiveSkillCD(TplId id, uint32_t cd)
{
    if(m_cdSkills.find(id) == m_cdSkills.end())
        return true;

    return m_cdSkills[id] + cd/1000 <= toUnixTime(Clock::now());
}

void SkillManager::updatePassiveSkillCD(TplId id)
{
    m_cdSkills[id] = toUnixTime(Clock::now());
}

void SkillManager::insertPassiveSkillCD(const PKCdStatus& status)
{
    m_cdSkills.insert({status.skillId, status.endtime});
}

void SkillManager::leaveScene()
{
    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PrivateRaw::CachePKCdStatus));
    auto msg = reinterpret_cast<PrivateRaw::CachePKCdStatus*>(buf.data());
    msg->roleId = m_owner.getOwnerId();
    for(const auto& iter : m_cdSkills)
    {
        buf.resize(buf.size() + sizeof(PKCdStatus));
        msg = reinterpret_cast<PrivateRaw::CachePKCdStatus*>(buf.data());
        msg->data[msg->size].skillId = iter.first;
        msg->data[msg->size].endtime = iter.second;
        ++msg->size;
    }
	ProcessIdentity dbcachedId("dbcached", 1);
    if(msg->size)
	    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(CachePKCdStatus), buf.data(), buf.size());

    //保存经验
    std::vector<uint8_t> exp_buf;
    exp_buf.reserve(128);
    exp_buf.resize(sizeof(PrivateRaw::ModifySkillData));
    auto exp_msg = reinterpret_cast<PrivateRaw::ModifySkillData*>(exp_buf.data());
    exp_msg->roleId = m_owner.getOwnerId();
    exp_msg->modifyType = ModifyType::modify;
    exp_msg->sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    exp_msg->job = m_owner.job();
    for(const auto& iter : m_activeSkill)
    {
        if(iter.second->getKind() == skill_kind::normal)
            continue;
        exp_buf.resize(exp_buf.size() + sizeof(SkillData));
        exp_msg = reinterpret_cast<PrivateRaw::ModifySkillData*>(exp_buf.data());
        const Skill::Ptr& sp = iter.second;
        exp_msg->data[exp_msg->size].skillId = sp->getID();
        exp_msg->data[exp_msg->size].skillLv = sp->getLevel();
        exp_msg->data[exp_msg->size].strengthenLv = sp->getStrengthenLv();
        exp_msg->data[exp_msg->size].exp = sp->getExp();
        ++exp_msg->size;
    }
    if(exp_msg->size)
	    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(ModifySkillData), exp_buf.data(), exp_buf.size());
}


void SkillManager::putEquipPassiveSkill(std::vector<TplId>& ids)
{
    m_passiveSkill.clear();
    for(const auto& iter : ids)
    {
        if(m_passiveSkill.find(iter) != m_passiveSkill.end())
            continue;
        Skill::Ptr sp = Skill::create(iter, 1);
        if(nullptr == sp)
        {
            LOG_TRACE("初始化被动技能失败{}", iter);
            continue;
        }
        
        if(sp->getType() != SkillType::passive)
            continue;

        if(!m_passiveSkill.insert(std::make_pair(iter, sp)).second)
        {
            LOG_TRACE("添加被动技能失败{}", iter);
            continue;
        }

        sp->initEffect();
        LOG_DEBUG("被动技能, 添加成功, id={}", iter);
    }
}

}

