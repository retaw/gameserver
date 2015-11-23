#include "fire.h"
#include "pk.h"
#include "pk_cfg.h"
#include "massive_config.h"
#include "scene_manager.h"

#include "world.h"

#include "protocol/rawmsg/public/fire_scene.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

Fire::Fire(PKId id, PK::Ptr owner)
: m_id(id)
, m_owner(owner)
{
}

void Fire::setLifeTime(uint16_t addSec)
{
    m_lifeEnd = Clock::now() + std::chrono::seconds {Massive::me().m_fireCfg.lifeTime + addSec};
}

bool Fire::checkTimeOut(const TimePoint& now) const
{
    return m_lifeEnd <= now;
}

void Fire::setOwnerAttr(const PKAttr& attr)
{
    m_ownerAttr = attr;
}

PKId Fire::id() const
{
    return m_id;
}

void Fire::setPos(Coord2D pos)
{
    m_pos = pos;
}

Coord2D Fire::pos() const
{
    return m_pos;
}

void Fire::setSceneId(SceneId sceneId)
{
    m_sceneId = sceneId;
}

Scene::Ptr Fire::scene() const
{
    return SceneManager::me().getById(m_sceneId);
}

void Fire::afterEnterScene() const
{
    auto s = scene();
    if(nullptr == s)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(32);
    buf.resize(sizeof(PublicRaw::FireAroundMe) + sizeof(PublicRaw::FireScreenData));
    auto msg = reinterpret_cast<PublicRaw::FireAroundMe*>(buf.data());
    fillScreenData(msg->data);
    msg->size = 1;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(FireAroundMe), buf.data(), buf.size(), pos());
}

void Fire::leaveScene()
{
    auto s = scene();
    if(nullptr == s)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(32);
    buf.resize(sizeof(PublicRaw::FireLeaveInfo) + sizeof(PublicRaw::FireScreenData));
    auto msg = reinterpret_cast<PublicRaw::FireLeaveInfo*>(buf.data());
    fillScreenData(msg->data);
    msg->size = 1;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(FireLeaveInfo), buf.data(), buf.size(), pos());

    //从场景删除
    s->eraseFire(shared_from_this());
}

void Fire::fillScreenData(PublicRaw::FireScreenData* data) const
{
    data->posx = pos().x;
    data->posy = pos().y;
}

void Fire::action()
{
    ++m_tick;
    if(m_tick != Massive::me().m_fireCfg.interval)
        return;

    m_tick = 0;
    auto s = scene();
    if(nullptr == s)
        return;
    Grid* selfGrid = s->getGridByGridCoord(pos());
    if(nullptr == selfGrid)
        return;

    auto exec = [&, this] (PK::Ptr def)
    {
        if(nullptr == def)
            return;
        if(def->isDead() || def->needErase())
            return;
        auto s = def->scene();
        if(nullptr == s)
            return;

        Role::Ptr defRole = getRole(def);
        if(nullptr != defRole 
           && (s->isArea(def->pos(), AreaType::security) || !defRole->m_attackMode.isEnemy(m_owner.lock())))
        {
            LOG_DEBUG("火墙, m_attackMode不是敌人关系, def:{}", def->name());
            return;
        }

        computeFireDamage(def);
    };

    selfGrid->execGrid(exec);
}

#define ENLARGE 1000

void Fire::computeFireDamage(PK::Ptr def) const
{
    if(def->m_wudi)
        return;
    HPChangeType type = HPChangeType::normal;
    int64_t min_temp = 0;
    int64_t max_temp = 0;

    int64_t g_shot = m_ownerAttr.shot;
    int64_t g_shotRatio = m_ownerAttr.shotRatio;
    int64_t d_pescape = ENLARGE * def->getTotalPEscape();
    int64_t d_escapeRatio = def->getTotalEscapeRatio();

    max_temp = (int64_t)(PK_PARAM.shotParam1() + g_shotRatio - d_escapeRatio + (g_shot * PK_PARAM.shotParam2() - d_pescape * PK_PARAM.shotParam3()) * PK_PARAM.shotParam4() / (std::abs(g_shot * PK_PARAM.shotParam2() - d_pescape * PK_PARAM.shotParam3()) + ENLARGE * ENLARGE * std::max(m_ownerAttr.shotk, def->shotk())));
    min_temp = std::min<int64_t>(max_temp, 1000);
    uint64_t finalSHot = std::max<int64_t>(min_temp, PK_PARAM.shotParam5());

    Random<uint64_t> rshot(1,1000);
    uint64_t rand = rshot.get();
    if(def->m_wudi && rand > finalSHot)
    {
        def->changeHpAndNotify(m_owner.lock(), 0, HPChangeType::escape);
        return;
    }

    int64_t finalAtt = 0;
    int64_t finalDef = 0;
    int64_t finalDmgAdd = 0;
    int64_t finalCritdmg = 0;
    
    int64_t g_atkMin = ENLARGE * m_ownerAttr.atkMin;
    int64_t g_atkMax = ENLARGE * m_ownerAttr.atkMax;
    int64_t g_lucky = ENLARGE * m_ownerAttr.lucky;
    int64_t g_evil = ENLARGE * m_ownerAttr.evil;
    int64_t d_lucky = ENLARGE * def->getTotalLucky();
    int64_t d_evil = ENLARGE * def->getTotalEvil();

    //有效攻击
    max_temp = std::max<int64_t>((g_atkMax-g_atkMin) * std::min<int64_t>(ENLARGE * (g_lucky-d_evil)/PK_PARAM.atkParam(), ENLARGE) / ENLARGE, 0);
    min_temp = std::min<int64_t>((g_atkMax-g_atkMin) * std::max<int64_t>(ENLARGE * (g_lucky-d_evil)/PK_PARAM.atkParam(), -ENLARGE) / ENLARGE, 0);
    Random<int64_t> r_att((int64_t)g_atkMin + max_temp, (int64_t)g_atkMax + min_temp);
    finalAtt = r_att.get();
    LOG_DEBUG("技能, 火墙, 有效攻击:{}, max_temp:{}, min_temp:{}", finalAtt, max_temp, min_temp);


    int64_t d_defMin, d_defMax;
    int64_t def_param = PK_PARAM.mdefParam();
    if(Job::warrior == m_ownerAttr.attackJob)//战士
    {
        d_defMin = ENLARGE * def->getTotalPDefMin();
        d_defMax = ENLARGE * def->getTotalPDefMax();
        def_param = PK_PARAM.pdefParam();
    }
    else
    {
        d_defMin = ENLARGE * def->getTotalMDefMin();
        d_defMax = ENLARGE * def->getTotalMDefMax();
    }

    //有效防御
    max_temp = std::max<int64_t>((d_defMax-d_defMin) * std::min<int64_t>(ENLARGE * (d_lucky-g_evil)/def_param, ENLARGE) / ENLARGE, 0);
    min_temp = std::min<int64_t>((d_defMax-d_defMin) * std::max<int64_t>(ENLARGE * (d_lucky-g_evil)/def_param, -ENLARGE) / ENLARGE, 0);
    Random<int64_t> r_def((int64_t)d_defMin + max_temp, (int64_t)d_defMax + min_temp);
    finalDef = r_def.get();
    LOG_DEBUG("技能, 火墙, 有效防御:{}, max_temp:{}, min_temp:{}", finalDef, max_temp, min_temp);


    int64_t g_crit = ENLARGE * m_ownerAttr.crit;
    int64_t g_critRatio = m_ownerAttr.critRatio;
    int64_t g_critDmg = m_ownerAttr.critDamage;
    int64_t d_antiCrit = ENLARGE * def->getTotalAntiCrit();
    //有效暴击率
    min_temp = std::min<int64_t>(g_critRatio + (g_crit * PK_PARAM.critParam1() - d_antiCrit * PK_PARAM.critParam2()) * PK_PARAM.critParam3() / (std::abs(g_crit * PK_PARAM.critParam1() - d_antiCrit * PK_PARAM.critParam2()) + ENLARGE * ENLARGE * std::max(m_ownerAttr.critk, def->critk())), PK_PARAM.critParam4());
    uint64_t finalCrit = std::max<int64_t>(min_temp, PK_PARAM.critParam5());
    Random<uint64_t> r(1,1000);
    rand = r.get();
    if(rand <= finalCrit)
        finalCritdmg = PK_PARAM.critParam6() + g_critDmg;


    finalDmgAdd = m_ownerAttr.damageAdd + m_ownerAttr.skillDamageAddPer;
    uint64_t g_dmgAddLv = ENLARGE * m_ownerAttr.damageAddLv;
    uint64_t d_dmgReduce = def->getTotalDmgReduce();
    uint64_t d_dmgReduceLv = ENLARGE * def->getTotalDmgReduceLv();
    //有效伤害加成
    max_temp = std::max<int64_t>(std::abs(g_dmgAddLv-d_dmgReduceLv), 1000);
    max_temp = (int64_t)(finalDmgAdd - d_dmgReduce + (std::pow(std::abs(g_dmgAddLv-d_dmgReduceLv), PK_PARAM.dmgaddParam1()/ENLARGE)/PK_PARAM.dmgaddParam2() - PK_PARAM.dmgaddParam3()) * (g_dmgAddLv-d_dmgReduceLv) / max_temp);
    finalDmgAdd = std::max<int64_t>(max_temp, PK_PARAM.dmgaddParam4());


    int32_t DM = std::max<int64_t>((finalAtt * PK_PARAM.finalWitchParam() * m_ownerAttr.skillPower/(ENLARGE*ENLARGE) + m_ownerAttr.skillDamage * ENLARGE - finalDef) * finalDmgAdd * (ENLARGE+finalCritdmg)/(ENLARGE*ENLARGE), std::max<int64_t>(g_atkMin * PK_PARAM.finalParam()/ENLARGE, ENLARGE))/ENLARGE + m_ownerAttr.skillConstDamage;

    LOG_TRACE("技能, 火墙, DM伤害{}", DM);

    int32_t hp = def->getHp();
    if(DM > hp)
        DM = hp;
    if(finalCritdmg > 0)
        type = HPChangeType::crit;

    def->changeHpAndNotify(m_owner.lock(), -DM, type);
}



//================================================
FireManager::FireManager()
{
}

FireManager& FireManager::me()
{
    static FireManager me;
    return me;
}

/*
 * center:火墙中心点   radius:半径(0,表示单个坐标)   addSec:火墙增加持续时间(秒)
 */
void FireManager::summonFires(PK::Ptr owner, Coord2D center, uint16_t radius, uint16_t addSec, const PKAttr& attr)
{
    if(nullptr == owner)
        return;

    auto s = owner->scene();
    if(nullptr == s)
        return;

    auto summonExec = [&, this] (Coord2D pos) -> bool
    {
        Grid* grid = s->getGridByGridCoord(pos);
        if(nullptr == grid)
            return false;
        if(!grid->enterable(SceneItemType::fire))
            return false;

        Fire::Ptr fire = Fire::create(++m_lastFireId, owner);
        if(nullptr == fire)
        {
            --m_lastFireId;
            LOG_DEBUG("火墙, 召唤失败, owner:{}", owner->name());
            return false;
        }
        if(!s->addFire(fire, pos))
        {
            --m_lastFireId;
            LOG_DEBUG("火墙, 添加到场景失败");
            return false;  
        }
        m_fires[owner->id()].push_back(fire);

        fire->setLifeTime(addSec);
        fire->setOwnerAttr(attr);
        fire->afterEnterScene();

        return false;
    };

    s->tryExecAreaByCircle(center, radius, summonExec);
}

void FireManager::erase(PKId ownerId)
{
    auto it = m_fires.find(ownerId);
    if(it == m_fires.end())
        return;
    auto& fires = it->second;
    for(const auto& fire : fires)
        fire->leaveScene();

    fires.clear();
}

void FireManager::regTimer()
{
    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&FireManager::timerLoop, this, std::placeholders::_1));
}

void FireManager::timerLoop(const TimePoint& now)
{
    auto it = m_fires.begin();
    for( ; it != m_fires.end(); ++it)
    {
        auto& fires = it->second;
        for(auto subIt = fires.begin(); subIt != fires.end(); )
        {
            Fire::Ptr fire = *subIt;
            if(nullptr == fire)
            {
                subIt = fires.erase(subIt);
                continue;
            }

            if(fire->checkTimeOut(now))
            {
                fire->leaveScene();
                subIt = fires.erase(subIt);
            }
            else
            {
                fire->action();
                ++subIt;
            }
        }
    }
}

}
