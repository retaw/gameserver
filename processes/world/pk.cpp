#include "pk.h"

#include "world.h"
#include "pk_cfg.h"
#include "role_manager.h"
#include "npc_manager.h"
#include "hero_ids.h"
#include "scene_manager.h"
#include "pet_manager.h"
#include "world_boss.h"
#include "massive_config.h"
#include "first_manager.h"

#include "water/componet/random.h"
#include "protocol/rawmsg/rawmsg_manager.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"

namespace world{


using namespace std;
using namespace water::componet;


//放大倍数
#define ENLARGE 1000

PK::PK(PKId id, const std::string& name, const Job& job, const SceneItemType& sceneItem)
: m_skillM(*this)
, m_skillEffectM(*this)
, m_pkstate(*this)
, m_buffM(*this)
, m_id(id)
, m_name(name)
, m_job(job)
, m_sceneItem(sceneItem)
, m_lastSkillTime(EPOCH)
, m_hp(0)
, m_mp(0)
, m_shotk(0)
, m_critk(0)
, m_feignDead(false)
, m_feignDeadRelivePercent(0)
, m_fightTime(EPOCH)
, m_petId(0)
, m_petTplId(0)
, m_petLevel(0)
, m_eraseFlag(false)
, m_jointStart(EPOCH)
, m_lastAttackSuccessSkill(0)
, m_doubleDamageFlag(0)
, m_doubleDamageLogic(0)
, m_demaxiya(false)
, m_wudi(false)
{
}


PKId PK::id() const
{
    return m_id;
}


const std::string& PK::name() const
{
    return m_name;
}


const Job& PK::job() const
{
    return m_job;
}

void PK::setLevel(uint32_t level)
{
    m_level = level;

    KValueBase::Ptr kp = kCT.get(level);
    if(nullptr == kp)
        return;

    m_shotk = kp->shotk;
    m_critk = kp->critk;
}


uint32_t PK::level() const
{
    return m_level;
}

void PK::setTurnLifeLevel(TurnLife level)
{
	m_turnLifeLevel = level;
}

TurnLife PK::turnLifeLevel() const
{
	return m_turnLifeLevel;
}

SceneItemType PK::sceneItemType() const
{
    return m_sceneItem;
}

PKId PK::getOwnerId() const
{
    return m_id;
}

SceneItemType PK::getOwnerSceneItemType() const
{
    return m_sceneItem;
}

Job PK::getOwnerJob() const
{
    return m_job;
}


void PK::setDir(componet::Direction dir)
{
    m_dir = dir;
}

componet::Direction PK::dir() const
{
    return m_dir;
}

void PK::setSpeed(uint32_t speed)
{
    m_speed = speed;
}

uint32_t PK::getSpeed() const 
{
    return m_speed;
}

void PK::setPos(Coord2D pos)
{
    m_pos = pos;
}

Coord2D PK::pos() const
{
    return m_pos;
}

Coord2D PK::backPos() const
{
    return m_pos.neighbor(componet::reverseDirection(m_dir));
}

void PK::setSceneId(SceneId id)
{
    m_sceneId = id;
}

SceneId PK::sceneId() const
{
    return m_sceneId;
}

Scene::Ptr PK::scene() const                       
{                                                    
    return SceneManager::me().getById(m_sceneId);    
}                                                    

bool PK::checkPublicCDTime() const
{
    using namespace std::chrono;
    if(Clock::now() < m_lastSkillTime + milliseconds {Massive::me().m_publicSkillCD})
    {
        /*LOG_DEBUG("技能public_cdtime中 atkName={}, now={}, lasttime={}, public_cd={} ~~~~~~~~", 
                  atk->name(), duration_cast<milliseconds>(nowtp-EPOCH).count(), duration_cast<milliseconds>(lasttp-EPOCH).count(), Massive::me().m_publicSkillCD);
        */
        return false;
    }
    return true;
}

void PK::setSkillPublicTime()
{
    m_lastSkillTime = Clock::now();
}

TimePoint PK::getSkillPublicTime() const
{
    return m_lastSkillTime;
}

void PK::clearSkillCD(uint32_t skillId)
{
    auto pskill = m_skillM.find(skillId);
    if(nullptr == pskill)
        return;

    pskill->clearCD(shared_from_this());
}

uint32_t PK::shotk() const
{
    return m_shotk;
}

uint32_t PK::critk() const
{
    return m_critk;
}

void PK::setHp(uint32_t dwHp)
{
    m_hp = dwHp;
}

void PK::setMp(uint32_t dwMp)
{
    m_mp = dwMp;
}

void PK::resetHpMp()
{
    uint32_t maxHp = getMaxHp();
    uint32_t maxMp = getMaxMp();
    m_hp = m_hp > maxHp ? maxHp : m_hp;
    m_mp = m_mp > maxMp ? maxMp : m_mp;
}

void PK::timerLoop(StdInterval interval, const water::componet::TimePoint& now)
{
    switch(interval)
    {
    case StdInterval::msec_100:
        break;
    case StdInterval::msec_500:
        m_skillEffectM.timerExec();
        break;
    case StdInterval::sec_1:
        m_buffM.timerExec(now);
        break;
    default:
        break;
    }
}

bool PK::isFeignDead() const
{
    return m_feignDead;
}

void PK::setFeignDead()
{
    m_feignDead = true;
}

void PK::clearFeignDead()
{
    m_feignDead = false;
}

void PK::setFeignDeadRelivePercent(uint32_t percent)
{
    m_feignDeadRelivePercent = percent;
}

uint32_t PK::feignDeadRelivePercent() const
{
    return m_feignDeadRelivePercent;
}

bool PK::dealFeignDead()
{
    return m_skillM.processPassiveSkill(skill_kind::passiveRelive);
}

void PK::markErase(bool flag)
{
    m_eraseFlag = flag;
}

bool PK::needErase() const
{
    return m_eraseFlag;
}


/*
 * 生命值
 */
uint32_t PK::getHp() const
{
    return m_hp;
}


/*
 * 魔法值 
 */
uint32_t PK::getMp() const
{
    return m_mp;
}


bool PK::changeHp(int32_t dwHp)
{
    if(0 == dwHp)
        return false;
    int32_t oldHp = getHp();
    if(oldHp+dwHp > 0)
    {
        if(oldHp+dwHp > (int32_t)getMaxHp())
        {
            setHp(getMaxHp());
        }
        else
        {
            setHp(oldHp+dwHp);
        }
    }
    else
    {
        setHp(0);
    }

    return (uint32_t)oldHp != getHp();
}


bool PK::changeMp(int32_t dwMp)
{
    if(0 == dwMp)
        return false;
    int32_t oldMp = getMp();
    if(oldMp+dwMp > 0)
    {
        if(oldMp+dwMp > (int32_t)getMaxMp())
        {
            setMp(getMaxMp());
        }
        else
        {
            setMp(oldMp+dwMp);
        }
    }
    else
    {
        setMp(0);
    }

    return (uint32_t)oldMp == getMp();
}


bool PK::sendToMe(TcpMsgCode msgCode) const
{
    return true;
}


bool PK::sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
    return true;
}


void PK::syncScreenDataTo9() const
{
}


void PK::sendMainToMe()
{
}


AttackRetcode PK::launchAttack(uint32_t skillId, Coord2D center)
{
    PublicRaw::RoleRequestAttack attackInfo;
    attackInfo.posX = center.x;
    attackInfo.posY = center.y;
    attackInfo.dir = uint8_t(pos().direction(center));
    attackInfo.step = 2;
    attackInfo.skillId = skillId;

    return launchAttack(&attackInfo);
}


AttackRetcode PK::launchAttack(const PublicRaw::RoleRequestAttack* rev)
{
    if(isDead() || needErase())
        return AttackRetcode::unknown;

	if(FirstManager::me().isActionMap(scene()) && FirstManager::me().isActionTimeReady())
		return AttackRetcode::unknown;

    if(!(rev->step == 1 && m_lastAttackSuccessSkill == rev->skillId)
       && !checkPublicCDTime())
    {
        retAttackFail(rev->skillId, AttackRetcode::pubCDTime);
        return AttackRetcode::pubCDTime;
    }

    auto pskill = m_skillM.find(rev->skillId);
    if(nullptr == pskill)
    {
        LOG_TRACE("[攻击],{},{},技能不存在,id={}", name(),id(),rev->skillId);
        return AttackRetcode::unknown;
    }

    handleInterrupt(interrupt_operate::attack, sceneItemType());
    AttackRetcode ret = pskill->action(shared_from_this(), rev);
    if(ret != AttackRetcode::success)
    {
        retAttackFail(rev->skillId, ret);
        LOG_TRACE("[攻击],{},{},攻击返回错误码,ret={}",name(),id(),ret);
    }

    return ret;
}

/*
 *
 *攻防伤害计算入口
 * continued:是否为造成的持续伤害
 *
 */
void PK::attackMe(PK::Ptr atk, uint8_t hit, const skill_kind& kind, uint8_t continued)
{
    if(nullptr == atk)
        return;

    auto s = scene();
    if(nullptr == s)
        return;

    if(atk->isDead() || isDead() || atk->needErase() || needErase())
        return;

    handleInterrupt(interrupt_operate::beAttack, atk->sceneItemType());
    //恶意攻击置灰名
    if(s->crime())
        atk->setGreyName(shared_from_this());

    int32_t DM = 0;
    HPChangeType hpType = HPChangeType::normal;

    switch(kind)
    {
    case skill_kind::joint:
        {
            if(atk->sceneItemType() != SceneItemType::role)
                return;
            hpType = calcJointSkilldamage(atk, hit, kind, &DM);
        }
        break;
    default:
        hpType = calcSkillDamage(atk, hit, kind, &DM);
        break;
    }

    changeHpAndNotify(atk, -DM, hpType, continued);
    underAttack(atk);
}


void PK::underAttack(PK::Ptr atk)
{
    return;
}

bool PK::isShot(PK::Ptr atk, const skill_kind& kind)
{
    for(const auto& iter : atk->m_attackHpChangeList)
    {
        if(iter.first == shared_from_this())
        {
            return iter.second.type != HPChangeType::escape;
        }
    }

    int64_t temp = 0;
    int64_t min_temp = 0;
    int64_t g_shot = atk->getTotalShot();
    int64_t g_shotRatio = atk->getTotalShotRatio();
    int64_t d_escape = getTotalPEscape();
    if(kind != skill_kind::normal && atk->job() != Job::warrior)
        d_escape = getTotalMEscape();
    int64_t d_escapeRatio = getTotalEscapeRatio();

    //有效命中率
    int64_t denominator = std::abs(g_shot * PK_PARAM.shotParam2() - d_escape * PK_PARAM.shotParam3()) + ENLARGE * std::max(atk->shotk(), shotk());
    if(0 == denominator)
        return true;
    temp = (int64_t)(PK_PARAM.shotParam1() * ENLARGE + g_shotRatio * ENLARGE - d_escapeRatio * ENLARGE + (g_shot * PK_PARAM.shotParam2() - d_escape * PK_PARAM.shotParam3()) * PK_PARAM.shotParam4() / denominator);
    min_temp = std::min<int64_t>(temp, 1000);
    uint64_t finalSHot = std::max<int64_t>(min_temp, PK_PARAM.shotParam5());

    Random<uint64_t> r(1,1000);
    uint64_t rand = r.get();
    return finalSHot >= rand;
}


uint32_t PK::constDamage(PK::Ptr atk, bool joint)
{
    uint32_t ret = atk->m_pkvalue.m_constDamage;
    if(sceneItemType() == SceneItemType::role)
        ret += atk->m_pkvalue.m_roleConstDamage;
    else if(sceneItemType() == SceneItemType::npc)
    {
        Npc::Ptr npc = std::static_pointer_cast<Npc>(shared_from_this());
        if(npc->type() == NpcType::boss)
            ret += atk->m_pkvalue.m_bossConstDamage;
        else
            ret += atk->m_pkvalue.m_mobConstDamage;
    }

    //处理合击炼心技能
    if(joint)
    {
        Role::Ptr atkRole = getRole(atk);
        if(nullptr != atkRole)
        {
            if(atk->sceneItemType() == SceneItemType::role)
            {
                if(sceneItemType() == SceneItemType::npc)
                    ret += atkRole->m_dragonHeart.rolePveConstDamage();
                else
                    ret += atkRole->m_dragonHeart.rolePvpConstDamage();
            }
            else if(atk->sceneItemType() == SceneItemType::hero)
            {
                if(sceneItemType() == SceneItemType::npc)
                    ret += atkRole->m_dragonHeart.heroPveConstDamage();
                else
                    ret += atkRole->m_dragonHeart.heroPvpConstDamage();
            }
        }
    }

    return ret;
}

uint64_t PK::calcFinalAtt(PK::Ptr atk, const skill_kind& kind)
{
    int64_t min_temp = 0;
    int64_t max_temp = 0;
    
    int64_t g_atkMin, g_atkMax;
    //平砍统一走物攻
    if(kind == skill_kind::normal)
    {
        g_atkMin = ENLARGE * atk->getTotalPAtkMin();
        g_atkMax = ENLARGE * atk->getTotalPAtkMax();
    }
    else
    {
        if(Job::warrior == atk->job())//战士
        {
            g_atkMin = ENLARGE * atk->getTotalPAtkMin();
            g_atkMax = ENLARGE * atk->getTotalPAtkMax();
        }
        else if(Job::magician == atk->job())//法师
        {
            g_atkMin = ENLARGE * atk->getTotalMAtkMin();
            g_atkMax = ENLARGE * atk->getTotalMAtkMax();
        }
        else//道术
        {
            g_atkMin = ENLARGE * atk->getTotalWitchMin();
            g_atkMax = ENLARGE * atk->getTotalWitchMax();
        }
    }
    int64_t g_lucky = ENLARGE * atk->getTotalLucky();
    int64_t d_evil = ENLARGE * getTotalEvil();
    //有效攻击
    max_temp = std::max<int64_t>((g_atkMax-g_atkMin) * std::min<int64_t>(ENLARGE * (g_lucky-d_evil)/PK_PARAM.atkParam(), ENLARGE) /ENLARGE, 0);
    min_temp = std::min<int64_t>((g_atkMax-g_atkMin) * std::max<int64_t>(ENLARGE * (g_lucky-d_evil)/PK_PARAM.atkParam(), -ENLARGE) / ENLARGE, 0);
    Random<int64_t> r_att((int64_t)g_atkMin + max_temp, (int64_t)g_atkMax + min_temp);
    uint64_t att = r_att.get();
    LOG_DEBUG("rand攻击, 有效攻击:{}, max_temp:{}, min_temp:{}", att, max_temp, min_temp);
    return att;
}

uint64_t PK::calcFinalDef(PK::Ptr atk, const skill_kind& kind)
{
    int64_t min_temp = 0;
    int64_t max_temp = 0;
    
    int64_t d_defMin, d_defMax;
    int64_t def_param = PK_PARAM.mdefParam();
    //平砍统一走物防
    if(kind == skill_kind::normal)
    {
        d_defMin = ENLARGE * getTotalPDefMin();
        d_defMax = ENLARGE * getTotalPDefMax();
        def_param = PK_PARAM.pdefParam();
    }
    else
    {
        if(Job::warrior == atk->job())//战士
        {
            d_defMin = ENLARGE * getTotalPDefMin();
            d_defMax = ENLARGE * getTotalPDefMax();
            def_param = PK_PARAM.pdefParam();
        }
        else
        {
            d_defMin = ENLARGE * getTotalMDefMin();
            d_defMax = ENLARGE * getTotalMDefMax();
        }
    }

    int64_t g_evil = ENLARGE * atk->getTotalEvil();
    int64_t d_lucky = ENLARGE * getTotalLucky();
    //有效防御
    max_temp = std::max<int64_t>((d_defMax-d_defMin) * std::min<int64_t>(ENLARGE * (d_lucky-g_evil)/def_param, ENLARGE) / ENLARGE, 0);
    min_temp = std::min<int64_t>((d_defMax-d_defMin) * std::max<int64_t>(ENLARGE * (d_lucky-g_evil)/def_param, -ENLARGE) / ENLARGE, 0);
    Random<int64_t> r_def((int64_t)d_defMin + max_temp, (int64_t)d_defMax + min_temp);
    uint64_t def = r_def.get();
    LOG_DEBUG("rand防御, 有效防御:{}, max_temp:{}, min_temp:{}", def, max_temp, min_temp);
    return def;
}

uint64_t PK::calcFinalCritdmg(PK::Ptr atk)
{
    int64_t min_temp = 0;
    uint64_t critdmg = 0;

    int64_t g_crit = ENLARGE * atk->getTotalCrit();
    int64_t g_critRatio = atk->getTotalCritRatio();
    int64_t g_critDmg = atk->getTotalCritDmg();
    int64_t d_antiCrit = ENLARGE * getTotalAntiCrit();
    //有效暴击率
    min_temp = std::min<int64_t>(g_critRatio + (g_crit * PK_PARAM.critParam1() - d_antiCrit * PK_PARAM.critParam2()) * PK_PARAM.critParam3() / (std::abs(g_crit * PK_PARAM.critParam1() - d_antiCrit * PK_PARAM.critParam2()) + ENLARGE * ENLARGE * std::max(atk->critk(), critk())), PK_PARAM.critParam4());
    uint64_t finalCrit = std::max<int64_t>(min_temp, PK_PARAM.critParam5());
    Random<uint64_t> r(1,1000);
    uint64_t rand = r.get();
    if(rand <= finalCrit)
    {
        //有效暴伤
        critdmg = PK_PARAM.critParam6() + g_critDmg;
    }
    //LOG_DEBUG("有效暴伤:{}", critdmg);
    return critdmg;
}

uint64_t PK::calcFinalDmgadd(PK::Ptr atk, bool joint)
{
    int64_t temp = 0;
    int64_t max_temp = 0;

    int64_t g_dmgAdd = atk->getTotalDmgAdd() + atk->m_pkvalue.m_damageAddPer;
    if(sceneItemType() == SceneItemType::npc)
    {
        Npc::Ptr npc = std::static_pointer_cast<Npc>(shared_from_this());
        if(npc->type() == NpcType::boss)
            g_dmgAdd += atk->m_pkvalue.m_bossDamageAdd;
        else
            g_dmgAdd += atk->m_pkvalue.m_mobDamageAdd;
    }
    else
        g_dmgAdd += atk->m_pkvalue.m_roleDamageAdd;

    //合击技能
    if(joint)
    {
        Role::Ptr atkRole = getRole(atk);
        if(nullptr != atkRole)
        {
            if(atk->sceneItemType() == SceneItemType::role)
            {
                //然后再区分pve pvp,实在是觉得太傻逼了cehua
                if(sceneItemType() == SceneItemType::npc)
                    g_dmgAdd += atkRole->m_dragonHeart.rolepveDamageAdd();
                else
                    g_dmgAdd += atkRole->m_dragonHeart.rolePvpDamageAdd();
            }
            else if(atk->sceneItemType() == SceneItemType::hero)
            {
                if(sceneItemType() == SceneItemType::npc)
                    g_dmgAdd += atkRole->m_dragonHeart.heropveDamageAdd();
                else
                    g_dmgAdd += atkRole->m_dragonHeart.heroPvpDamageAdd();
            }
        }
    }
    int64_t g_dmgAddLv = ENLARGE * atk->getTotalDmgAddLv();
    int64_t d_dmgReduce = getTotalDmgReduce();
    int64_t d_dmgReduceLv = ENLARGE * getTotalDmgReduceLv();
    //有效伤害加成
    max_temp = std::max<int64_t>(std::abs(g_dmgAddLv-d_dmgReduceLv), 1000);
    temp = (int64_t)(g_dmgAdd - d_dmgReduce + (std::pow(std::abs(g_dmgAddLv-d_dmgReduceLv), PK_PARAM.dmgaddParam1()/ENLARGE)/PK_PARAM.dmgaddParam2() - PK_PARAM.dmgaddParam3()) * (g_dmgAddLv-d_dmgReduceLv) / max_temp);
    uint64_t dmgadd = std::max<int64_t>(temp, PK_PARAM.dmgaddParam4());
    LOG_DEBUG("有效伤害加成:g_dmgAdd={}, dmgadd={}", g_dmgAdd, dmgadd);
    return dmgadd;
}



/*
 * 伤害计算
 */
HPChangeType PK::calcSkillDamage(PK::Ptr atk, uint8_t hit, const skill_kind& kind, int32_t* DM)
{
    HPChangeType type = HPChangeType::normal;
    if(m_wudi || (0 == hit && !isShot(atk, kind)))
    {
        LOG_TRACE("[技能],攻击闪避{att_id},{att_name},{def_id},{def_name}",atk->id(), atk->name(), id(), name());
        return HPChangeType::escape;
    }


    int32_t DMParam = ENLARGE;
    int64_t g_atkMin = 0;
    switch(atk->job())
    {
    case Job::warrior:
        g_atkMin = ENLARGE * atk->getTotalPAtkMin();
        DMParam = PK_PARAM.finalWarriorParam();
        break;
    case Job::magician:
        g_atkMin = ENLARGE * atk->getTotalMAtkMin();
        DMParam = PK_PARAM.finalMagicParam();
        break;
    default:
        g_atkMin = ENLARGE * atk->getTotalWitchMin();
        DMParam = PK_PARAM.finalWitchParam();
        break;
    }

    if(kind == skill_kind::normal)
        DMParam = ENLARGE;
    int64_t finalAtt = calcFinalAtt(atk, kind);
    int64_t finalDef = calcFinalDef(atk, kind);
    int64_t finalCritdmg = calcFinalCritdmg(atk);
    int64_t finalDmgAdd = calcFinalDmgadd(atk, kind == skill_kind::joint);

    if(kind == skill_kind::assassinate)//战士刺杀
    {
        if(0 == atk->m_tempFinalAtk)
            atk->m_tempFinalAtk = finalAtt;
        else
            finalAtt = atk->m_tempFinalAtk;
    }

    uint32_t skillPower = atk->m_pkvalue.m_skillPower;
    uint32_t skillDamage = atk->m_pkvalue.m_skillDamage;
    if(kind == skill_kind::joint)//合击技能
    {
        Role::Ptr atkRole = getRole(atk);
        if(atk->sceneItemType() == SceneItemType::role)
        {
            skillPower += atkRole->m_dragonHeart.roleSkillPower();
            skillDamage += atkRole->m_dragonHeart.roleSkillDamage();
        }
        else
        {
            skillPower += atkRole->m_dragonHeart.heroSkillPower();
            skillDamage += atkRole->m_dragonHeart.heroSkillPower();
        }
    }

    *DM = std::max<int64_t>((finalAtt * DMParam * skillPower/(ENLARGE*ENLARGE) + skillDamage * ENLARGE - finalDef * std::max<int64_t>(ENLARGE-atk->m_pkvalue.m_ignorePdefencePer, 0)/ENLARGE) * finalDmgAdd * (ENLARGE+finalCritdmg)/(ENLARGE*ENLARGE), std::max<int64_t>(g_atkMin * PK_PARAM.finalParam()/ENLARGE, ENLARGE))/ENLARGE + constDamage(atk, kind == skill_kind::joint);

    LOG_TRACE("[技能],伤害属性, DM:{}, atk:{},{}, def:{},{}, finalAtt:{}, finalDef:{}, finalCritdmg:{}, finalDmgAdd:{}", *DM, atk->id(), atk->name(), id(), name(), finalAtt, finalDef, finalCritdmg, finalDmgAdd);

    if(finalCritdmg > 0)
        type = HPChangeType::crit;

    int32_t hp = getHp();
    if(atk->m_demaxiya)
        *DM = hp;
    else if(*DM > hp)
        *DM = hp;
    return type;
}



/*
 * 合击技能伤害计算
 */
HPChangeType PK::calcJointSkilldamage(PK::Ptr atk, uint8_t hit, const skill_kind& kind, int32_t* DM)
{
    int32_t roleDM = 0;
    int32_t heroDM = 0;
    HPChangeType hpType = HPChangeType::normal;

    Role::Ptr atkRole = std::static_pointer_cast<Role>(atk);
    if(nullptr == atkRole)
        return hpType;
    //hero 伤害
    Hero::Ptr atkHero = atkRole->m_heroManager.getSummonHero();
    if(nullptr != atkHero)
    {
        atkHero->m_pkvalue.m_skillPower = atk->m_pkvalue.m_skillPower;
        atkHero->m_pkvalue.m_skillDamage = atk->m_pkvalue.m_skillDamage;
        atkHero->m_pkvalue.m_damageAddPer = atk->m_pkvalue.m_damageAddPer;
        atkHero->m_pkvalue.m_constDamage = atk->m_pkvalue.m_constDamage;

        hpType = calcSkillDamage(atkHero, hit, kind, &heroDM);
    }

    //role 伤害
    HPChangeType roleHpType = calcSkillDamage(atk, hit, kind, &roleDM);
    *DM = roleDM + heroDM;
    return hpType > roleHpType ? hpType : roleHpType;
}



uint16_t PK::jointSkillReadyTime() const
{
    return 0;
}

void PK::setJointReadyState()
{
    m_jointStart = Clock::now();

    PublicRaw::NotifyReadyState notify;
    notify.clear = false;
    notify.readyTime = jointSkillReadyTime();
    sendToMe(RAWMSG_CODE_PUBLIC(NotifyReadyState), &notify, sizeof(notify));
}

void PK::clearJointReadyState()
{
    m_jointStart = EPOCH;

    PublicRaw::NotifyReadyState notify;
    notify.clear = true;
    notify.readyTime = 0;
    sendToMe(RAWMSG_CODE_PUBLIC(NotifyReadyState), &notify, sizeof(notify));

#ifdef WATER_DEBUG_HEROAI
    LOG_DEBUG("clearJointState");
#endif
}

bool PK::isJointReadyState() const
{
    return m_jointStart + std::chrono::seconds{jointSkillReadyTime()} >= Clock::now();
}


/*
 * 技能对应的职业
 */
bool PK::canJointSkillReady(const std::pair<Job, Job>& jobs)
{
    if(isJointReadyState())
    {
        LOG_DEBUG("合击, 上一个合击技能还未释放完毕");
        return false;
    }
    Role::Ptr atkRole = getRole(shared_from_this());
    if(nullptr == atkRole)
        return false;
    Hero::Ptr hero = atkRole->m_heroManager.getSummonHero();
    if(nullptr == hero)
    {
        LOG_DEBUG("合击, 没有召唤英雄");
        return false;
    }

    if(jobs.first != job() || jobs.second != hero->job())
    {
        LOG_DEBUG("合击, 非法使用与当前主角和英雄不同职业的技能");
        return false;
    }

    if(!atkRole->checkAnger())
    {
        LOG_DEBUG("合击, 怒气不够");
        return false;
    }

    atkRole->subAnger();
    setJointReadyState();
    return true;
}


/*
 * 伤害触发处理接口
 * DM:伤害值
 * conintued:是否属于持续性伤害
 */
void PK::onDamage(PK::Ptr atk, int32_t DM)
{
    if(sceneItemType() == SceneItemType::npc)
    {
        Npc::Ptr npc = std::static_pointer_cast<Npc>(shared_from_this());
        WorldBoss::me().npcDamage(npc->tplId(), atk, DM);
    }
}


void PK::setLastAttackSuccessSkill(TplId skillId)
{
    m_lastAttackSuccessSkill = skillId;
}

uint32_t PK::lastAttackSuccessSkill() const
{
    return m_lastAttackSuccessSkill;
}


/*
 * 道士施毒术技能伤害计算
 */
void PK::computePoisonSkillDamage()
{
    if(isDead() || needErase() || m_wudi)
        return;
    HPChangeType type = HPChangeType::normal;
    int64_t min_temp = 0;
    int64_t max_temp = 0;

    int64_t finalAtt = 0;
    int64_t finalDef = 0;
    int64_t finalDmgAdd = 0;
    int64_t finalCritdmg = 0;
    
    int64_t g_atkMin = ENLARGE * m_poisonAttackerAttr.atkMin;
    int64_t g_atkMax = ENLARGE * m_poisonAttackerAttr.atkMax;
    int64_t g_lucky = ENLARGE * m_poisonAttackerAttr.lucky;
    int64_t g_evil = ENLARGE * m_poisonAttackerAttr.evil;
    int64_t d_lucky = ENLARGE * getTotalLucky();
    int64_t d_evil = ENLARGE * getTotalEvil();

    //有效攻击
    max_temp = std::max<int64_t>((g_atkMax-g_atkMin) * std::min<int64_t>(ENLARGE * (g_lucky-d_evil)/PK_PARAM.atkParam(), ENLARGE) / ENLARGE, 0);
    min_temp = std::min<int64_t>((g_atkMax-g_atkMin) * std::max<int64_t>(ENLARGE * (g_lucky-d_evil)/PK_PARAM.atkParam(), -ENLARGE) / ENLARGE, 0);
    Random<int64_t> r_att((int64_t)g_atkMin + max_temp, (int64_t)g_atkMax + min_temp);
    finalAtt = r_att.get();
    LOG_DEBUG("技能, 施毒术, def:{},{}, 有效攻击:{}, max_temp:{}, min_temp:{}", name(), id(), finalAtt, max_temp, min_temp);


    int64_t d_defMin, d_defMax;
    int64_t def_param = PK_PARAM.mdefParam();
    if(Job::warrior == m_poisonAttackerAttr.attackJob)//战士
    {
        d_defMin = ENLARGE * getTotalPDefMin();
        d_defMax = ENLARGE * getTotalPDefMax();
        def_param = PK_PARAM.pdefParam();
    }
    else
    {
        d_defMin = ENLARGE * getTotalMDefMin();
        d_defMax = ENLARGE * getTotalMDefMax();
    }

    //有效防御
    max_temp = std::max<int64_t>((d_defMax-d_defMin) * std::min<int64_t>(ENLARGE * (d_lucky-g_evil)/def_param, ENLARGE) / ENLARGE, 0);
    min_temp = std::min<int64_t>((d_defMax-d_defMin) * std::max<int64_t>(ENLARGE * (d_lucky-g_evil)/def_param, -ENLARGE) / ENLARGE, 0);
    Random<int64_t> r_def((int64_t)d_defMin + max_temp, (int64_t)d_defMax + min_temp);
    finalDef = r_def.get();
    LOG_DEBUG("技能, 施毒术, def:{},{}, 有效防御:{}, defMin:{}, defMax:{}, max_temp:{}, min_temp:{}", name(), id(), d_defMin, d_defMax, finalDef, max_temp, min_temp);


    int64_t g_crit = ENLARGE * m_poisonAttackerAttr.crit;
    int64_t g_critRatio = m_poisonAttackerAttr.critRatio;
    int64_t g_critDmg = m_poisonAttackerAttr.critDamage;
    int64_t d_antiCrit = ENLARGE * getTotalAntiCrit();
    //有效暴击率
    min_temp = std::min<int64_t>(g_critRatio + (g_crit * PK_PARAM.critParam1() - d_antiCrit * PK_PARAM.critParam2()) * PK_PARAM.critParam3() / (std::abs(g_crit * PK_PARAM.critParam1() - d_antiCrit * PK_PARAM.critParam2()) + ENLARGE * ENLARGE * std::max(m_poisonAttackerAttr.critk, critk())), PK_PARAM.critParam4());
    uint64_t finalCrit = std::max<int64_t>(min_temp, PK_PARAM.critParam5());
    Random<uint64_t> r(1,1000);
    uint64_t rand = r.get();
    if(rand <= finalCrit)
        finalCritdmg = PK_PARAM.critParam6() + g_critDmg;
    LOG_TRACE("技能, 施毒术, def:{},{}, finalCritdmg:{}", name(), id(), finalCritdmg);


    finalDmgAdd = m_poisonAttackerAttr.damageAdd + m_poisonAttackerAttr.skillDamageAddPer;
    uint64_t g_dmgAddLv = ENLARGE * m_poisonAttackerAttr.damageAddLv;
    uint64_t d_dmgReduce = getTotalDmgReduce();
    uint64_t d_dmgReduceLv = ENLARGE * getTotalDmgReduceLv();
    //有效伤害加成
    max_temp = std::max<int64_t>(std::abs(g_dmgAddLv-d_dmgReduceLv), 1000);
    max_temp = (int64_t)(finalDmgAdd - d_dmgReduce + (std::pow(std::abs(g_dmgAddLv-d_dmgReduceLv), PK_PARAM.dmgaddParam1()/ENLARGE)/PK_PARAM.dmgaddParam2() - PK_PARAM.dmgaddParam3()) * (g_dmgAddLv-d_dmgReduceLv) / max_temp);
    finalDmgAdd = std::max<int64_t>(max_temp, PK_PARAM.dmgaddParam4());
    LOG_TRACE("技能, 施毒术, def:{},{}, finalDmgAdd:{}", name(), id(), finalDmgAdd);


    int32_t DM = std::max<int64_t>((finalAtt * PK_PARAM.finalWitchParam() * m_poisonAttackerAttr.skillPower/(ENLARGE*ENLARGE) + m_poisonAttackerAttr.skillDamage * ENLARGE - finalDef) * finalDmgAdd * (ENLARGE+finalCritdmg)/(ENLARGE*ENLARGE), std::max<int64_t>(g_atkMin * PK_PARAM.finalParam()/ENLARGE, ENLARGE))/ENLARGE + m_poisonAttackerAttr.skillConstDamage;

    LOG_TRACE("技能, 施毒术, def:{},{}, DM伤害{}", name(), id(), DM);

    int32_t hp = getHp();
    if(DM > hp)
        DM = hp;
    if(finalCritdmg > 0)
        type = HPChangeType::crit;

    PK::Ptr atk = PK::getPkptr(m_poisonAttackerAttr.attackId, static_cast<SceneItemType>(m_poisonAttackerAttr.sceneItem));
    changeHpAndNotify(atk, -DM, type);
}



void PK::cachePoisonAttrDB(RoleId roleId) const
{
    if(!m_pkstate.issetStatus(visual_status::poisoned)
       || 0 == roleId)
        return;
    PrivateRaw::CachePoisonAttr send;
    send.roleId = roleId;
    send.id = sceneItemType() == SceneItemType::pet ? getOwnerId() : id();
    send.sceneItem = static_cast<uint8_t>(sceneItemType());
    send.job = job();
    send.ownerJob = getOwnerJob();
    send.data = m_poisonAttackerAttr;

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(CachePoisonAttr), &send, sizeof(send));
}



/*
 * 血量改变唯一入口
 * changeVal: hp变化值  正为加血  负减血
 * continued: pk流程专用, 其它调用一律为默认值1
 */
void PK::changeHpAndNotify(PK::Ptr atk, int32_t changeVal, HPChangeType type, uint8_t continued/*=1*/)
{
	Scene::Ptr s = scene(); 
	if(nullptr == s)
		return;

    int32_t oldHp = getHp();
    if(!changeHp(changeVal) && type != HPChangeType::escape)
        return;

    int32_t newHp = getHp();
    PublicRaw::HpChangedToNine send;
    send.id = id();
    send.sceneItem = static_cast<uint8_t>(sceneItemType());
    send.type = static_cast<uint16_t>(type);
    send.hp = newHp;
    if(nullptr != atk)
	{
        send.attackId = atk->id();
		send.attackSceneItemType = static_cast<uint8_t>(atk->sceneItemType());
	}
    send.continued = continued;
	s->sendCmdTo9(RAWMSG_CODE_PUBLIC(HpChangedToNine), &send, sizeof(send), pos());

    if(newHp - oldHp < 0)
    {
        //处理伤害触发
        onDamage(atk, newHp - oldHp);

        //处理二次伤害
        if(0 == continued)
            atk->updateHpChangeList(shared_from_this(), newHp - oldHp, type);
    }

    if(0 == getHp())
        toDie(atk);
}


/*
 * mp改变唯一入口
 * changeVal: mp变化值  正加蓝   负减蓝
 * noticeNine: 是否通知九屏(true为pk流程专用, 其它情况调用该接口一律为默认值false)
 */
void PK::changeMpAndNotify(int32_t changeVal, bool noticeNine/*=false*/)
{
	Scene::Ptr s = scene(); 
	if(nullptr == s)
		return;
    if(!changeMp(changeVal))
        return;

    if(noticeNine)
    {
        PublicRaw::MpChangedToNine send;
        send.id = id();
        send.sceneItem = static_cast<uint8_t>(sceneItemType());
        send.mp = getMp();
        send.attackId = 0;
        send.attackSceneItemType = static_cast<uint8_t>(SceneItemType::none);
        s->sendCmdTo9(RAWMSG_CODE_PUBLIC(MpChangedToNine), &send, sizeof(send), pos());
        return;
    }

    PublicRaw::UpdateSelfMp send;
    send.sceneItem = static_cast<uint8_t>(sceneItemType());
    send.mp = getMp();
    sendToMe(RAWMSG_CODE_PUBLIC(UpdateSelfMp), &send, sizeof(send));
}


/*
 * 攻击动作播放9屏
 */
void PK::retAttackCmdTo9(componet::Direction dir, uint32_t skillId, uint32_t skillLevel, Coord2D pos)
{
	Scene::Ptr s = scene(); 
	if(nullptr == s)
		return;

    setDir(dir);
    setSkillPublicTime();

    PublicRaw::RetAttackSucess send;
    send.skillId = skillId;
    send.level = skillLevel;
    send.doubleEffect = isDoubleEffect(skillId) ? 1 : 0;
    send.attId = id();
    send.sceneItem = static_cast<uint8_t>(sceneItemType());
    send.dir = static_cast<uint8_t>(dir);
    send.posX = pos.x;
    send.posY = pos.y;

    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(RetAttackSucess), &send, sizeof(send), pos);
}


/*
 * 攻击失败返回
 */
void PK::retAttackFail(uint32_t skillId, AttackRetcode ret)
{
    if(sceneItemType() != SceneItemType::role)
        return;

    Role::Ptr role = std::static_pointer_cast<Role>(shared_from_this());
    std::string text;
    switch(ret)
    {
    case AttackRetcode::mp:
        text = "法力值不足";
        break;
    case AttackRetcode::cdtime:
        text = "技能CD中";
        break;
    case AttackRetcode::pubCDTime:
        text = "技能公共CD中";
        break;
    case AttackRetcode::range:
        text = "超出技能射程";
        break;
    case AttackRetcode::invalidTarget:
        text = "无效目标";
        break;
    case AttackRetcode::unknown:
        text = "未知错误";
        break;
    default:
        break;
    }
    role->sendSysChat(text);

    PublicRaw::RetAttackFail send;
    send.skillId = skillId;
    send.retcode = static_cast<uint8_t>(ret);
    sendToMe(RAWMSG_CODE_PUBLIC(RetAttackFail), &send, sizeof(send));
}



/*
 * 冲锋
 * 规则:1,方向上第一格上如果没有任何对象,则直接冲锋,直到方向上遇到格子上有对象为止
 *      2,方向上第一格上有对象,如果能推动,继续向前,否则停止
 * cell:最大可冲锋的格数
 */
void PK::chongfeng(TplId skillId, uint32_t lv, Direction dir, uint32_t cell)
{
    if(0 == cell)
        return;

    Scene::Ptr s = scene();
    if(nullptr == s)
        return;

    Coord2D oldPos = pos();
    Coord2D newPos = oldPos;
    Grid* nextGrid = s->getGridByGridCoord(newPos.neighbor(dir));
    if(nullptr == nextGrid)
        return;

    //下一格不可进入直接返回
    if(!nextGrid->enterable(sceneItemType()))
    {
        chongfengMsgTo9(skillId, lv, shared_from_this(), dir, MoveType::chongfeng, newPos);
        return;
    }

    if(nextGrid->empty())
    {//第一格为空的情况只要接着判断后面的格子上对象为空就okay
        uint32_t i = 0;
        while(i < cell)
        {
            newPos = newPos.neighbor(dir);
            ++i;
            nextGrid = s->getGridByGridCoord(newPos.neighbor(dir));
            if(nullptr == nextGrid || !nextGrid->empty() || !nextGrid->enterable(sceneItemType()))
                break;
        }

        chongfengMsgTo9(skillId, lv, shared_from_this(), dir, MoveType::chongfeng, newPos);
        return;
    }


    //紧邻目标的情况
    Grid* nextNextGrid = nullptr;
    bool npcExist = false;
    std::vector<Grid> grids;
    uint32_t i = 0;
    auto role = getRole(shared_from_this());
    while(i < cell)
    {
        bool canPush = true;
        auto canPushExec = [&](PK::Ptr target)
        {
            if(nullptr == target)
                return;
            if(nullptr != role && attack_mode::peace == role->m_attackMode.mode() && target->sceneItemType() != SceneItemType::npc)
            {
                canPush = false;
                return;
            }
            else if(level() <= target->level())
            {
                canPush = false;
                return;
            }
        };
        nextGrid->execGrid(canPushExec);
        if(!canPush)
            break;
        if(!npcExist && nextGrid->npcSize())
            npcExist = true;

        nextNextGrid = s->getGridByGridCoord(newPos.neighbor(dir).neighbor(dir));
        if(nullptr == nextNextGrid)
            break;
        if((npcExist && !nextNextGrid->enterable(SceneItemType::npc))
           || !nextNextGrid->enterable(sceneItemType()))
            break;

        grids.push_back(*nextGrid);
        newPos = newPos.neighbor(dir);
        nextGrid = nextNextGrid;
        ++i;
    }

    chongfengMsgTo9(skillId, lv, shared_from_this(), dir, MoveType::chongfeng, newPos);
    auto callback = [&, this](PK::Ptr target)
    {
        if(nullptr != target && !target->isDead() && !target->needErase())
            target->chongfengMsgTo9(skillId, lv, shared_from_this(), reverseDirection(dir), MoveType::hitback, newPos.neighbor(dir));
    };
    for(auto& iter : grids)
    {
        iter.execGrid(callback);
    }
}


void PK::chongfengMsgTo9(TplId skillId, uint32_t level, PK::Ptr atk, Direction dir, MoveType type, Coord2D newPos)
{
    if(nullptr == atk)
        return;
    auto s = scene();
    if(nullptr == s)
        return;
    if(!changePos(newPos, dir, type))
        return;

    PublicRaw::ChongfengMsgToNine send;
    if(MoveType::hitback == type)
    {
        send.defId = id();
        send.defSceneItemType = static_cast<uint8_t>(sceneItemType());
    }

    send.type = type;
    send.skillId = skillId;
    send.level = level;
    send.attId = atk->id();
    send.attSceneItemType = static_cast<uint8_t>(atk->sceneItemType());
    send.dir = static_cast<uint8_t>(dir);
    send.posX = pos().x;
    send.posY = pos().y;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(ChongfengMsgToNine), &send, sizeof(send), pos());
}


void PK::beforeSkillActionClear()
{
    m_pkvalue.clear();
    clearHpChangeList();
    m_tempFinalAtk = 0;
    m_rangeHitBackPK.clear();
    m_doubleDamageLogic = 0;
    m_lastAttackSuccessSkill = 0;
}


void PK::afterSkillActionClear(TplId skillId)
{
    m_pkvalue.clear();
    clearHpChangeList();
    m_rangeHitBackPK.clear();
    notifyDoubleSkillEffect(skillId);
}


void PK::updateHpChangeList(PK::Ptr def, int32_t value, HPChangeType type)
{
    DamageUnit unit;
    unit.type = type;
    unit.hp = value;
    m_attackHpChangeList.push_back(std::make_pair(def,unit));
}


void PK::clearHpChangeList()
{
    m_attackHpChangeList.clear();
}



PK::Ptr PK::getPkptr(PKId id, SceneItemType sceneItem)
{
    switch(sceneItem)
    {
    case SceneItemType::role:
        return RoleManager::me().getById(id);
    case SceneItemType::npc:
        return NpcManager::me().getById(id);
    case SceneItemType::hero:
        return HeroIDs::me().getById(id);
    case SceneItemType::pet:
        return PetManager::me().getById(id);
    default:
        break;
    }
    return nullptr;
}


void PK::notifyDoubleSkillEffect(TplId skillId)
{
    if(m_doubleDamageFlag > 0)
        m_doubleDamageSkillLogic.insert({std::make_pair(skillId, m_doubleDamageLogic), m_doubleDamageFlag});
    else
        eraseDoubleDamageSkillLogic(skillId, m_doubleDamageLogic);

    //下次该技能攻击有两次特效,通知前端
    if(m_doubleDamageFlag > 1)
    {
        LOG_DEBUG("技能, 两次伤害两次特效, skillId:{}", skillId);
        PublicRaw::NotifyDoubleSkillEffect send;
        send.skillId = skillId;
        sendToMe(RAWMSG_CODE_PUBLIC(NotifyDoubleSkillEffect), &send, sizeof(send));
    }

    m_doubleDamageFlag = 0;
    m_doubleDamageLogic = 0;
}

bool PK::issetDoubleDamageSkillLogic(TplId skillId, uint32_t logic_id) const
{
    std::pair<TplId, uint32_t> tmp;
    tmp.first = skillId;
    tmp.second = logic_id;
    return m_doubleDamageSkillLogic.find(tmp) != m_doubleDamageSkillLogic.end();
}

bool PK::isDoubleEffect(TplId skillId) const
{
    for(const auto& iter : m_doubleDamageSkillLogic)
    {
        if(iter.first.first == skillId)
            return iter.second > 1;
    }
    return false;
}

void PK::eraseDoubleDamageSkillLogic(TplId skillId, uint32_t logic_id)
{
    std::pair<TplId, uint32_t> tmp;
    tmp.first = skillId;
    tmp.second = logic_id;
    m_doubleDamageSkillLogic.erase(tmp);
}

void PK::setGreyName(PK::Ptr def)
{
    if(sceneItemType() == SceneItemType::npc 
       || def->sceneItemType() == SceneItemType::npc)
        return;

    Role::Ptr atkRole = getRole(shared_from_this());
    if(nullptr == atkRole)
        return;

    atkRole->updateGreynameTime();
    if(atkRole->issetNameColor(name_color::grey)
       || atkRole->issetNameColor(name_color::red))
        return;

    Role::Ptr defRole = getRole(def);
    if(nullptr == defRole)
        return;
    if(defRole->issetNameColor(name_color::grey)
       || defRole->issetNameColor(name_color::red))
        return;

    atkRole->setNameColor(name_color::grey);
}

void PK::syncNameColorTo9(uint8_t color)
{
    auto s = scene();
    if(nullptr == s)
        return;
    PublicRaw::SyncNameColorToNine sync;
    sync.id = id();
    sync.sceneItem = static_cast<uint8_t>(sceneItemType());
    sync.namecolor = color;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(SyncNameColorToNine), &sync, sizeof(sync), pos());
}

/*
 * deadPK: 死亡对象
 */
void PK::addEvil(PK::Ptr deadPK)
{
    if(sceneItemType() == SceneItemType::npc 
       || deadPK->sceneItemType() == SceneItemType::npc)
        return;

    auto s = scene();
    if(nullptr == s || 0 == s->crime())
        return;
    Role::Ptr deadRole = getRole(deadPK);
    if(nullptr == deadRole)
        return;
    if(deadRole->issetNameColor(name_color::grey)
       || deadRole->issetNameColor(name_color::red))
        return;

    Role::Ptr role = getRole(shared_from_this());
    if(nullptr == role)
        return;

    role->m_attackMode.addEvil();
}

/*
 * 
 */
bool PK::isFriend(PK::Ptr atk)
{
    if(atk == shared_from_this())
        return true;
    //暂时定 npc没有友方
    if(sceneItemType() == SceneItemType::npc)
        return false;

    Role::Ptr defRole = getRole(shared_from_this());
    if(nullptr == defRole)
        return false;

    return false == defRole->m_attackMode.isEnemy(atk);
}

/*
 * 判断两个pk对象是否敌对关系
 */
bool PK::isEnemy(PK::Ptr atk, PK::Ptr def)
{
    if(atk->isDead() || def->isDead() || atk->needErase() || def->needErase())
        return false;
    if(atk == def)
        return false;
    auto s = atk->scene();
    if(nullptr == s)
        return false;

    if(def->sceneItemType() == SceneItemType::npc)
    {
        Npc::Ptr npc = std::static_pointer_cast<Npc>(def);
        if(npc->type() == NpcType::npc || npc->type() == NpcType::collect)
            return false;

        if(npc->type() == NpcType::belong)//有归属类npc处理
        {
            if(s->copyType() == CopyMap::shabake)
            {
                Role::Ptr atkRole = getRole(atk);
                if(nullptr != atkRole && npc->getOwnerId() == atkRole->factionId())
                    return false;
            }
        }
    }

    Role::Ptr defRole = getRole(def);
    if(nullptr != defRole)
    {
        if(atk->sceneItemType() != SceneItemType::npc 
           && s->isArea(def->pos(), AreaType::security))
            return false;
        else if(!defRole->m_attackMode.isEnemy(atk))
            return false;
    }

    return true;
}

bool PK::isFight()
{
    if(EPOCH == m_fightTime)
        return false;
    return m_fightTime + std::chrono::seconds{5} >= Clock::now();
}

void PK::updateFightTime()
{
    m_fightTime = Clock::now();
}

void PK::unHide()
{
    m_buffM.unHide();
}

void PK::setPetId(PKId petId)
{
    m_petId = petId;
}

void PK::setPetTplId(TplId petTplId)
{
    m_petTplId = petTplId;
}

void PK::setPetSkillId(uint32_t petSkillId)
{
    m_petSkillId = petSkillId;
}

void PK::setPetLevel(uint32_t level)
{
    m_petLevel = level;
}

PKId PK::petId() const
{
    return m_petId;
}

TplId PK::petTplId() const
{
    return m_petTplId;
}

uint32_t PK::petSkillId() const
{
    return m_petSkillId;
}

uint32_t PK::petLevel() const
{
    return m_petLevel;
}

Pet::Ptr PK::pet() const
{
    return PetManager::me().getById(m_petId);
}

void PK::handleInterrupt(interrupt_operate op, SceneItemType interruptModel)
{
    if(sceneItemType() == SceneItemType::npc)
        return;
    switch(op)
    {
    case interrupt_operate::move:
        {
            unHide();
        }
        break;
    case interrupt_operate::attack:
        {
            unHide();
            updateFightTime();
        }
        break;
    case interrupt_operate::beAttack:
        {
            if(interruptModel != SceneItemType::npc)
                updateFightTime();
        }
        break;
    default:
        break;
    }
}

}

