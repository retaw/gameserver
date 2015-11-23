#include "hero.h"
#include "role.h"
#include "pk_cfg.h"
#include "scene.h"
#include "world.h"
#include "exp_config.h"
#include "relations_manager.h"
#include "massive_config.h"
#include "scene_object_manager.h"
#include "fire.h"
#include "stone_package.h"
#include "hero_config.h"

#include "water/componet/logger.h"
#include "water/componet/serialize.h"

#include "protocol/rawmsg/public/hero_scene.h"
#include "protocol/rawmsg/public/hero_scene.codedef.public.h"
#include "protocol/rawmsg/private/friend.h"
#include "protocol/rawmsg/private/friend.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include <algorithm>

namespace world{

Hero::Hero(HeroId heroId, Role::Ptr role, const HeroInfoPra& info, const PrivateRaw::RetHeroSerializeData* rev)
: PK(heroId, "英雄", info.job, SceneItemType::hero)
  , m_owner(role)
  , m_sex(info.sex)
  , m_levelProps(info.job, SceneItemType::hero)
  , m_packageSet(SceneItemType::hero, info.job, role->id())
  , m_exp(info.exp)
  , m_dead(false)
  , m_wash(SceneItemType::hero, role->id(), *this)
  , m_wing(SceneItemType::hero, role->id(), *this)
  , m_zhuansheng(SceneItemType::hero, role->id(), *this)
{
    init(info, rev);
    m_ai.moveData.stepWalkInterval = std::chrono::milliseconds(480);
    m_ai.moveData.stepRunInterval = std::chrono::milliseconds(560);
}

void Hero::init(const HeroInfoPra& info, const PrivateRaw::RetHeroSerializeData* rev)
{
    setLevel(info.level);
    setTurnLifeLevel(info.turnLife);
    initAttrMember();

    //反序列化得到的所有变长数据存入各自的vector中
    std::string ss;
    ss.append((const char*)rev->buf,(std::string::size_type)rev->size);
    Deserialize<std::string> ds(&ss);
    std::vector<RoleObjData::ObjData> objVec;
    std::vector<SkillData> skillDataVec;
    std::vector<PKCdStatus> pkCDStatusVec;
    std::vector<BuffData> buffVec;
    std::vector<WashPropInfo> washPropVec;
    ds >> objVec;
    ds >> skillDataVec;
    ds >> buffVec;
    ds >> pkCDStatusVec;
    ds >> washPropVec;
    ds >> m_poisonAttackerAttr;

    uint32_t petTplId;
    ds >> petTplId;
    ds >> m_petPoisonAttr;
    ds >> m_petBuffs;

    //反序列化完,初始化各模块数据
    m_packageSet.loadFromDB(objVec);
    m_skillM.loadFromDB(skillDataVec);
    m_buffM.loadFromDB(buffVec);
    initPassiveSkillCD(pkCDStatusVec);
    m_wash.loadFromDB(washPropVec);
    setPetTplId(petTplId);

    //放最后
    if(0 == info.hp)
    {
        setHp(getMaxHp());
        setMp(getMaxMp());
    }
    else
    {
        setHp(info.hp);
        setMp(info.mp);
    }
}


void Hero::initAttrMember()
{
    m_attrMembers.clear();
    m_attrMembers.push_back(&m_levelProps);
    m_attrMembers.push_back(&m_wash);
}

SceneItemType Hero::getOwnerSceneItemType() const
{
    Role::Ptr role = getOwner();
    if(nullptr == role)
        return SceneItemType::none;

    return SceneItemType::role;
}

Role::Ptr Hero::getOwner() const
{
    Role::Ptr role = m_owner.lock(); 
    if(role == nullptr)
        return nullptr;

    return role;
}

RoleId Hero::getOwnerId() const
{
    Role::Ptr role = m_owner.lock(); 
    if(role == nullptr)
        return 0;

    return role->id();
}

Job Hero::getOwnerJob() const
{
    Role::Ptr role = m_owner.lock(); 
    if(role == nullptr)
        return Job::none;

    return role->job();
}

const Sex& Hero::sex() const
{
    return m_sex;
}

bool Hero::sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
    Role::Ptr role = getOwner();
    if(role == nullptr)
        return false;

    return role->sendToMe(msgCode, msg, msgSize);
}

void Hero::afterEnterScene()
{
    Scene::Ptr s = scene();
    if(nullptr == s)
        return;

    {
        m_skillM.initSkill(10100);
    }

    //sendSkillListToMe();
    sendMainToMe();
    enterVisualScreens(s->get9ScreensByGridCoord(pos()));
    sendSummonHeroTo9();
}

void Hero::beforeLeaveScene()
{
    cachePoisonAttrDB(getOwnerId());
    Scene::Ptr s = scene();
    if(nullptr == s)
        return;

    leaveVisualScreens(s->get9ScreensByGridCoord(pos()));
}

void Hero::enterVisualScreens(const std::vector<Screen*>& screens) const
{
    Scene::Ptr s = scene();
    if(nullptr == s)
        return;

    //自己进入别人的视野
    PublicRaw::HerosAroundMe send;
    fillScreenData(&(send.heros[0]));
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(HerosAroundMe), &send, sizeof(send));
}
void Hero::leaveVisualScreens(const std::vector<Screen*>& screens) const
{
    Scene::Ptr s = scene();
    if(nullptr == s)
        return;

    //将自己从别人的视野中删除
    PublicRaw::HeroLeaveInfo send;  
    send.heroId[0] = id(); 
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(HeroLeaveInfo), &send, sizeof(send));
}

void Hero::fillScreenData(HeroScreenData* data) const
{
    fillBasicData(data);  
    data->posX = pos().x;
    data->posY = pos().y;
    data->level = level();
    data->maxhp = getMaxHp();
    data->hp = getHp();
    data->isDead = isDead();
    data->dir = static_cast<decltype(data->dir)>(dir());

    data->weapon = getTplIdByObjChildType(ObjChildType::weapon);
    data->clothes = getTplIdByObjChildType(ObjChildType::clothes);
    data->wing = getTplIdByObjChildType(ObjChildType::wing);

    Role::Ptr role = getOwner();
    if(nullptr != role)
    {
        role->name().copy(data->roleName, sizeof(data->roleName)-1);
        data->nameColor = role->nameColor();
        data->factionId = role->factionId();
    }
    data->turnLifeLevel = turnLifeLevel();
}

void Hero::fillBasicData(HeroBasicData* data) const
{
    Role::Ptr role = getOwner();
    if(role == nullptr)
        return;

    data->roleId = role->id();
    data->heroId  = id();
    data->job = job();  
    data->sex = sex(); 
}

void Hero::sendMainToMe()
{
    Role::Ptr role = getOwner();
    if(role == nullptr)
        return;

    resetHpMp();
    PublicRaw::HeroSelfMainInfo send;
    fillMainData(&send.info); 
    role->sendToMe(RAWMSG_CODE_PUBLIC(HeroSelfMainInfo), &send, sizeof(send));  
}

void Hero::syncScreenDataTo9() const
{
    Scene::Ptr s = scene();
    if(nullptr == s)
        return;

    PublicRaw::BroadCastHeroScreenDataToNine send;
    fillScreenData(&send.info);
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(BroadCastHeroScreenDataToNine), 
                  &send, sizeof(send), pos());
}

void Hero::fillMainData(HeroMainData* data) const
{
    fillBasicData(data);
    data->maxhp = getMaxHp();
    data->hp = getHp();
    data->maxmp = getMaxMp();
    data->mp = getMp();

    data->maxPAtk = getTotalPAtkMax();
    data->minPAtk = getTotalPAtkMin();
    data->maxMAtk = getTotalMAtkMax();
    data->minMAtk = getTotalMAtkMin();
    data->maxWitch = getTotalWitchMax();
    data->minWitch = getTotalWitchMin();
    data->maxPDef = getTotalPDefMax();
    data->minPDef = getTotalPDefMin();
    data->maxMDef = getTotalMDefMax();
    data->minMDef = getTotalMDefMin();
    data->shot = getTotalShot();
    data->pEscape = getTotalPEscape();
    data->mEscape = getTotalMEscape();
    data->crit = getTotalCrit();
    data->critdmg = getTotalCritDmg();
    data->antiCrit = getTotalAntiCrit();
    data->lucky = getTotalLucky();
    //data->pk
    data->dmgAddLevel = getTotalDmgAddLv();
    data->dmgReduceLevel = getTotalDmgReduceLv();
    data->hpLevel = getHpLv();
    data->mpLevel = getMpLv();

    data->weapon = getTplIdByObjChildType(ObjChildType::weapon);
    data->clothes = getTplIdByObjChildType(ObjChildType::clothes);
    data->wing = getTplIdByObjChildType(ObjChildType::wing);

    data->level = level();
    data->gotExp = getCurLevelGotExp();
    data->needExp = getLevelUpNeedExp();
    data->expRate = 0;
    if(data->needExp > 0)
        data->expRate = 1000 * data->gotExp / data->needExp;
    data->turnLifeLevel = turnLifeLevel();
}

void Hero::sendSummonHeroTo9()
{
    Role::Ptr role = getOwner();
    if(role == nullptr)
        return;

    Scene::Ptr s = scene();
    if(nullptr == s)
        return;

    PublicRaw::RetSummonHeroToNine send;
    send.roleId = role->id();
    send.heroId = id();
    send.job = job();
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(RetSummonHeroToNine), &send, sizeof(send), pos());
}

void Hero::timerLoop(StdInterval interval, const water::componet::TimePoint& now)
{
    m_lastTimerTime = now;
    PK::timerLoop(interval, now);

    switch(interval)
    {
    case StdInterval::msec_100:
        aiLoop(now);
        followThePath(now);
        break;
    case StdInterval::msec_500:
        break;
	case StdInterval::sec_3:
		{
			autoAddHpAndMp();
		}
		break;
    default:
        break;
    }
}

bool Hero::changePos(Coord2D newPos, componet::Direction dir, MoveType type)
{
    Scene::Ptr s = scene();
    if(s == nullptr)
        return false;

    Coord2D oldPos = pos();
    if(type == MoveType::hitback && oldPos == newPos)
        return false;

    Grid* newGrid = s->getGridByGridCoord(newPos);
    if(newGrid == nullptr)
        return false;

    Grid* oldGrid = s->getGridByGridCoord(oldPos);
    if(oldGrid == nullptr)
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
            if(middleGrid == nullptr || !middleGrid->enterable(SceneItemType::hero))
                legalMove = false;
        }
    }
    else if(type == MoveType::chongfeng || type == MoveType::hitback || type == MoveType::blink)
    {
        legalMove = true;
    }

    if(!legalMove)
    {
        return false;
    }

    //判断是否走完了上一格, 防加速外挂
    Hero::Ptr me = std::static_pointer_cast<Hero>(shared_from_this());
    if(!newGrid->addHero(me))
    {
        LOG_DEBUG("hero:({}), 目标格子不可进入, changePos {}->{}", this, oldPos, newPos);
        return false;
    }

    //发生了屏切换, 有离开和进入视野问题要解决
    if(newScreen != oldScreen)
    {
        if(!newScreen->addHero(me))
        {
            LOG_DEBUG("hero:{}, 目标屏不可进入, changePos {}->{}", this, oldPos, newPos);
            newGrid->eraseHero(me);
            return false;
        }
        oldScreen->eraseHero(me);

        //已序的新老九屏
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
    oldGrid->eraseHero(me);
    setPos(newPos);

    setDir(dir);
    handleInterrupt(interrupt_operate::move, SceneItemType::hero);

    LOG_DEBUG("hero changPos, from={}, to={}, type={}, dir={}", oldPos, newPos, type, dir);

    if(MoveType::chongfeng == type || MoveType::hitback == type)
        return true;

    //发送新坐标给九屏内的玩家
    syncNewPosTo9Screens(type);

    return true;
}


void Hero::syncNewPosTo9Screens(MoveType type) const
{
    Scene::Ptr s = scene();
    if(s == nullptr)
        return;

    PublicRaw::UpdateHeroPosToClient send;
    send.heroId = id();
    send.posX = pos().x;
    send.posY = pos().y;
    send.dir = static_cast<decltype(send.dir)>(dir());
    send.type = type;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(UpdateHeroPosToClient), &send, sizeof(send), pos());

    return;
}

TplId Hero::getTplIdByObjChildType(ObjChildType childType) const
{
    PackageType packageType = getEquipPackageType();
    if(packageType == PackageType::none)
        return 0;

    return m_packageSet.getTplIdByObjChildType(childType, packageType);
}

PackageType Hero::getEquipPackageType() const
{
    if(job() == Job::warrior)
        return PackageType::equipOfWarrior;
    else if(job() == Job::magician)
        return PackageType::equipOfMagician;
    else if(job() == Job::taoist)
        return PackageType::equipOfTaoist;

    return PackageType::none; 
}

PackageType Hero::getStonePackageType() const
{
    if(job() == Job::warrior)
        return PackageType::stoneOfWarrior;
    else if(job() == Job::magician)
        return PackageType::stoneOfMagician;
    else if(job() == Job::taoist)
        return PackageType::stoneOfTaoist;

    return PackageType::none;
}

bool Hero::isDead() const
{
    return m_dead;
}

void Hero::toDie(PK::Ptr attacker)
{
    m_dead = true;
    m_skillEffectM.clear();
    m_buffM.processDeath();
    Role::Ptr role = getOwner();
    if(role == nullptr)
        return;
    addEnemy(attacker);	
    attacker->addEvil(shared_from_this());
    role->m_heroManager.requestRecallHero();
}

void Hero::reset()
{
    Role::Ptr role = m_owner.lock(); 
    if(role == nullptr)
        return;
	
	if(role->getSummonHeroFlag())
		return;

	m_dead = false;
	if(getHp() != getMaxHp())
	{
		setHp(getMaxHp());
	}
	if(getMp() != getMaxMp())
	{
		setMp(getMaxMp());
	}	

	return;
}

void Hero::addEnemy(PK::Ptr atk)
{
    if(atk->sceneItemType() != SceneItemType::role && atk->sceneItemType() != SceneItemType::hero)
        return;
    auto s = scene();
    if(nullptr != s && 0 == s->crime())
        return;
    PrivateRaw::InsertEnemy send;
    send.roleId = getOwner()->id();

    if(atk->sceneItemType() == SceneItemType::role)
    {
        send.enemyId = atk->id();
    }
    if(atk->sceneItemType() == SceneItemType::hero)
    {
        auto hero = std::static_pointer_cast<Hero>(atk);
        send.enemyId = hero->getOwnerId();
    }
    send.beKilledType = BeKilledType::hero;
    ProcessIdentity dbcachedId("func", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(InsertEnemy), &send, sizeof(send));
    LOG_DEBUG("添加仇人, 向func发送仇人信息, roleId = {}, enemyId = {}",
              send.roleId, send.enemyId);
}

void Hero::handleInterrupt(interrupt_operate op, SceneItemType interruptModel)
{
    PK::handleInterrupt(op, interruptModel);
}

void Hero::dieDropEquip()
{
    auto s = scene();
    if(nullptr == s || 0 == s->dropEquip())
        return;
    uint32_t antiDropEquip = getTotalAntiDropEquip();
    if(antiDropEquip >= 10000)
        return;
    Role::Ptr role = getOwner();
    if(nullptr == role)
        return;

    std::vector<RoleId> owners;
    owners.push_back(role->id());
    std::vector<SceneObject::Ptr> dropObjList;
    auto heroEquipPackageDropExec = [&, this] (CellInfo& cellInfo) -> bool
    {
        if(nullptr == cellInfo.objPtr
           || Bind::yes == cellInfo.bind)
            return true;

        //罪恶值系数
        uint64_t evilParam = 1000 * (1 + role->m_attackMode.evilVal() * Massive::me().m_dropCfg.evilParam1 / 1000 / (role->m_attackMode.evilVal() * Massive::me().m_dropCfg.evilParam2 / 1000 + Massive::me().m_dropCfg.constant));
        //掉率
        uint64_t dropRate = Massive::me().m_dropCfg.equipPackage * cellInfo.objPtr->prob() * evilParam * (1-antiDropEquip/10000) / 100 / 1000;
        Random<uint64_t> r(1,10000);
        uint64_t rand = r.get();
        if(rand > dropRate)
        {
            LOG_DEBUG("死亡掉落, 概率性失败, dropRate:{} rand={}", dropRate, rand);
            return true;
        }

        //掉落
        SceneObject::Ptr sceneObj = SceneObjectManager::me().createObj(cellInfo.objPtr->tplId(), cellInfo.item, Bind::no, owners, Clock::now() + std::chrono::seconds {Massive::me().m_dropCfg.ownerTime});
        if(nullptr == sceneObj)
            return true;

        LOG_TRACE("死亡掉落, hero=[rolename:{}, {}, {}], obj({}, {}, {}), packageType={}, cell={}", role->name(), id(), job(), cellInfo.objPtr->name(), cellInfo.objPtr->tplId(), cellInfo.item, getEquipPackageType(), cellInfo.cell);
        if(nullptr != m_packageSet.eraseObjByCell(cellInfo.cell, getEquipPackageType(), "英雄死亡掉落"))
            dropObjList.push_back(sceneObj);
        return true;
    };
    m_packageSet.execPackageCell(getEquipPackageType(), heroEquipPackageDropExec);

    s->addSceneObj(pos(), dropObjList);
}

void Hero::setLevel(uint32_t level) 
{
    PK::setLevel(level);
    m_levelProps.setLevel(level);

    return;
}

void Hero::setTurnLifeLevel(TurnLife level)
{
    TurnLife oldLevel = turnLifeLevel();
    PK::setTurnLifeLevel(level);

    m_zhuansheng.calcAttribute();
    updateTurnLifeLevelToDB();		
    LOG_TRACE("转生, 英雄, 转生成功! ownerId={}, job={}, level={}->level={}", 
              getOwnerId(), job(), oldLevel, level);
    return;
}

bool Hero::updateTurnLifeLevelToDB()
{
    PrivateRaw::UpdateHeroTurnLifeLevel send;
    send.roleId = id();
    send.job = job();
    send.turnLifeLevel = turnLifeLevel();

    ProcessIdentity dbcachedId("dbcached", 1);
    const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateHeroTurnLifeLevel), &send, sizeof(send));

    return ret;
}

void Hero::initSkill(std::vector<SkillData>& data)
{
    m_skillM.loadFromDB(data);
}

void Hero::initPassiveSkillCD(std::vector<PKCdStatus>& data)
{
    for(const auto& iter : data)
        m_skillM.insertPassiveSkillCD(iter);
}

void Hero::sendSkillListToMe()
{
    return m_skillM.sendSkillListToMe();
}

void Hero::addExp(uint64_t exp)
{
    Role::Ptr role = getOwner();
    if(role == nullptr || 0 == exp)
        return;

    uint64_t oldExp = getExp();
    m_exp += exp;

    //组队获得加成
    uint64_t teamAddExp = exp * RelationsManager::me().getTeamMeberExtraAddExpRatio(getOwner());
    m_exp += teamAddExp;
    m_exp += exp * m_buffM.heroExpAddPercent();
    LOG_TRACE("英雄, 获得经验, roleId={}, hero={}, exp={}->{}, addExp={}, teamAddExp={}", 
              role->id(), job(), oldExp, m_exp, exp, teamAddExp);

    if(!updateLevelAndExpToDB())
        return;

    judgeLevelUp();
    sendHeroCurLevelGotExp();
    role->sendSysChat(ChannelType::screen_right_down, "获得 英雄经验: {}", exp);
    return;
}

uint64_t Hero::getExp() const
{
    return m_exp;
}

void Hero::judgeLevelUp()
{
    uint32_t levelUpNum = getHeroCanLevelUpNum();
    if(0 == levelUpNum)
        return;

    for(uint32_t i = 0; i < levelUpNum; i++)
    {
        levelUp();
    }

    return;
}

uint32_t Hero::getHeroCanLevelUpNum() const
{
    uint32_t curLevel = level();
    const auto& cfg = ExpConfig::me().expCfg;
    if(curLevel >= cfg.m_expMap.size())
        return 0;

    uint32_t count = 0;
    for(auto pos = cfg.m_expMap.begin(); pos != cfg.m_expMap.end(); ++pos)
    {
        if(getExp() < pos->second.needExp_hero)
            continue;

        if(turnLifeLevel() < pos->second.needTurnLife)
            break;

        count++;
    }

    if(count <= curLevel)
        return 0;

    return SAFE_SUB(count, curLevel);
}

void Hero::levelUp(uint32_t upNum/* = 1*/, bool GmFlag/* = false*/)
{
    Role::Ptr role = getOwner();
    if(role == nullptr)
        return;

    uint32_t curLevel = level();
    uint32_t nextLevel = curLevel + upNum;

    const auto& cfg = ExpConfig::me().expCfg;
    if(curLevel >= cfg.m_expMap.size())
        return;

    auto pos = cfg.m_expMap.find(nextLevel);
    if(pos == cfg.m_expMap.end())
        return;

    const uint64_t needExp = pos->second.needExp_hero;

    if(GmFlag)
        m_exp = needExp;
    if(m_exp < needExp)
        return;

    setLevel(nextLevel);

    //策划要求升级后满血满蓝
    setHp(getMaxHp());
    setMp(getMaxMp());

    // world -> db
    if(!updateLevelAndExpToDB())
        return;

    // S -> C
    sendMainToMe();
    syncScreenDataTo9();
    m_skillM.unlockSkill();
    LOG_TRACE("英雄, 升级, roleId={}, hero={}, level={}->{}, exp={}, needExp={}", 
              role->id(), job(), curLevel, nextLevel, m_exp, needExp);

    return;
}

bool Hero::updateLevelAndExpToDB()
{
    Role::Ptr role = getOwner();
    if(role == nullptr)
        return false;

    PrivateRaw::UpdateHeroLevelExp send;
    send.roleId = role->id();
    send.job = job();
    send.level = level();
    send.exp = getExp();

    ProcessIdentity dbcachedId("dbcached", 1);
    const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateHeroLevelExp), &send, sizeof(send));
    LOG_TRACE("英雄，经验，send UpdateHeroLevelExp to {}, {}, roleId={}, hero={}, level={}, exp={}",
              dbcachedId, ret ? "ok" : "falied",
              send.roleId,  send.job, send.level, send.exp);

    return ret;
}

void Hero::sendHeroCurLevelGotExp()
{
    Role::Ptr role = getOwner();
    if(role == nullptr)
        return;

    PublicRaw::RetHeroCurLevelGotExp send;
    send.heroId = id();
    send.gotExp = getCurLevelGotExp();
    send.expRate = 0;
    uint64_t needExp = getLevelUpNeedExp();
    if(needExp > 0)
        send.expRate = 1000 * send.gotExp / getLevelUpNeedExp();
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetHeroCurLevelGotExp), &send, sizeof(send));
}

//当前等级已获得的经验值 == 累计经验值 - 当前等级需要经验值
uint64_t Hero::getCurLevelGotExp() const
{
    const auto& cfg = ExpConfig::me().expCfg;
    auto pos = cfg.m_expMap.find(level());
    if(pos == cfg.m_expMap.end())
        return 0;

    uint64_t curLevelNeedExp = pos->second.needExp_hero;
    return SAFE_SUB(getExp(), curLevelNeedExp);
}

// 升级需要经验值 == 下一级需要经验值 - 当前等级需要经验值
uint64_t Hero::getLevelUpNeedExp() const
{
    uint32_t curLevel = level();
    uint32_t nextLevel = curLevel + 1;

    const auto& cfg = ExpConfig::me().expCfg;
    auto iterCur = cfg.m_expMap.find(curLevel);
    if(iterCur == cfg.m_expMap.end())
        return 0;

    uint64_t curLevelNeedExp = iterCur->second.needExp_hero;
    if(curLevel == cfg.m_expMap.size())	//当前为最大等级时，则累计经验值 - 当前等级需要经验值
    {
        auto pos = cfg.m_expMap.find(curLevel);
        if(pos == cfg.m_expMap.end())
            return 0;

        return SAFE_SUB(getExp(), curLevelNeedExp);
    }

    auto iterNext = cfg.m_expMap.find(nextLevel);
    if(iterNext == cfg.m_expMap.end())
        return 0;

    uint64_t nextLevelNeedExp = iterNext->second.needExp_hero;
    return SAFE_SUB(nextLevelNeedExp, curLevelNeedExp);
}

void Hero::autoAddHpAndMp()
{
	if(isDead())
		return;

    uint32_t maxHp = getMaxHp();
    uint32_t maxMp = getMaxMp();
	uint32_t addHp = MAX(SAFE_DIV(maxHp * 50, 1000), 10);
	uint32_t addMp = MAX(SAFE_DIV(maxMp * 50, 1000), 10);
	if(isFight())
	{
		addHp = MAX(SAFE_DIV(maxHp * 2, 1000), 2);
		addMp = MAX(SAFE_DIV(maxMp * 2, 1000), 2);
	}

	if(getHp() + addHp >= maxHp)
	{
		addHp = SAFE_SUB(maxHp, getHp());
	}
	if(getMp() + addMp >= maxMp)
	{
		addMp = SAFE_SUB(maxMp, getMp());
	}

	if(0 != addHp)
	{
        changeHpAndNotify(nullptr, addHp, HPChangeType::recovery);
	}
	if(0 != addMp)
	{
        changeMpAndNotify(addMp);
	}
}


/*
 * 生命上限
 */
uint32_t Hero::getMaxHp() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(maxhp);
    }

    ret += m_packageSet.getHp(getEquipPackageType());
    ret += m_packageSet.getHp(getStonePackageType());
    ret += owner->m_horse.getMaxHp(SceneItemType::hero);

    //计算有效生命
    ret = ret * (1000 + getHpRatio() + (std::pow(getHpLv(), PK_PARAM.hpParam1()/1000) / PK_PARAM.hpParam2()/1000 - PK_PARAM.hpParam3()/1000) * 1000) / 1000;
    //buff计算放最后面, 下面的所有属性计算都应满足这一点
    ret = m_buffM.getHpMpBuffConstProp(PropertyType::maxhp, ret);
    return ret;
}


/*
 * 生命加成
 */
uint32_t Hero::getHpRatio() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(addhpRatio);
    }

    ret += owner->m_horse.getHpRatio(SceneItemType::hero);
    ret = m_buffM.getHpMpBuffPercentProp(PropertyType::addhpRatio, ret);
    return ret;
}


/*
 * 生命等级
 */
uint32_t Hero::getHpLv() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(hpLv);
    }

    ret += m_packageSet.getHpLv(getEquipPackageType());
    ret += m_packageSet.getHpLv(getStonePackageType());
    ret += owner->m_horse.getHpLv(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::hpLv, ret);
    return ret;
}



/*
 * 魔法上限
 */
uint32_t Hero::getMaxMp() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(maxmp);
    }

    ret += m_packageSet.getMp(getEquipPackageType());
    ret += m_packageSet.getMp(getStonePackageType());
    ret += owner->m_horse.getMaxMp(SceneItemType::hero);

    //有效魔法
    ret = ret * (1000 + getMpRatio() + (std::pow(getMpLv(), PK_PARAM.mpParam1()/1000) / PK_PARAM.mpParam2()/1000 - PK_PARAM.mpParam3()/1000) * 1000) / 1000;
    ret = m_buffM.getHpMpBuffConstProp(PropertyType::maxmp, ret);
    return ret;
}


/*
 * 魔法加成
 */
uint32_t Hero::getMpRatio() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(addmpRatio);
    }

    ret += owner->m_horse.getMpRatio(SceneItemType::hero);
    ret += m_buffM.getHpMpBuffPercentProp(PropertyType::addmpRatio, ret);
    return ret;
}


/*
 * 魔法等级
 */
uint32_t Hero::getMpLv() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(mpLv);
    }

    ret += m_packageSet.getMpLv(getEquipPackageType());
    ret += m_packageSet.getMpLv(getStonePackageType());
    ret += owner->m_horse.getMpLv(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::mpLv, ret);
    return ret;
}


/*
 * 最小物攻
 */
uint32_t Hero::getTotalPAtkMin() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_attackMin);
    }

    ret += m_packageSet.getPAtkMin(getEquipPackageType());
    ret += m_packageSet.getPAtkMin(getStonePackageType());
    ret += owner->m_horse.getPAtkMin(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::p_attackMin, ret);
    return ret;
}


/*
 * 最大物攻
 */
uint32_t Hero::getTotalPAtkMax() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_attackMax);
    }

    ret += m_packageSet.getPAtkMax(getEquipPackageType());
    ret += m_packageSet.getPAtkMax(getStonePackageType());
    ret += owner->m_horse.getPAtkMax(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::p_attackMax, ret);
    return ret;
}


/*
 * 最小魔攻
 */
uint32_t Hero::getTotalMAtkMin() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_attackMin);
    }

    ret += m_packageSet.getMAtkMin(getEquipPackageType());
    ret += m_packageSet.getMAtkMin(getStonePackageType());
    ret += owner->m_horse.getMAtkMin(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::m_attackMin, ret);
    return ret;
}


/*
 * 最大魔攻
 */
uint32_t Hero::getTotalMAtkMax() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_attackMax);
    }

    ret += m_packageSet.getMAtkMax(getEquipPackageType());
    ret += m_packageSet.getMAtkMax(getStonePackageType());
    ret += owner->m_horse.getMAtkMax(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::m_attackMax, ret);
    return ret;
}


/*
 * 最小道术
 */
uint32_t Hero::getTotalWitchMin() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(witchMin);
    }

    ret += m_packageSet.getWitchMin(getEquipPackageType());
    ret += m_packageSet.getWitchMin(getStonePackageType());
    ret += owner->m_horse.getWitchMin(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::witchMin, ret);
    return ret;
}


/*
 * 最大道术
 */
uint32_t Hero::getTotalWitchMax() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(witchMax);
    }

    ret += m_packageSet.getWitchMax(getEquipPackageType());
    ret += m_packageSet.getWitchMax(getStonePackageType());
    ret += owner->m_horse.getWitchMax(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::witchMax, ret);
    return ret;
}


/*
 * min物防
 */
uint32_t Hero::getTotalPDefMin() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_defenceMin);
    }

    ret += m_packageSet.getPDefMin(getEquipPackageType());
    ret += m_packageSet.getPDefMin(getStonePackageType());
    ret += owner->m_horse.getPDefMin(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::p_defenceMin, ret);
    return ret;
}


/*
 * max物防
 */
uint32_t Hero::getTotalPDefMax() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_defenceMax);
    }

    ret += m_packageSet.getPDefMax(getEquipPackageType());
    ret += m_packageSet.getPDefMax(getStonePackageType());
    ret += owner->m_horse.getPDefMax(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::p_defenceMax, ret);
    return ret;
}


/*
 * min魔防
 */
uint32_t Hero::getTotalMDefMin() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_defenceMin);
    }

    ret += m_packageSet.getMDefMin(getEquipPackageType());
    ret += m_packageSet.getMDefMin(getStonePackageType());
    ret += owner->m_horse.getMDefMin(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::m_defenceMin, ret);
    return ret;
}


/*
 * max魔防
 */
uint32_t Hero::getTotalMDefMax() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_defenceMax);
    }

    ret += m_packageSet.getMDefMax(getEquipPackageType());
    ret += m_packageSet.getMDefMax(getStonePackageType());
    ret += owner->m_horse.getMDefMax(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::m_defenceMax, ret);
    return ret;
}


/*
 * 幸运值
 */
uint32_t Hero::getTotalLucky() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(lucky);
    }

    ret += m_packageSet.getLucky(getEquipPackageType());
    ret += m_packageSet.getLucky(getStonePackageType());
    ret += owner->m_horse.getLucky(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::lucky, ret);
    return ret;
}


/*
 * 诅咒值
 */
uint32_t Hero::getTotalEvil() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(evil);
    }

    ret += m_packageSet.getEvil(getEquipPackageType());
    ret += m_packageSet.getEvil(getStonePackageType());
    ret += owner->m_horse.getEvil(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::evil, ret);
    return ret;
}


/*
 * 命中
 */
uint32_t Hero::getTotalShot() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(shot);
    }

    ret += m_packageSet.getShot(getEquipPackageType());
    ret += m_packageSet.getShot(getStonePackageType());
    ret += owner->m_horse.getShot(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::shot, ret);
    return ret;
}


/*
 * 命中率
 */
uint32_t Hero::getTotalShotRatio() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(shotRatio);
    }

    ret += m_packageSet.getShotRatio(getEquipPackageType());
    ret += m_packageSet.getShotRatio(getStonePackageType());
    ret += owner->m_horse.getShotRatio(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::shotRatio, ret);
    return ret;
}


/*
 * 物闪
 */
uint32_t Hero::getTotalPEscape() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_escape);
    }

    ret += m_packageSet.getPEscape(getEquipPackageType());
    ret += m_packageSet.getPEscape(getStonePackageType());
    ret += owner->m_horse.getPEscape(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::p_escape, ret);
    return ret;
}


/*
 * 魔闪
 */
uint32_t Hero::getTotalMEscape() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_escape);
    }

    ret += m_packageSet.getMEscape(getEquipPackageType());
    ret += m_packageSet.getMEscape(getStonePackageType());
    ret += owner->m_horse.getMEscape(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::m_escape, ret);
    return ret;
}


/*
 * 闪避率
 */
uint32_t Hero::getTotalEscapeRatio() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(escapeRatio);
    }

    ret += m_packageSet.getEscapeRatio(getEquipPackageType());
    ret += m_packageSet.getEscapeRatio(getStonePackageType());
    ret += owner->m_horse.getEscapeRatio(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::escapeRatio, ret);
    return ret;
}


/*
 * 暴击
 */
uint32_t Hero::getTotalCrit() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(crit);
    }

    ret += m_packageSet.getCrit(getEquipPackageType());
    ret += m_packageSet.getCrit(getStonePackageType());
    ret += owner->m_horse.getCrit(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::crit, ret);
    return ret;
}


/*
 * 暴击率
 */
uint32_t Hero::getTotalCritRatio() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(critRatio);
    }

    ret += m_packageSet.getCritRatio(getEquipPackageType());
    ret += m_packageSet.getCritRatio(getStonePackageType());
    ret += owner->m_horse.getCritRatio(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::critRatio, ret);
    return ret;
}


/*
 * 坚韧
 */
uint32_t Hero::getTotalAntiCrit() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(antiCrit);
    }

    ret += m_packageSet.getAntiCrit(getEquipPackageType());
    ret += m_packageSet.getAntiCrit(getStonePackageType());
    ret += owner->m_horse.getAntiCrit(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::antiCrit, ret);
    return ret;
}


/*
 * 暴伤
 */
uint32_t Hero::getTotalCritDmg() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(critDamage);
    }

    ret += m_packageSet.getCritDamage(getEquipPackageType());
    ret += m_packageSet.getCritDamage(getStonePackageType());
    ret += owner->m_horse.getCritDmg(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::critDamage, ret);
    return ret;
}


/*
 * 增伤
 */
uint32_t Hero::getTotalDmgAdd() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(damageAdd);
    }

    ret +=  m_packageSet.getDamageAdd(getEquipPackageType());
    ret +=  m_packageSet.getDamageAdd(getStonePackageType());
    ret += owner->m_horse.getDmgAdd(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::damageAdd, ret);
    return ret;
}


/*
 * 增伤等级
 */
uint32_t Hero::getTotalDmgAddLv() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(damageAddLv);
    }

    ret += m_packageSet.getDamageAddLv(getEquipPackageType());
    ret += m_packageSet.getDamageAddLv(getStonePackageType());
    ret += owner->m_horse.getDmgAddLv(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::damageAddLv, ret);
    return ret;
}


/*
 * 减伤
 */
uint32_t Hero::getTotalDmgReduce() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(damageReduce);
    }

    ret += m_packageSet.getDamageReduce(getEquipPackageType());
    ret += m_packageSet.getDamageReduce(getStonePackageType());
    ret += owner->m_horse.getDmgReduce(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::damageReduce, ret);
    return ret;
}


/*
 * 减伤等级
 */
uint32_t Hero::getTotalDmgReduceLv() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(damageReduceLv);
    }

    ret += m_packageSet.getDamageReduceLv(getEquipPackageType());
    ret += m_packageSet.getDamageReduceLv(getStonePackageType());
    ret += owner->m_horse.getDmgReduceLv(SceneItemType::hero);
    ret = m_buffM.getBuffProps(PropertyType::damageReduceLv, ret);
    return ret;
}


/*
 * 防爆(装备掉落)
 */
uint32_t Hero::getTotalAntiDropEquip() const
{
    Role::Ptr owner = getOwner();
    if(nullptr == owner)
        return 0;
    uint32_t ret = 0;
    for(const auto& iter : m_attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(antiDropEquip);
    }

    ret += m_packageSet.getAntiDropEquip(getEquipPackageType());
    ret += m_packageSet.getAntiDropEquip(getStonePackageType());
    ret += owner->m_horse.getAntiDropEquip(SceneItemType::hero);
    return ret;
}

uint32_t Hero::getMinStrongLevel() const
{
    PackageType packageType = getEquipPackageType();
    if(packageType == PackageType::none)
        return 0;

    EquipPackage::Ptr packagePtr = std::static_pointer_cast<EquipPackage>(m_packageSet.getPackageByPackageType(packageType));
    if(packagePtr == nullptr)
        return 0;

    return packagePtr->getMinStrongLevel();
}

uint32_t Hero::getStoneTotalLevel() const
{
    PackageType packageType = getStonePackageType();
    if(packageType == PackageType::none)
        return 0;

    StonePackage::Ptr packagePtr = std::static_pointer_cast<StonePackage>(m_packageSet.getPackageByPackageType(packageType));
    if(packagePtr == nullptr)
        return 0;

    return packagePtr->getStoneTotalLevel();
}
/*
   void Hero::moveTo(Coord2D goal) const
   {
   PublicRaw::CommandHeroMoveTo send;
   send.posX = goal.x;
   send.posY = goal.y;

   sendToMe(RAWMSG_CODE_PUBLIC(CommandHeroMoveTo), &send, sizeof(send));  
   }
   */

bool Hero::stop()
{
    auto& path = m_ai.moveData.path;
    if(!path.empty())
    {
        auto it = path.begin();
        path.erase(++it, path.end());
        return false;
    }

    //    //路径已经为空, 同步一次当前坐标位置
    //    syncNewPosTo9Screens(MoveType::walk);
    return m_ai.moveData.nextStepAble == componet::EPOCH || m_ai.moveData.nextStepAble < componet::Clock::now();
}

void Hero::moveToNearby(Coord2D goal, Coord1D nearby /*= 0*/)
{
    m_ai.moveData.goal = goal;
    m_ai.moveData.nearby = nearby;

    //正在向那边走, 不用再寻路了
    if(!m_ai.moveData.path.empty() &&
       Coord2D::distance(m_ai.moveData.path.back(), goal) <= nearby)
        return;

    auto s = scene();
    if(s == nullptr)
        return;

    componet::TimePoint now = componet::Clock::now();

    Coord2D start = m_ai.moveData.path.empty() ? pos() : m_ai.moveData.path.front();
    m_ai.moveData.path = s->findPath(start, goal, SceneItemType::npc);
#ifdef WATER_DEBUG_HEROAI
    s->highlightMapArea(getOwner(), m_ai.moveData.path);
#endif
    if(!m_ai.moveData.path.empty() && m_ai.moveData.nextStepAble < now)//说明当前立刻可以开走
    {
        m_ai.moveData.nextStepAble = now;
        followThePath(now);
    }
}

void Hero::followThePath(const componet::TimePoint& now)
{
    if(m_ai.moveData.path.empty())
        return;

    auto s = scene();
    if(s == nullptr)
        return;

    //当前这一步还没走完
    if(now < m_ai.moveData.nextStepAble)
        return;

    //已经走到了
    if(Coord2D::distance(pos(), m_ai.moveData.goal) <= m_ai.moveData.nearby)
    {
        m_ai.moveData.path.clear();
        return;
    }

    auto caculateNextStep = [](Coord2D curPos, std::list<Coord2D>& path) -> MoveType
    {
        auto it = path.begin();
        if(it == path.end())
            return MoveType::none;

        auto firstPos = *it;
        auto distance = std::max(std::abs(curPos.x - firstPos.x), std::abs(curPos.y - firstPos.y));
        if(distance == 1)
        {
            if(++it != path.end())
            {
                auto secondPos = *it;
                if(curPos.direction(firstPos) == firstPos.direction(secondPos))
                {
                    path.pop_front();
                    return MoveType::run;
                }
            }
            return MoveType::walk;
        }

        if(distance == 2)
            return MoveType::run;

        return MoveType::none;
    };

    auto curPos = pos();
    auto moveType = caculateNextStep(curPos, m_ai.moveData.path);
    if(moveType == MoveType::none)
    {
        m_ai.moveData.path.clear();
        return;
    }

    auto nextPos = m_ai.moveData.path.front();

    LOG_DEBUG("hero followPath to {}", nextPos);
    if(!changePos(nextPos, curPos.direction(nextPos), moveType))
    {
        m_ai.moveData.path = s->findPath(pos(), m_ai.moveData.goal, SceneItemType::npc);
        if(m_ai.moveData.path.empty())
            return;

        moveType = caculateNextStep(curPos, m_ai.moveData.path);
        if(moveType == MoveType::none)
        {
            m_ai.moveData.path.clear();
            return;
        }
        nextPos = m_ai.moveData.path.front();

        if(!changePos(nextPos, curPos.direction(nextPos), moveType)) //不可能
        {
            LOG_ERROR("hero 重寻路成功但路径不可用, {}->{}", pos(), m_ai.moveData.path.front());
            m_ai.moveData.path.clear();
            return;
        }
        s->highlightMapArea(getOwner(), m_ai.moveData.path);
    }

    m_ai.moveData.path.pop_front();
    if(moveType == MoveType::walk)
        m_ai.moveData.nextStepAble = m_ai.moveData.nextStepAble + m_ai.moveData.stepWalkInterval;
    else //if(moveType == MoveType::run)
        m_ai.moveData.nextStepAble = m_ai.moveData.nextStepAble + m_ai.moveData.stepRunInterval;

    m_ai.moveData.lastStepDone = m_ai.moveData.lastStepDone + m_ai.moveData.stepDuration;
}

bool Hero::backToOwnerNearby(bool forceBack)
{
    Role::Ptr owner = getOwner();
    if(owner == nullptr)
        return false;

    auto ownerPos = owner->pos();
    auto ownerBackDir = componet::reverseDirection(owner->dir());
    auto idlePos = ownerPos.neighbor(ownerBackDir).neighbor(ownerBackDir);
    auto curPos  = pos();

    if(forceBack)
    {
        lockOnTarget(nullptr);
        for(auto item : m_ai.targets)
            item.second.reset();
        moveToNearby(idlePos, 0);
        return true;
    }

    auto distance = Coord2D::distance(idlePos, curPos);
    if(distance > AI::MAX_FOLLOW_DISTANCE) //超出最大视野距离, 立刻闪现返回
    {
        changePos(idlePos, owner->dir(), MoveType::blink);
        m_ai.moveData.path.clear();
    }
    else if(distance > AI::MAX_ACTION_DISTANCE) //超出最大互动距离, 走回来
    {
        moveToNearby(idlePos, 0);
    }
    else
    {
        return false;
    }

    lockOnTarget(nullptr);
    for(auto item : m_ai.targets)
        item.second.reset();
    return true;
}

void Hero::aiLoop(const componet::TimePoint& now)
{
    if(isDead())
        return;

    Role::Ptr owner = getOwner();
    if(owner == nullptr)
        return;

    auto s = scene();
    if(s == nullptr)
        return;

    //优先级1: 不能离主人太远
    if(backToOwnerNearby()) //需要回到主人身边走
        return;

    //优先级2: 释放合击技能
    PK::Ptr target(nullptr);
    if(owner->isJointReadyState())
    {
#ifdef WATER_DEBUG_HEROAI
        LOG_DEBUG("inJointState");
#endif
        auto jointSkillId = HeroConfig::me().getJointSkillId(owner->job(), job());
        auto skill = owner->m_skillM.find(jointSkillId);
        if(skill != nullptr)
        {
            auto attRange = skill->getMaxHeroDistance();
            target = owner->target();
            if(target != nullptr)
            {
                if(attRange < Coord2D::distance(target->pos(), pos()))
                {
                    moveToNearby(target->pos(), attRange);
                    return;
                }
//                lockOnTarget(target, true);
                //尝试放一下合击技能
                if(AttackRetcode::success == owner->launchAttack(jointSkillId, target->pos()))
                {
                    owner->sendSysChat(ChannelType::system, "release JS to {} 成功", target->name());
                    return;
                }
                owner->sendSysChat(ChannelType::system, "release JS to {} 失败", target->name());
            }
        }
    }

    //优先级3: 依据ai模式挑选攻击目标
    auto isLegalTarget = [s, owner](PK::Ptr target)->bool
    {
        if(target == nullptr)
            return false;

        if(target->isDead())
            return false;

        if(target->scene() != s)
            return false;

        if(target->pos().distance(owner->pos()) > AI::MAX_ACTION_DISTANCE)
            return false;
        return true;
    };

    do{
        //先检查现有目标
        if(isLegalTarget(target))
            break;

        //第1优先目标: 玩家手动指定
        target = m_ai.targets[AI::TargetPriority::manual].lock();
        if(isLegalTarget(target))
            break;
        target = nullptr;

        //第2优先目标: 和平模式时不参与pk
        if(m_ai.mode == AIMode::peaceful)
            break;

        //第3优先目标: 在主人攻击精英boss和参与pk时, 与主人的目标保持目标一致
        if(m_ai.ownerLastAttSthTime + std::chrono::milliseconds(3000) < now)
            m_ai.targets[AI::TargetPriority::attByOwner].reset();
        target = m_ai.targets[AI::TargetPriority::attByOwner].lock();
        if(isLegalTarget(target))
        {
            if(target->sceneItemType() == SceneItemType::npc)
            {
                auto npc = std::static_pointer_cast<Npc>(target);
                auto type = npc->type();
                if(type == NpcType::elite || type == NpcType::boss || type == NpcType::mob)
                    break;
            }
            else if(target->sceneItemType() == SceneItemType::hero || target->sceneItemType() == SceneItemType::role)
                break;
        }
        target = nullptr;

        //第4优先级目标: 有人正在攻击主人或攻击自己, 打他
        if(m_ai.beAttackTime + std::chrono::seconds(AI::ATTBACK_DURATION_SEC) < now)
            m_ai.targets[AI::TargetPriority::attUs].reset();
        target = m_ai.targets[AI::TargetPriority::attUs].lock();
        if(isLegalTarget(target))
            break;
        m_ai.targets[AI::TargetPriority::attUs].reset();
        target = nullptr;

        //第5优先级目标: 攻击自己找的目标
        if(m_ai.mode != AIMode::initiative)
            break;
        target = m_ai.targets[AI::TargetPriority::foundByMyself].lock();
        if(isLegalTarget(target))
            break;

        //目标已失效, 需要重新寻找
        m_ai.targets[AI::TargetPriority::foundByMyself].reset();
        target = nullptr;

        //遍历周围目标单位, 寻找目标
        auto findTargetNpc = [s, &isLegalTarget, &target](const Coord2D& pos) -> bool
        {
            auto grid = s->getGridByGridCoord(pos);
            if(grid == nullptr)
                return false;
            auto npcs = grid->getAllNpcs();
            for(auto npc : npcs)
            {
                auto type = npc->type();
                if(type != NpcType::mob && type != NpcType::elite && type != NpcType::boss)
                    continue;
                if(isLegalTarget(npc))
                {
                    target = npc;
                    break;
                }
            }
            return false;
        };
        s->tryExecSpiral(owner->pos(), 10, findTargetNpc);
        if(target != nullptr)
            m_ai.targets[AI::TargetPriority::foundByMyself] = target;
    } while(false);

    //有目标则攻击, 没有就回到主人身边, done
    if(target == nullptr)
        backToOwnerNearby(true);
    else
        lockOnAndAttack(target);
}

void Hero::lockOnAndAttack(PK::Ptr target)
{
    if(target == nullptr)
        return;

    auto s = scene();
    if(s == nullptr)
        return;

    auto owner = getOwner();
    if(owner == nullptr)
        return;

    if(!checkPublicCDTime())
    {
        stop();
        return;
    }

    const auto& cfg = HeroConfig::me().getAICfg(job());
    if(cfg == nullptr)
    {
        moveToNearby(target->pos(), 2); //没找到可用配置, 仅仅向着目标移动即可
        return;
    }

    const auto& bannedSkills = owner->m_roleSundry.heroSkillSetting(job()); //已禁用技能
    auto isBanned = [this, &bannedSkills](TplId skillTplId) -> bool
    {
        return bannedSkills.find(skillTplId) != bannedSkills.end();
    };

    auto tryAttackWithTheSkill = [this, target, &bannedSkills](TplId skillTplId) -> bool
    {
        auto skill = m_skillM.find(skillTplId);
        if(skill == nullptr)
            return false;

        if(!skill->checkCDTime())
            return false;

        if(bannedSkills.find(skillTplId) != bannedSkills.end())
            return false;

        if(skill->getMaxDistance() >= Coord2D::distance(target->pos(), pos()))
        {
            if(stop())
            {
                bool ret = AttackRetcode::success == launchAttack(skillTplId, target->pos());
#ifdef WATER_DEBUG_HEROAI
                if(ret)
                    LOG_DEBUG("hero choosen skillId={}", skillTplId);
#endif
                return ret;
            }
        }
        else
        {
#ifdef WATER_DEBUG_HEROAI
            LOG_DEBUG("hero approch target,  skillId={}, attRange={}", skillTplId, skill->getMaxDistance());
#endif
            moveToNearby(target->pos(), skill->getMaxDistance());
        }
        return true;
    };

    TplId skillId = 0;

    {//技能释放优先级为: 高级技能 > 多目标普攻 > 单目标普攻 > 基础平砍
        //1 按优先级尝试高级技能
        for(auto skillTplId : cfg->skillPriority)
        {
            if(tryAttackWithTheSkill(skillTplId))
            {
                skillId = skillTplId;
                break;
            }
        }

        //2 尝试多体普攻, 蛋疼要死的地方
        if(skillId == 0 && !isBanned(cfg->multiTargetAttSkill)) //多体普攻
        {
            //检查是否适用多目标普攻
            bool betterThanSingleSkill = false;
            switch (job())
            {
            case Job::warrior://面前1格距离内的半圆
                {
                    auto target2MeDir = pos().direction(target->pos());
                    Direction relativeDirs[] =  //面前的5个方向
                    {
                        Direction::leftup, Direction::up, Direction::rightup,
                        Direction::left,                  Direction::right,
                    };
                    for(auto dir : relativeDirs)
                    {
                        auto absoluteDir = relativeDirection_2_absoluteDirection(target2MeDir, dir);
                        auto neighborPos = pos().neighbor(absoluteDir);
                        if(neighborPos == target->pos())
                            continue;
                        auto grid = s->getGridByGridCoord(neighborPos);
                        if(grid == nullptr)
                            continue;
                        if(grid->npcSize() > 0 || grid->roleSize() > 0 || 
                           grid->heroSize() > 0 || grid->petSize() > 0)
                        {
                            betterThanSingleSkill = true;
                            break;
                        }
                    }
                }
                break;
            case Job::magician://目标周围1格内
                {
                    auto allNeighborsPos = target->pos().allNeighbors();
                    for(auto neighborPos : allNeighborsPos)
                    {
                        if(neighborPos == pos())
                            continue;
                        auto grid = s->getGridByGridCoord(neighborPos);
                        if(grid == nullptr)
                            continue;
                        if(grid->npcSize() > 0 || grid->roleSize() > 0 || 
                           grid->heroSize() > 0 || grid->petSize() > 0)
                        {
                            betterThanSingleSkill = true;
                            break;
                        }
                    }
                }
                break;
            case Job::taoist://直接用群攻
                break;
            default:
                return;
            }
            if(betterThanSingleSkill && tryAttackWithTheSkill(cfg->multiTargetAttSkill))
                skillId = cfg->multiTargetAttSkill;
        }
        //3 尝试单体普攻
        if(skillId == 0 && tryAttackWithTheSkill(cfg->singleTargetAttSkill))
            skillId = cfg->singleTargetAttSkill;
        //4 尝试平砍
        if(skillId == 0 && tryAttackWithTheSkill(cfg->defaultSkill))
            skillId = cfg->defaultSkill;
    }
}


void Hero::onOwnerAttackSth(PK::Ptr target, const componet::TimePoint& tp)
{
    m_ai.targets[AI::TargetPriority::attByOwner] = target;
    if(target != nullptr)
        m_ai.ownerLastAttSthTime = tp;
}

void Hero::lockOnTarget(PK::Ptr target, bool silence)
{
    auto owner = getOwner();
    if(owner == nullptr)
        return;

    auto t = m_ai.targets[AI::TargetPriority::manual].lock();
    m_ai.targets[AI::TargetPriority::manual] = target;

    if(t != target)
    {
        PublicRaw::HeroLockOnTarget send;
        if(target != nullptr)
        {
            if(target->pos().distance(owner->pos()) > AI::MAX_ACTION_DISTANCE) //如果攻击距离过远
            {
                if(!silence)
                    owner->sendSysChat("目标过远, 无法锁定");
                return;
            }
            send.type = static_cast<decltype(send.type)>(target->sceneItemType());
            send.id = target->id();
        }
        sendToMe(RAWMSG_CODE_PUBLIC(HeroLockOnTarget), &send, sizeof(send));
    }
}

void Hero::setAIMode(AIMode mode)
{
    m_ai.mode = mode;
}

void Hero::underAttack(PK::Ptr atk)
{
    if(atk == nullptr)
        return;

    auto lastAtk = m_ai.targets[AI::TargetPriority::attUs].lock();
    if(lastAtk != nullptr)
    {
        if(lastAtk == atk)
        {
            m_ai.beAttackTime = m_lastTimerTime;
            return;
        }

        if(m_ai.beAttackTime + std::chrono::seconds(AI::ATTBACK_DURATION_SEC) > m_lastTimerTime)
            return;
    }

    m_ai.beAttackTime = m_lastTimerTime;
    m_ai.targets[AI::TargetPriority::attUs] = atk;
}

}

