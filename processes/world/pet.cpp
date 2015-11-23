#include "pet.h"
#include "pet_manager.h"
#include "scene_manager.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/pet_scene.codedef.public.h"

#include <algorithm>

namespace world{

Pet::Pet(PKId id, PetTpl::Ptr petTpl, PetLevelTpl::Ptr petLevelTpl, PK::Ptr me)
:PK(id, petTpl->name, petTpl->job, SceneItemType::pet)
,m_owner(me)
,m_petTpl(petTpl)
,m_petLevelTpl(petLevelTpl)
,m_dead(false)
{
}

void Pet::fillScreenData(PublicRaw::PetScreenData* data) const
{
    data->ownerSceneItem = static_cast<uint8_t>(m_ownerSceneItem);
    data->id = id();
    data->tplId = m_petTpl->id;
    data->posx = pos().x;
    data->posy = pos().y;
    data->dir = static_cast<decltype(data->dir)>(dir());
    data->maxhp = getMaxHp();
    data->hp = getHp();
    data->pkStatus = m_pkstate.pkstatus();

    Role::Ptr role = getRole(m_owner.lock());
    if(nullptr != role)
    {
        role->name().copy(data->roleName, sizeof(data->roleName)-1);
        data->factionId = role->factionId();
    }
}

void Pet::initTplData()
{
    auto owner = m_owner.lock();
    if(SceneItemType::role == owner->sceneItemType())
    {
        m_ownerSceneItem = SceneItemType::role;
        Role::Ptr role = std::static_pointer_cast<Role>(owner);
        m_poisonAttackerAttr = role->m_petPoisonAttr;
        m_buffM.loadFromDB(role->m_petBuffs);
    }
    else if(SceneItemType::hero == owner->sceneItemType())
    {
        m_ownerSceneItem = SceneItemType::hero;
        Hero::Ptr hero = std::static_pointer_cast<Hero>(owner);
        m_poisonAttackerAttr = hero->m_petPoisonAttr;
        m_buffM.loadFromDB(hero->m_petBuffs);
    }

    setLevel(owner->level());
    addAttribute(m_petTpl->props);
    addAttribute(m_petLevelTpl->props);

    setHp(getMaxHp());
    setMp(getMaxMp());

    for(const auto& iter : m_petTpl->skillIDs)
        m_skillM.initSkill(iter.first, iter.second);
}

PK::Ptr Pet::getOwner() const
{
    return m_owner.lock();
}

PKId Pet::getOwnerId() const
{
    if(nullptr == m_owner.lock())
        return 0;
    return (m_owner.lock())->id();
}

SceneItemType Pet::getOwnerSceneItemType() const
{
    auto owner = m_owner.lock();
    if(nullptr == owner)
        return SceneItemType::none;

    return owner->sceneItemType();
}

Job Pet::getOwnerJob() const
{
    auto owner = m_owner.lock();
    if(nullptr == owner)
        return Job::none;

    return owner->job();
}

void Pet::separateFromOwner()
{
    auto owner = m_owner.lock();
    if(nullptr == owner)
        return;

    owner->setPetId(0);
    owner->setPetTplId(0);
    clearSelfBuff();
}

void Pet::clearSelfBuff()
{
    m_buffM.processOffline();
    auto owner = m_owner.lock();
    if(nullptr == owner)
        return;

    if(owner->sceneItemType() == SceneItemType::role)
    {
        Role::Ptr role = std::static_pointer_cast<Role>(owner);
        role->m_petBuffs.clear();
    }
    else
    {
        Hero::Ptr hero = std::static_pointer_cast<Hero>(owner);
        hero->m_petBuffs.clear();
    }
}

void Pet::syncScreenDataTo9() const
{
    auto s = scene();
    if(s == nullptr)
        return;
    PublicRaw::BroadCastPetScreenDataToNine sync;
    fillScreenData(&sync.data);
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(BroadCastPetScreenDataToNine), &sync, sizeof(sync), pos());
}

void Pet::afterEnterScene()
{
    auto s = scene();
    if(s == nullptr)
        return;

    enterVisualScreens(s->get9ScreensByGridCoord(pos()));
}

void Pet::beforeLeaveScene()
{
    Role::Ptr role = getRole(shared_from_this());
    if(nullptr != role)
        cachePoisonAttrDB(role->id());

    auto s = scene();
    if(s == nullptr)
        return;
    leaveVisualScreens(s->get9ScreensByGridCoord(pos()));
}

void Pet::enterVisualScreens(const std::vector<Screen*>& screens) const
{
    auto s = scene();
    if(s == nullptr)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::PetsAroundMe) + sizeof(PublicRaw::PetScreenData));
    auto msg = reinterpret_cast<PublicRaw::PetsAroundMe*>(buf.data());
    fillScreenData(msg->pets);
    msg->size = 1;
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(PetsAroundMe), buf.data(), buf.size());
}

void Pet::leaveVisualScreens(const std::vector<Screen*>& screens) const
{
	auto s = scene();
	if(nullptr == s)
		return;

    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::PetLeaveInfo) + sizeof(PKId));
    auto msg = reinterpret_cast<PublicRaw::PetLeaveInfo*>(buf.data());
    msg->id[0] = id();
    msg->size = 1;
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(PetLeaveInfo), buf.data(), buf.size());
}

bool Pet::changePos(Coord2D newPos, componet::Direction dir, MoveType type)
{
    auto s = scene();
    if(s == nullptr)
        return false;

    Coord2D oldPos = pos();
    if(type == MoveType::hitback && oldPos == newPos)
        return false;

    Grid* newGrid = s->getGridByGridCoord(newPos);
    if(newGrid == nullptr)
        return false;

    Grid* oldGrid = s->getGridByGridCoord(oldPos);
    if(oldGrid == nullptr)//不可能
        return false;

    Screen* newScreen = s->getScreenByGridCoord(newPos);
    if(newScreen == nullptr)
        return false;

    Screen* oldScreen = s->getScreenByGridCoord(oldPos);
    if(oldScreen == nullptr)
        return false;

    //距离过远
    bool legalMove = false;
    if(type == MoveType::walk)
    {
        legalMove = (oldPos.x == newPos.x || std::abs(oldPos.x - newPos.x) == 1) && (oldPos.y - newPos.y == 0 || std::abs(oldPos.y - newPos.y) == 1);
    }
    else if(type == MoveType::run)
    {
        legalMove = (oldPos.x == newPos.x || std::abs(oldPos.x - newPos.x) == 2) && (oldPos.y - newPos.y == 0 || std::abs(oldPos.y - newPos.y) == 2);
        if(legalMove)
        {
            Grid* middleGrid = s->getGridByGridCoord(Coord2D((oldPos.x + newPos.x) / 2, (oldPos.y + newPos.y) / 2));
            if(middleGrid == nullptr || !middleGrid->enterable(SceneItemType::pet))
                legalMove = false;
        }
    }
    else if(type == MoveType::chongfeng || type == MoveType::hitback || type == MoveType::blink)
    {
        legalMove = true;
    }

    if(!legalMove)
    {
		LOG_DEBUG("请求移动到非法位置{}, 当前位置{}", newPos, pos());
        return false;
    }

    LOG_DEBUG("宠物移动, newPos:({},{}), dir:{}, movetype:{}", newPos.x, newPos.y, dir, type);

    //从老格子到新格子
    Pet::Ptr me = std::static_pointer_cast<Pet>(shared_from_this());;
    if(!newGrid->addPet(me))
    {
        return false;
    }
    //发生了屏切换, 有离开和进入视野问题要解决
    if(newScreen != oldScreen)
    {
        if(!newScreen->addPet(me))
        {
            newGrid->erasePet(me);
            return false;
        }
        oldScreen->erasePet(me);

        //已序的新老9屏
        std::vector<Screen*> old9Screens = s->get9ScreensByGridCoord(oldPos);
        std::sort(old9Screens.begin(), old9Screens.end());
        std::vector<Screen*> new9Screens = s->get9ScreensByGridCoord(newPos);
        std::sort(new9Screens.begin(), new9Screens.end());
        //脱离视野
        std::vector<Screen*> leavedScreens;
        std::set_difference(old9Screens.begin(), old9Screens.end(),
                            new9Screens.begin(), new9Screens.end(),
                            std::back_inserter(leavedScreens));
        leaveVisualScreens(leavedScreens);
        //进入视野
        std::vector<Screen*> enteredScreens;
        std::set_difference(new9Screens.begin(), new9Screens.end(),
                            old9Screens.begin(), old9Screens.end(),
                            std::back_inserter(enteredScreens));
        enterVisualScreens(enteredScreens);
    }
    oldGrid->erasePet(me);
    setPos(newPos);

    setDir(dir);

    if(type == MoveType::chongfeng || type == MoveType::hitback)
        return true;

    //发送新坐标给9屏内的玩家
    syncNewPosTo9Screens(type);
    return true;
}

void Pet::syncNewPosTo9Screens(MoveType type) const
{
	Scene::Ptr s = scene();
	if(s == nullptr)
		return;

    PublicRaw::UpdatePetPosToClient send;
    send.id = id();
    send.posx = pos().x;
    send.posy = pos().y;
    send.dir = static_cast<decltype(send.dir)>(dir());
    send.type = type;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(UpdatePetPosToClient), &send, sizeof(send), pos());
}

void Pet::timerLoop(StdInterval interval, const water::componet::TimePoint& now)
{
    PK::timerLoop(interval, now);
}

void Pet::toDie(PK::Ptr atk)
{
    m_dead = true;
    m_skillEffectM.clear();
    m_buffM.processDeath();

    erase(true);
}

bool Pet::isDead() const
{
    return m_dead;
}

void Pet::erase(bool separate)
{
    if(needErase())
        return;

    markErase(true);
    beforeLeaveScene();
    if(separate)
        separateFromOwner();
}


//======================属性接口========================

/*
 * 生命上限
 */
uint32_t Pet::getMaxHp() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(maxhp);
    ret = m_buffM.getHpMpBuffConstProp(PropertyType::maxhp, ret);
    return ret;
}


/*
 * 魔法上限
 */
uint32_t Pet::getMaxMp() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(maxmp);
    ret = m_buffM.getHpMpBuffConstProp(PropertyType::maxmp, ret);
    return ret;
}


/*
 * 最小物攻
 */
uint32_t Pet::getTotalPAtkMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_attackMin);
    ret = m_buffM.getBuffProps(PropertyType::p_attackMin, ret);
    return ret;
}


/*
 * 最大物攻
 */
uint32_t Pet::getTotalPAtkMax() const 
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_attackMax);
    ret = m_buffM.getBuffProps(PropertyType::p_attackMax, ret);
    return ret;
}


/*
 * 最小魔攻
 */
uint32_t Pet::getTotalMAtkMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_attackMin);
    ret = m_buffM.getBuffProps(PropertyType::m_attackMin, ret);
    return ret;
}


/*
 * 最大魔攻
 */
uint32_t Pet::getTotalMAtkMax() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_attackMax);
    ret = m_buffM.getBuffProps(PropertyType::m_attackMax, ret);
    return ret;
}


/*
 * 最小道术
 */
uint32_t Pet::getTotalWitchMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(witchMin);
    ret = m_buffM.getBuffProps(PropertyType::witchMin, ret);
    return ret;
}


/*
 * 最大道术
 */
uint32_t Pet::getTotalWitchMax() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(witchMax);
    ret = m_buffM.getBuffProps(PropertyType::witchMax, ret);
    return ret;
}


/*
 * min物防
 */
uint32_t Pet::getTotalPDefMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_defenceMin);
    ret = m_buffM.getBuffProps(PropertyType::p_defenceMin, ret);
    return ret;
}


/*
 * max物防
 */
uint32_t Pet::getTotalPDefMax() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_defenceMax);
    ret = m_buffM.getBuffProps(PropertyType::p_defenceMax, ret);
    return ret;
}


/*
 * min魔防
 */
uint32_t Pet::getTotalMDefMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_defenceMin);
    ret = m_buffM.getBuffProps(PropertyType::m_defenceMin, ret);
    return ret;
}


/*
 * max魔防
 */
uint32_t Pet::getTotalMDefMax() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_defenceMax);
    ret = m_buffM.getBuffProps(PropertyType::m_defenceMax, ret);
    return ret;
}


/*
 * 幸运值
 */
uint32_t Pet::getTotalLucky() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(lucky);
    ret = m_buffM.getBuffProps(PropertyType::lucky, ret);
    return ret;
}


/*
 * 诅咒值
 */
uint32_t Pet::getTotalEvil() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(evil);
    ret = m_buffM.getBuffProps(PropertyType::evil, ret);
    return ret;
}


/*
 * 命中
 */
uint32_t Pet::getTotalShot() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(shot);
    ret = m_buffM.getBuffProps(PropertyType::shot, ret);
    return ret;
}


/*
 * 命中率
 */
uint32_t Pet::getTotalShotRatio() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(shotRatio);
    ret = m_buffM.getBuffProps(PropertyType::shotRatio, ret);
    return ret;
}


/*
 * 物闪
 */
uint32_t Pet::getTotalPEscape() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_escape);
    ret = m_buffM.getBuffProps(PropertyType::p_escape, ret);
    return ret;
}


/*
 * 魔闪
 */
uint32_t Pet::getTotalMEscape() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_escape);
    ret = m_buffM.getBuffProps(PropertyType::m_escape, ret);
    return ret;
}


/*
 * 闪避率
 */
uint32_t Pet::getTotalEscapeRatio() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(escapeRatio);
    ret = m_buffM.getBuffProps(PropertyType::escapeRatio, ret);
    return ret;
}


/*
 * 暴击
 */
uint32_t Pet::getTotalCrit() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(crit);
    ret = m_buffM.getBuffProps(PropertyType::crit, ret);
    return ret;
}


/*
 * 暴击率
 */
uint32_t Pet::getTotalCritRatio() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(critRatio);
    ret = m_buffM.getBuffProps(PropertyType::critRatio, ret);
    return ret;
}


/*
 * 坚韧
 */
uint32_t Pet::getTotalAntiCrit() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(antiCrit);
    ret = m_buffM.getBuffProps(PropertyType::antiCrit, ret);
    return ret;
}


/*
 * 暴伤
 */
uint32_t Pet::getTotalCritDmg() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(critDamage);
    ret = m_buffM.getBuffProps(PropertyType::critDamage, ret);
    return ret;
}


/*
 * 增伤
 */
uint32_t Pet::getTotalDmgAdd() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(damageAdd);
    ret = m_buffM.getBuffProps(PropertyType::damageAdd, ret);
    return ret;
}


/*
 * 减伤
 */
uint32_t Pet::getTotalDmgReduce() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(damageReduce);
    ret = m_buffM.getBuffProps(PropertyType::damageReduce, ret);
    return ret;
}

}

