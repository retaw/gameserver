#include "horse.h"
#include "role.h"
#include "world.h"
#include "object_config.h"
#include "store_manager.h"
#include "scene.h"
#include "water/componet/serialize.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/private/horse.h"
#include "protocol/rawmsg/private/horse.codedef.private.h"

#include "protocol/rawmsg/public/horse.h"
#include "protocol/rawmsg/public/horse.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

const uint32_t rideCD = 3;

HorseLevelProp::HorseLevelProp(const SceneItemType sceneItem)
: m_sceneItem(sceneItem)
{
}

void HorseLevelProp::updateLevelProp(HorseTrainTpl::Ptr horseTrainTpl)
{
    if(nullptr == horseTrainTpl)
        return;
    if(m_sceneItem == SceneItemType::role)
    {
        if(horseTrainTpl->roleProps.empty())
            return;
        setAttribute(horseTrainTpl->roleProps);
    }
    else
    {
        if(horseTrainTpl->heroProps.empty())
            return;
        setAttribute(horseTrainTpl->heroProps);
    }
}



void HorseSkinProp::updateSkinProp(HorseSkinTpl::Ptr horseSkinTpl)
{
    if(nullptr == horseSkinTpl)
        return;

    addAttribute(horseSkinTpl->skinProps);
}



//======================== 华丽丽的分割线 =========================
//
Horse::Horse(Role& me, uint8_t state)
: m_open(0)
, m_saveFlag(false)
, m_owner(me)
, m_rideState(static_cast<RideState>(state))
, m_star(1)
, m_exp(0)
, m_roleProps(SceneItemType::role)
, m_heroProps(SceneItemType::hero)
, m_curskin(0)
{
    openHorse();
}

void Horse::initBasicData()
{
    //内网测试强制开启该功能
    m_open = 1;
    if(m_star == 0)
        m_star = 1;
    setHorseTrainTpl();
    m_roleProps.updateLevelProp(m_horseTrainTpl);
    m_heroProps.updateLevelProp(m_horseTrainTpl);
}

void Horse::initFirstSkin()
{
    if(nullptr == m_horseTrainTpl)
        return;
    m_curskin = m_horseTrainTpl->skin;
    m_skinSet.insert(m_curskin);
}

void Horse::initSkinList()
{
    if(!isOpen())
        return;

    if(0 == m_curskin)
        initFirstSkin();

    for(const auto& iter : m_skinSet)
    {
        HorseSkinTpl::Ptr horseSkinTpl = HorseBase::me().getSkinTpl(iter);
        if(nullptr == horseSkinTpl)
            continue;
        m_skinProps.updateSkinProp(horseSkinTpl);
    }
}

void Horse::initRaiseRate()
{
    if(!isOpen())
        return;
    if(m_raiseRate.empty())
    {
        auto exec = [this] (HorseRateTpl::Ptr horseRateTpl) -> void
        {
            std::pair<uint16_t, uint8_t> temp;
            temp.first = 0;
            temp.second = 0;
            m_raiseRate.insert({horseRateTpl->rate, temp});
        };

        HorseBase::me().execRate(exec);
    }
}

void Horse::setHorseTrainTpl()
{
    m_horseTrainTpl = HorseBase::me().getTrainTpl(m_star);
}

bool Horse::isMaxStar() const
{
    return nullptr == HorseBase::me().getTrainTpl(m_star+1);
}

uint8_t Horse::getRaiseRate() const
{
    uint16_t totalRand = 0;
    uint8_t rate = 1;
    std::vector<std::pair<uint8_t, uint16_t>> raiseWeights;
    for(const auto& iter : m_raiseRate)
    {
        HorseRateTpl::Ptr horseRateTpl = HorseBase::me().getRateTpl(iter.first);
        if(nullptr == horseRateTpl)
            continue;
        if(iter.second.first < horseRateTpl->needRaiseCount) //未激活
            continue;
        if(0 != horseRateTpl->effectNum && 0 == iter.second.second) //已激活必暴
        {
            rate = rate < iter.first ? iter.first : rate;
            continue;
        }

        totalRand += horseRateTpl->weight;
        raiseWeights.push_back(std::make_pair(iter.first, horseRateTpl->weight));
    }

    if(rate <= 1)
    {
        Random<uint16_t> r(1, totalRand);
        uint16_t rand = r.get();
        uint16_t min = 0;
        uint16_t max = 0;
        for(const auto& iter : raiseWeights)
        {
            min = max;
            max += iter.second;
            if(rand <= max && rand >= min)
            {
                rate = iter.first;
                break;
            }
        }
        LOG_DEBUG("坐骑, 培养, 触发暴击倍率 rate={}, totalRand={}, rand={}", rate, totalRand, rand);
    }

    return rate;
}

void Horse::addRaiseCount()
{
    uint8_t maxRate = 1;
    for(auto& iter : m_raiseRate)
    {
        HorseRateTpl::Ptr horseRateTpl = HorseBase::me().getRateTpl(iter.first);
        if(nullptr == horseRateTpl)
            continue;
        if(iter.second.first >= horseRateTpl->needRaiseCount)
            continue;
        ++iter.second.first;
        if(iter.second.first >= horseRateTpl->needRaiseCount)
            maxRate = iter.first;
    }

    if(maxRate > 1)
        m_owner.sendSysChat("{}倍暴击已激活，下次培养必定触发该暴击", maxRate);
}

void Horse::subRateEffectNum(uint8_t rate)
{
    HorseRateTpl::Ptr horseRateTpl = HorseBase::me().getRateTpl(rate);
    if(nullptr == horseRateTpl)
        return;
    if(m_raiseRate.find(rate) == m_raiseRate.end())
        return;

    std::pair<uint16_t, uint8_t>& temp = m_raiseRate[rate];
    if(0 != horseRateTpl->effectNum 
       && ++temp.second >= horseRateTpl->effectNum)
    {//超过作用次数全部清空
        temp.first = 0;
        temp.second = 0;
    }
}

RideState Horse::rideState() const
{
    return m_rideState;
}

void Horse::clearRideState()
{
    m_rideState = RideState::off;
}

uint8_t Horse::star() const
{
    return m_star;
}

uint32_t Horse::curskin() const
{
    return m_curskin;
}

bool Horse::isRide() const
{
    if(!isOpen())
        return false;
    return RideState::on == m_rideState;
}

void Horse::onRide()
{
    if(!isOpen())
        return;
    if(RideState::on == m_rideState)
        return;
    auto s = m_owner.scene();
    if(nullptr == s)
        return;
    if(!s->canRide())
    {
        m_owner.sendSysChat("该场景无法骑乘");
        return;
    }

    if(m_owner.isFight())
    {
        m_owner.sendSysChat("战斗状态无法骑乘");
        return;
    }

    m_rideState = RideState::on;
    m_owner.m_heroManager.passiveRecallHero();
    broadcastRideTo9();
}

void Horse::offRide(bool isDead/*=false*/)
{
    if(!isOpen())
        return;
    if(RideState::off == m_rideState)
        return;
    m_rideState = RideState::off;

    if(!isDead)
    {
        broadcastRideTo9();
        if(m_owner.getSummonHeroFlag())
            m_owner.m_heroManager.requestSummonHero(m_owner.getDefaultCallHero());
    }
}

void Horse::openHorse()
{
    m_open = 1;
}

bool Horse::isOpen() const
{
    return 1 == m_open;
}

void Horse::retRaiseInfo() const
{
    if(!isOpen())
        return;
    PublicRaw::RetRaiseInfo ret;
    ret.star = m_star;
    ret.curskin = m_curskin;
    ret.exp = m_exp;

    uint32_t index = 0;
    for(const auto & iter : m_raiseRate)
    {
        if(1 == iter.first)
            continue;

        ret.ele[index].rate = iter.first;
        ret.ele[index].raiseCount = iter.second.first;
        ret.ele[index].effectNum = iter.second.second;
        ++index;
    }

    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetRaiseInfo), &ret, sizeof(ret));
}

void Horse::broadcastRideTo9() const
{
    auto s = m_owner.scene();
    if(nullptr == s)
        return;
    PublicRaw::BroadcastRide9 send;
    send.roleId = m_owner.id();
    send.state = static_cast<uint8_t>(m_rideState);
    send.skin = m_curskin;

    if(m_rideState == RideState::on)
    {
        HeroInfoPra param = m_owner.m_heroManager.getDefaultHeroInfoPra();
        send.heroJob = param.job;
        send.heroSex = param.sex;
        send.heroClother = param.clother;
    }

    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(BroadcastRide9), &send, sizeof(send), m_owner.pos());
}

void Horse::retRaiseResult(uint8_t retcode)
{
    PublicRaw::RetRaiseResult ret;
    ret.retcode = retcode;
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetRaiseResult), &ret, sizeof(ret));
}

/*
 * 坐骑培养接口
 */
void Horse::raiseHorse(TplId objId, uint8_t autoyb)
{
    if(!isOpen())
        return;
    if(nullptr == m_horseTrainTpl)
    {
        LOG_DEBUG("坐骑, 培养找不到配置, star={}", m_star);
        return;
    }
    if(m_horseTrainTpl->costObjs.find(objId) == m_horseTrainTpl->costObjs.end())
    {
        LOG_DEBUG("坐骑, 培养道具非法, objId:{}", objId);
        return;
    }

    if(isMaxStar())
    {
        retRaiseResult(0);
        m_owner.sendSysChat("坐骑已经满级了");
        return;
    }

    //检查金币是否足够
    if(!m_owner.checkMoney(m_horseTrainTpl->moneyType, m_horseTrainTpl->money))
    {
        retRaiseResult(0);
        LOG_DEBUG("坐骑, 培养, 金币不够({}, {})", m_horseTrainTpl->moneyType, m_horseTrainTpl->money);
        return;
    }

    if(ObjectConfig::me().objectCfg.m_objBasicDataMap.find(objId) == ObjectConfig::me().objectCfg.m_objBasicDataMap.end())
    {
        LOG_DEBUG("坐骑, 培养, 骑道具配置表中找不到该道具, objId={}", objId);
        return;
    }

    if(autoyb)
    {
        if(!m_owner.autoReduceObjMoney(objId, 1, "坐骑自动"))
        {
            retRaiseResult(0);
            return;
        }
    }
    else
    {
        if(m_owner.getObjNum(objId, PackageType::role) < 1)
        {//道具不够
            retRaiseResult(0);
            LOG_DEBUG("坐骑, owner () 培养, 材料不够, objId={}", m_owner.name(), objId);
            return;
        }
        m_owner.eraseObj(objId, 1, PackageType::role, "坐骑培养");
    }

    m_owner.reduceMoney(m_horseTrainTpl->moneyType, m_horseTrainTpl->money, "坐骑");

    ObjBasicData objbase = ObjectConfig::me().objectCfg.m_objBasicDataMap[objId];
    uint8_t rate = getRaiseRate();
    addRaiseCount();
    subRateEffectNum(rate);
    m_exp += rate * objbase.horseExp;
    while(m_exp >= m_horseTrainTpl->nextexp)
    {
        m_exp = SAFE_SUB(m_exp, m_horseTrainTpl->nextexp);
        upStar();
    }

    retRaiseResult(rate);
    retRaiseInfo();
    m_saveFlag = true;
}

void Horse::upStar()
{
    HorseTrainTpl::Ptr temp = HorseBase::me().getTrainTpl(m_star+1);
    if(nullptr == temp)
    {
        m_owner.sendSysChat("坐骑已满级了");
        return;
    }

    m_star += 1;
    m_horseTrainTpl = temp;
    m_roleProps.updateLevelProp(m_horseTrainTpl);
    m_heroProps.updateLevelProp(m_horseTrainTpl);

    if(1 == m_star % 10)
        activeSkin(temp->skin);

    m_owner.sendMainToMe();
    Hero::Ptr hero = m_owner.m_heroManager.getSummonHero();
    if(nullptr != hero && !m_horseTrainTpl->heroProps.empty())
        hero->sendMainToMe();

    m_saveFlag = true;
}

void Horse::activeSkin(uint16_t skin)
{
    if(0 == skin)
        return;
    if(m_skinSet.find(skin) != m_skinSet.end())
        return;

    m_skinSet.insert(skin);
    //更新皮肤属性
    HorseSkinTpl::Ptr horseSkinTpl = HorseBase::me().getSkinTpl(skin);
    m_skinProps.updateSkinProp(horseSkinTpl);
    changeSkin(skin);

    if("" != horseSkinTpl->notice)
        m_owner.sendSysChat(ChannelType::screen_middle, horseSkinTpl->notice, m_owner.name());
    m_saveFlag = true;
}

void Horse::changeSkin(uint16_t skin)
{
    if(m_skinSet.find(skin) == m_skinSet.end())
        return;
    m_curskin = skin;
    if(isRide())
    {
        auto s = m_owner.scene();
        if(nullptr != s)
        {
            PublicRaw::HuanHuaSkin ret;
            ret.roleId = m_owner.id();
            ret.skin = skin;
            s->sendCmdTo9(RAWMSG_CODE_PUBLIC(HuanHuaSkin), &ret, sizeof(ret), m_owner.pos());
        }
    }

    PublicRaw::RetCurSkin send;
    send.skin = skin;
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetCurSkin), &send, sizeof(send));
    m_saveFlag = true;
}

void Horse::retActiveSkins()
{
    std::vector<uint8_t> buf;
    buf.reserve(64);
    buf.resize(sizeof(PublicRaw::RetActiveSkins));
    for(const auto& iter : m_skinSet)
    {
        buf.resize(buf.size() + sizeof(uint16_t));
        auto msg = reinterpret_cast<PublicRaw::RetActiveSkins*>(buf.data());
        msg->skin[msg->size++] = iter;
    }

    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetActiveSkins), buf.data(), buf.size());
}

void Horse::afterEnterScene()
{
    auto s = m_owner.scene();
    if(nullptr == s)
        return;
    if(!s->canRide())
        m_rideState = RideState::off;
}

void Horse::leaveScene()
{
    saveDB();
}

void Horse::loadFromDB(const std::string& horseStr)
{
    Deserialize<std::string> ds(&horseStr);

    ds >> m_open;
    ds >> m_star;
    ds >> m_exp;
    ds >> m_curskin;
    ds >> m_skinSet;
    ds >> m_raiseRate;

    initBasicData();
    initSkinList();
    initRaiseRate();
}

void Horse::saveDB()
{
    if(!m_saveFlag)
        return;

    Serialize<std::string> iss;
    iss.reset();

    iss << m_open;
    iss << m_star;
    iss << m_exp;
    iss << m_curskin;
    iss << m_skinSet;
    iss << m_raiseRate;

    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PrivateRaw::ModifyHorseData) + iss.tellp());
    auto msg = reinterpret_cast<PrivateRaw::ModifyHorseData*>(buf.data());
    msg->roleId = m_owner.id();
    msg->size = iss.tellp();
    std::memcpy(msg->buf, iss.buffer()->data(), iss.tellp());

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(ModifyHorseData), buf.data(), buf.size());
    m_saveFlag = false;
}

void Horse::zeroClear()
{
    for(auto& iter : m_raiseRate)
    {
        iter.second.first = 0;
        iter.second.second = 0;
    }
}

uint32_t Horse::getMaxHp(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(maxhp);
        ret += m_skinProps.GET_ATTRIBUTE(maxhp);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(maxhp);

    return ret;
}

uint32_t Horse::getHpRatio(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(addhpRatio);
        ret += m_skinProps.GET_ATTRIBUTE(addhpRatio);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(addhpRatio);

    return ret;
}

uint32_t Horse::getHpLv(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(hpLv);
        ret += m_skinProps.GET_ATTRIBUTE(hpLv);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(hpLv);

    return ret;
}

uint32_t Horse::getMaxMp(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(maxmp);
        ret += m_skinProps.GET_ATTRIBUTE(maxmp);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(maxmp);

    return ret;
}

uint32_t Horse::getMpRatio(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(addmpRatio);
        ret += m_skinProps.GET_ATTRIBUTE(addmpRatio);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(addmpRatio);

    return ret;
}

uint32_t Horse::getMpLv(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(mpLv);
        ret += m_skinProps.GET_ATTRIBUTE(mpLv);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(mpLv);

    return ret;
}

uint32_t Horse::getPAtkMin(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(p_attackMin);
        ret += m_skinProps.GET_ATTRIBUTE(p_attackMin);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(p_attackMin);

    return ret;
}

uint32_t Horse::getPAtkMax(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(p_attackMax);
        ret += m_skinProps.GET_ATTRIBUTE(p_attackMax);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(p_attackMax);

    return ret;
}

uint32_t Horse::getMAtkMin(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(m_attackMin);
        ret += m_skinProps.GET_ATTRIBUTE(m_attackMin);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(m_attackMin);

    return ret;
}

uint32_t Horse::getMAtkMax(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(m_attackMax);
        ret += m_skinProps.GET_ATTRIBUTE(m_attackMax);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(m_attackMax);

    return ret;
}

uint32_t Horse::getWitchMin(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(witchMin);
        ret += m_skinProps.GET_ATTRIBUTE(witchMin);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(witchMin);

    return ret;
}

uint32_t Horse::getWitchMax(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(witchMax);
        ret += m_skinProps.GET_ATTRIBUTE(witchMax);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(witchMax);

    return ret;
}

uint32_t Horse::getPDefMin(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(p_defenceMin);
        ret += m_skinProps.GET_ATTRIBUTE(p_defenceMin);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(p_defenceMin);

    return ret;
}

uint32_t Horse::getPDefMax(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(p_defenceMax);
        ret += m_skinProps.GET_ATTRIBUTE(p_defenceMax);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(p_defenceMax);

    return ret;
}

uint32_t Horse::getMDefMin(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(m_defenceMin);
        ret += m_skinProps.GET_ATTRIBUTE(m_defenceMin);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(m_defenceMin);

    return ret;
}

uint32_t Horse::getMDefMax(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(m_defenceMax);
        ret += m_skinProps.GET_ATTRIBUTE(m_defenceMax);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(m_defenceMax);

    return ret;
}


uint32_t Horse::getLucky(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(lucky);
        ret += m_skinProps.GET_ATTRIBUTE(lucky);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(lucky);

    return ret;
}

uint32_t Horse::getEvil(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(evil);
        ret += m_skinProps.GET_ATTRIBUTE(evil);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(evil);

    return ret;
}

uint32_t Horse::getShot(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(shot);
        ret += m_skinProps.GET_ATTRIBUTE(shot);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(shot);

    return ret;
}

uint32_t Horse::getShotRatio(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(shotRatio);
        ret += m_skinProps.GET_ATTRIBUTE(shotRatio);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(shotRatio);

    return ret;
}

uint32_t Horse::getPEscape(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(p_escape);
        ret += m_skinProps.GET_ATTRIBUTE(p_escape);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(p_escape);

    return ret;
}

uint32_t Horse::getMEscape(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(m_escape);
        ret += m_skinProps.GET_ATTRIBUTE(m_escape);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(m_escape);

    return ret;
}

uint32_t Horse::getEscapeRatio(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(escapeRatio);
        ret += m_skinProps.GET_ATTRIBUTE(escapeRatio);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(escapeRatio);

    return ret;
}

uint32_t Horse::getCrit(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(crit);
        ret += m_skinProps.GET_ATTRIBUTE(crit);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(crit);

    return ret;
}

uint32_t Horse::getCritRatio(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(critRatio);
        ret += m_skinProps.GET_ATTRIBUTE(critRatio);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(critRatio);

    return ret;
}

uint32_t Horse::getAntiCrit(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(antiCrit);
        ret += m_skinProps.GET_ATTRIBUTE(antiCrit);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(antiCrit);

    return ret;
}

uint32_t Horse::getCritDmg(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(critDamage);
        ret += m_skinProps.GET_ATTRIBUTE(critDamage);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(critDamage);

    return ret;
}

uint32_t Horse::getDmgAdd(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(damageAdd);
        ret += m_skinProps.GET_ATTRIBUTE(damageAdd);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(damageAdd);

    return ret;
}

uint32_t Horse::getDmgAddLv(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(damageAddLv);
        ret += m_skinProps.GET_ATTRIBUTE(damageAddLv);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(damageAddLv);

    return ret;
}

uint32_t Horse::getDmgReduce(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(damageReduce);
        ret += m_skinProps.GET_ATTRIBUTE(damageReduce);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(damageReduce);

    return ret;
}

uint32_t Horse::getDmgReduceLv(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(damageReduceLv);
        ret += m_skinProps.GET_ATTRIBUTE(damageReduceLv);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(damageReduceLv);

    return ret;
}


uint32_t Horse::getAntiDropEquip(const SceneItemType sceneItem) const
{
    uint32_t ret = 0;
    if(sceneItem == SceneItemType::role)
    {
        ret += m_roleProps.GET_ATTRIBUTE(antiDropEquip);
        ret += m_skinProps.GET_ATTRIBUTE(antiDropEquip);
    }
    else
        ret += m_heroProps.GET_ATTRIBUTE(antiDropEquip);

    return ret;
}

}
