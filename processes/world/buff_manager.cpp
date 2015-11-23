#include "buff_manager.h"
#include "pk.h"
#include "scene_manager.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/buff.h"
#include "protocol/rawmsg/private/buff.codedef.private.h"
#include "protocol/rawmsg/public/buff_scene.h"
#include "protocol/rawmsg/public/buff_scene.codedef.public.h"

#include "world.h"

namespace world{

using namespace water;
using namespace water::componet;

enum SpecialBuffID : uint32_t
{
    shield_start    = 10161, //魔法盾buff开始ID
    shield_end      = 10162, //魔法盾结束ID
    poison_start    = 10211, //施毒术buff开始ID
    poison_end      = 10211, //
    hide_start      = 10221, //隐身buff开始ID
    hide_end        = 10221,
};

BuffManager::BuffManager(PK& me)
    : m_owner(me)
{
}


uint32_t BuffManager::buffSize() const
{
    return m_buffList.size();
}


bool BuffManager::showBuff(TplId buffId)
{
    if(m_owner.isDead())
        return false;
    BuffBase::Ptr dest = buffCT.get(buffId);
    if(nullptr == dest)
    {
        LOG_DEBUG("BUFF, 添加buffId:{}, 找不到配置", buffId);
        return false;
    }

    //检查buff规则
    bool insert = false;
    auto iter = m_buffList.begin();
    for( ; iter != m_buffList.end(); ++iter)
    {
        BuffBase::Ptr& source = iter->second.buffPtr;
        if(source->group == dest->group)
        {
            if(dest->action_type == buff_action::cover 
               && dest->priority >= source->priority)
            {
                unshowBuff(iter->first);
                insert = true;
            }
            else if(dest->action_type == buff_action::not_cover 
                    && iter->first == buffId
                    && dest->time_merge)
            {
                if(iter->second.sec)
                    iter->second.sec += dest->sec;
                else
                    iter->second.endtime += dest->endtime;
            }
            else
            {
                LOG_DEBUG("BUFF, 同组buff不覆盖, group:{}, buffId:{}", dest->group, buffId);
                return false;
            }
            break;
        }
    }

    if(iter == m_buffList.end() || insert)
    {
        BuffElement element;
        element.buffId = buffId;
        element.sec = dest->sec;
        if(dest->endtime)
            element.endtime = toUnixTime(Clock::now()) + dest->endtime;
        element.dur = dest->dur;
        element.buffPtr = dest;
        m_buffList.insert(std::make_pair(buffId, element));

        auto s = m_owner.scene();
        if(s)
        {
            std::vector<uint8_t> buf;
            buf.reserve(32);
            buf.resize(sizeof(PublicRaw::AddBuff) + sizeof(uint32_t));
            auto msg = reinterpret_cast<PublicRaw::AddBuff*>(buf.data());
            msg->id = m_owner.id();
            msg->sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
            msg->buffId[0] = buffId;
            msg->size = 1;
            s->sendCmdTo9(RAWMSG_CODE_PUBLIC(AddBuff), buf.data(), buf.size(), m_owner.pos());

            m_owner.m_pkstate.setStatus(dest->status_id);
        }

        if(!dest->props.empty() || !dest->percent_props.empty())
        {
            calcAttr();
            m_owner.sendMainToMe();
        }
        actionOnHpMp(dest);
    }

    updateToDB(buffId, ModifyType::modify);
    return true;
}


void BuffManager::unshowBuff(TplId buffId)
{
    if(!isShowBuff(buffId))
        return;
    auto s = m_owner.scene();
    if(s)
    {
        PublicRaw::EraseBuff send;
        send.id = m_owner.id();
        send.sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
        send.buffId = buffId;
        s->sendCmdTo9(RAWMSG_CODE_PUBLIC(EraseBuff), &send, sizeof(send), m_owner.pos());
    }

    bool notifyMe = false;
    if(!m_buffList[buffId].buffPtr->props.empty() 
       || !m_buffList[buffId].buffPtr->percent_props.empty())
        notifyMe = true;

    updateToDB(buffId, ModifyType::erase);
    m_owner.m_pkstate.clearStatus(m_buffList[buffId].buffPtr->status_id);
    m_buffList.erase(buffId);
    if(notifyMe)
    {
        calcAttr();
        m_owner.sendMainToMe();
    }
}



bool BuffManager::isShowBuff(TplId buffId) const
{
    return m_buffList.find(buffId) != m_buffList.end();
}


void BuffManager::timerExec(const water::componet::TimePoint& now)
{
    std::vector<uint32_t> dels;
    for(auto& iter : m_buffList)
    {
        BuffElement& element = iter.second;
        if(0 == element.sec && element.endtime <= toUnixTime(now))
        {
            dels.push_back(iter.first);
            continue;
        }

        if(element.buffPtr->interval > 0)
        {
            ++element.tick;
            if(element.tick == element.buffPtr->interval)
            {
                if(iter.first >= SpecialBuffID::poison_start
                   && iter.first <= SpecialBuffID::poison_end)
                {
                    m_owner.computePoisonSkillDamage();
                }
                else
                {
                    actionOnHpMp(element.buffPtr);
                }

                element.tick = 0;
            }
        }

        if(m_owner.isDead())
            break;

        if(element.sec)
            --element.sec;
    }

    for(const auto& delIt : dels)
    {
        unshowBuff(delIt);
    }
}


void BuffManager::calcAttr()
{
    m_constBuffProps.reset();
    m_percentBuffProps.reset();
    for(const auto& iter : m_buffList)
    {
        const BuffBase::Ptr& source = iter.second.buffPtr;
        if(nullptr == source)
            continue;
        m_constBuffProps.addAttribute(source->props);
        m_percentBuffProps.addAttribute(source->percent_props);
    }
}

uint32_t BuffManager::getBuffProps(PropertyType type, int32_t baseVal) const
{
    int32_t value = 0;
    int32_t percent = 0;
    switch(type)
    {
    case PropertyType::hpLv:
        value = m_constBuffProps.GET_ATTRIBUTE(hpLv);
        percent = m_percentBuffProps.GET_ATTRIBUTE(hpLv);
        break;
    case PropertyType::mpLv:
        value = m_constBuffProps.GET_ATTRIBUTE(mpLv);
        percent = m_percentBuffProps.GET_ATTRIBUTE(mpLv);
        break;
    case PropertyType::p_attackMin:
        value = m_constBuffProps.GET_ATTRIBUTE(p_attackMin);
        percent = m_percentBuffProps.GET_ATTRIBUTE(p_attackMin);
        break;
    case PropertyType::p_attackMax:
        value = m_constBuffProps.GET_ATTRIBUTE(p_attackMax);
        percent = m_percentBuffProps.GET_ATTRIBUTE(p_attackMax);
        break;
    case PropertyType::m_attackMin:
        value = m_constBuffProps.GET_ATTRIBUTE(m_attackMin);
        percent = m_percentBuffProps.GET_ATTRIBUTE(m_attackMin);
        break;
    case PropertyType::m_attackMax:
        value = m_constBuffProps.GET_ATTRIBUTE(m_attackMax);
        percent = m_percentBuffProps.GET_ATTRIBUTE(m_attackMax);
        break;
    case PropertyType::witchMin:
        value = m_constBuffProps.GET_ATTRIBUTE(witchMin);
        percent = m_percentBuffProps.GET_ATTRIBUTE(witchMin);
        break;
    case PropertyType::witchMax:
        value = m_constBuffProps.GET_ATTRIBUTE(witchMax);
        percent = m_percentBuffProps.GET_ATTRIBUTE(witchMax);
        break;
    case PropertyType::p_defenceMin:
        value = m_constBuffProps.GET_ATTRIBUTE(p_defenceMin);
        percent = m_percentBuffProps.GET_ATTRIBUTE(p_defenceMin);
        break;
    case PropertyType::p_defenceMax:
        value = m_constBuffProps.GET_ATTRIBUTE(p_defenceMax);
        percent = m_percentBuffProps.GET_ATTRIBUTE(p_defenceMax);
        break;
    case PropertyType::m_defenceMin:
        value = m_constBuffProps.GET_ATTRIBUTE(m_defenceMin);
        percent = m_percentBuffProps.GET_ATTRIBUTE(m_defenceMin);
        break;
    case PropertyType::m_defenceMax:
        value = m_constBuffProps.GET_ATTRIBUTE(m_defenceMax);
        percent = m_percentBuffProps.GET_ATTRIBUTE(m_defenceMax);
        break;
    case PropertyType::lucky:
        value = m_constBuffProps.GET_ATTRIBUTE(lucky);
        percent = m_percentBuffProps.GET_ATTRIBUTE(lucky);
        break;
    case PropertyType::evil:
        value = m_constBuffProps.GET_ATTRIBUTE(evil);
        percent = m_percentBuffProps.GET_ATTRIBUTE(evil);
        break;
    case PropertyType::shot:
        value = m_constBuffProps.GET_ATTRIBUTE(shot);
        percent = m_percentBuffProps.GET_ATTRIBUTE(shot);
        break;
    case PropertyType::shotRatio:
        value = m_constBuffProps.GET_ATTRIBUTE(shotRatio);
        percent = m_percentBuffProps.GET_ATTRIBUTE(shotRatio);
        break;
    case PropertyType::p_escape:
        value = m_constBuffProps.GET_ATTRIBUTE(p_escape);
        percent = m_percentBuffProps.GET_ATTRIBUTE(p_escape);
        break;
    case PropertyType::m_escape:
        value = m_constBuffProps.GET_ATTRIBUTE(m_escape);
        percent = m_percentBuffProps.GET_ATTRIBUTE(m_escape);
        break;
    case PropertyType::escapeRatio:
        value = m_constBuffProps.GET_ATTRIBUTE(escapeRatio);
        percent = m_percentBuffProps.GET_ATTRIBUTE(escapeRatio);
        break;
    case PropertyType::crit:
        value = m_constBuffProps.GET_ATTRIBUTE(crit);
        percent = m_percentBuffProps.GET_ATTRIBUTE(crit);
        break;
    case PropertyType::critRatio:
        value = m_constBuffProps.GET_ATTRIBUTE(critRatio);
        percent = m_percentBuffProps.GET_ATTRIBUTE(critRatio);
        break;
    case PropertyType::antiCrit:
        value = m_constBuffProps.GET_ATTRIBUTE(antiCrit);
        percent = m_percentBuffProps.GET_ATTRIBUTE(antiCrit);
        break;
    case PropertyType::critDamage:
        value = m_constBuffProps.GET_ATTRIBUTE(critDamage);
        percent = m_percentBuffProps.GET_ATTRIBUTE(critDamage);
        break;
    case PropertyType::damageAdd:
        value = m_constBuffProps.GET_ATTRIBUTE(damageAdd);
        percent = m_percentBuffProps.GET_ATTRIBUTE(damageAdd);
        break;
    case PropertyType::damageReduce:
        value = m_constBuffProps.GET_ATTRIBUTE(damageReduce);
        percent = m_percentBuffProps.GET_ATTRIBUTE(damageReduce);
        break;
    case PropertyType::damageAddLv:
        value = m_constBuffProps.GET_ATTRIBUTE(damageAddLv);
        percent = m_percentBuffProps.GET_ATTRIBUTE(damageAddLv);
        break;
    case PropertyType::damageReduceLv:
        value = m_constBuffProps.GET_ATTRIBUTE(damageReduceLv);
        percent = m_percentBuffProps.GET_ATTRIBUTE(damageReduceLv);
        break;
    case PropertyType::antiDropEquip:
        value = m_constBuffProps.GET_ATTRIBUTE(antiDropEquip);
        percent = m_percentBuffProps.GET_ATTRIBUTE(antiDropEquip);
    default:
        break;
    }

    return (1000+percent)*baseVal/1000 + value > 0 ? (1000+percent)*baseVal/1000 + value : 0;
}


uint32_t BuffManager::getHpMpBuffConstProp(PropertyType type, int32_t baseVal) const
{
    int32_t value = 0;
    switch(type)
    {
    case PropertyType::maxhp:
        value = m_constBuffProps.GET_ATTRIBUTE(maxhp);
        break;
    case PropertyType::maxmp:
        value = m_constBuffProps.GET_ATTRIBUTE(maxmp);
        break;
    default:
        break;
    }

    value += baseVal;
    return value < 0 ? 0 : value;
}


uint32_t BuffManager::getHpMpBuffPercentProp(PropertyType type, int32_t baseVal) const
{
    int32_t percent = 0;
    switch(type)
    {
    case PropertyType::addhpRatio:
        percent = m_percentBuffProps.GET_ATTRIBUTE(addhpRatio);
        break;
    case PropertyType::addmpRatio:
        percent = m_percentBuffProps.GET_ATTRIBUTE(addmpRatio);
        break;
    default:
        break;
    }

    percent = baseVal * (1000 + percent) / 1000;
    return percent < 0 ? 0 : percent;
}


uint16_t BuffManager::roleExpAddPercent() const
{
    uint16_t ret = 0;
    for(const auto& iter : m_buffList)
    {
        const BuffBase::Ptr& source = iter.second.buffPtr;
        if(nullptr == source)
            continue;
        ret += source->role_exp_percent;
    }
    return ret;
}


uint16_t BuffManager::heroExpAddPercent() const
{
    uint16_t ret = 0;
    for(const auto& iter : m_buffList)
    {
        const BuffBase::Ptr& source = iter.second.buffPtr;
        if(nullptr == source)
            continue;
        ret += source->hero_exp_percent;
    }
    return ret;
}


void BuffManager::processDeath()
{
    //npc死亡所有buff都清除
    bool clearAll = false;
    if(m_owner.sceneItemType() == SceneItemType::npc
       || m_owner.sceneItemType() == SceneItemType::pet)
        clearAll = true;

    std::vector<uint32_t> dels;
    for(const auto& iter : m_buffList)
    {
        const BuffBase::Ptr& source = iter.second.buffPtr;
        if(clearAll || 0 == source->die_exist)
            dels.push_back(iter.first);
    }

    for(const auto& delIt : dels)
        unshowBuff(delIt);
}


void BuffManager::processOffline()
{
    bool clearAll = false;
    if(m_owner.sceneItemType() == SceneItemType::pet)
        clearAll = true;
    std::vector<uint32_t> dels;
    for(const auto& iter : m_buffList)
    {
        const BuffBase::Ptr& source = iter.second.buffPtr;
        if(clearAll || 0 == source->offline_exist)
            dels.push_back(iter.first);
    }

    updateToDB(dels, ModifyType::erase);
}

void BuffManager::updateToDB(TplId buffId, ModifyType type)
{
    if(m_owner.sceneItemType() == SceneItemType::npc)
        return;
    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PrivateRaw::ModifyBuffData) + sizeof(BuffData));
    auto msg = reinterpret_cast<PrivateRaw::ModifyBuffData*>(buf.data());
    msg->roleId = m_owner.getOwnerId();
    msg->job = m_owner.job();
    msg->sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    msg->ownerSceneItemType = static_cast<uint8_t>(m_owner.getOwnerSceneItemType());
    msg->modifyType = type;
    msg->size = 1;

    const auto& element = m_buffList[buffId];
    msg->data[0].buffId = buffId;
    msg->data[0].sec = element.sec;
    msg->data[0].endtime = element.endtime;
    msg->data[0].dur = element.dur;
    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(ModifyBuffData), buf.data(), buf.size());
}

void BuffManager::updateToDB(std::vector<TplId>& buffIDs, ModifyType type)
{
    if(m_owner.sceneItemType() == SceneItemType::npc)
        return;
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PrivateRaw::ModifyBuffData));
    auto msg = reinterpret_cast<PrivateRaw::ModifyBuffData*>(buf.data());
    msg->roleId = m_owner.getOwnerId();
    msg->job = m_owner.job();
    msg->ownerJob = m_owner.getOwnerJob();
    msg->sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    msg->ownerSceneItemType = static_cast<uint8_t>(m_owner.getOwnerSceneItemType());
    msg->modifyType = type;

    for(const auto& iter : buffIDs)
    {
        buf.resize(buf.size() + sizeof(BuffData));
        auto msg = reinterpret_cast<PrivateRaw::ModifyBuffData*>(buf.data());
        const auto& element = m_buffList[iter];
        msg->data[msg->size].buffId = iter;
        msg->data[msg->size].sec = element.sec;
        msg->data[msg->size].endtime = element.endtime;
        msg->data[msg->size].dur = element.dur;
        ++msg->size;
    }
    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(ModifyBuffData), buf.data(), buf.size());
}

void BuffManager::loadFromDB(std::vector<BuffData>& data)
{
    uint32_t now = toUnixTime(Clock::now());
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PrivateRaw::ModifyBuffData));
    auto msg = reinterpret_cast<PrivateRaw::ModifyBuffData*>(buf.data());
    msg->roleId = m_owner.getOwnerId();
    msg->modifyType = ModifyType::erase;
    for(const auto& iter : data)
    {
        BuffBase::Ptr dest = buffCT.get(iter.buffId);
        if(nullptr == dest || iter.endtime <= now)
        {
            buf.resize(buf.size() + sizeof(BuffData));
            msg = reinterpret_cast<PrivateRaw::ModifyBuffData*>(buf.data());
            msg->data[msg->size].buffId = iter.buffId;
            ++msg->size;
            continue;
        }

        BuffElement ele;
        ele.buffPtr = dest;
        ele.buffId = iter.buffId;
        ele.sec = iter.sec;
        ele.endtime = iter.endtime;
        ele.dur = iter.dur;
        m_buffList.insert(std::make_pair(ele.buffId, ele));
        m_owner.m_pkstate.loadStatus(dest->status_id);
    }

    if(msg->size > 0)
    {
        ProcessIdentity dbcachedId("dbcached", 1);
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(ModifyBuffData), buf.data(), buf.size());
    }
}

void BuffManager::retBuffList(std::shared_ptr<PK> asker)
{
    if(buffSize() > 0)
    {
        std::vector<uint8_t> selectedBuffs;
        selectedBuffs.reserve(128);
        selectedBuffs.resize(sizeof(PublicRaw::RetSelectedBuff));
        auto buffMsg = reinterpret_cast<PublicRaw::RetSelectedBuff*>(selectedBuffs.data());
        buffMsg->id = m_owner.id();
        buffMsg->sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());

        for(const auto& iter : m_buffList)
        {
            selectedBuffs.resize(selectedBuffs.size() + sizeof(uint32_t));
            buffMsg = reinterpret_cast<PublicRaw::RetSelectedBuff*>(selectedBuffs.data());
            *(buffMsg->buffId+buffMsg->size) = iter.first;
            ++buffMsg->size;
        }
        asker->sendToMe(RAWMSG_CODE_PUBLIC(RetSelectedBuff), selectedBuffs.data(), selectedBuffs.size());
        return;
    }

    LOG_DEBUG("BUFF,请求{},{} 时, 发现为空", m_owner.name(), m_owner.id());
}

void BuffManager::retBuffTips(std::shared_ptr<PK> asker, TplId buffId)
{
    if(!isShowBuff(buffId))
    {
        LOG_DEBUG("BUFF, 请求buff={} tips时, 没有该buff", buffId);
        return;
    }

    const BuffElement& ele = m_buffList[buffId];
    PublicRaw::RetSelectedBuffTips ret;
    ret.id = m_owner.id();
    ret.sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    ret.buffId = buffId;
    ret.time = ele.endtime > 0 ? ele.endtime - toUnixTime(Clock::now()) : ele.sec;
    ret.dur = ele.dur;

    asker->sendToMe(RAWMSG_CODE_PUBLIC(RetSelectedBuffTips), &ret, sizeof(ret));
}

void BuffManager::sendBuffListToMe() const
{
    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::AddBuff));
    auto msg = reinterpret_cast<PublicRaw::AddBuff*>(buf.data());
    msg->id = m_owner.id();
    msg->sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    for(const auto& iter : m_buffList)
    {
        buf.resize(buf.size() + sizeof(uint32_t));
        msg = reinterpret_cast<PublicRaw::AddBuff*>(buf.data());
        msg->buffId[msg->size] = iter.first;
        ++msg->size;
    }
    if(msg->size)
        m_owner.sendToMe(RAWMSG_CODE_PUBLIC(AddBuff), buf.data(), buf.size());
}


void BuffManager::actionOnHpMp(BuffBase::Ptr buffPtr)
{
    int32_t changehp = 0;
    changehp += buffPtr->recovery_hp;
    changehp -= buffPtr->damage_hp;
    if(buffPtr->recovery_hp_percent)
        changehp += buffPtr->recovery_hp_percent * m_owner.getMaxHp();
    else if(buffPtr->damage_hp_percent)
        changehp -= buffPtr->damage_hp_percent * m_owner.getMaxHp();
    if(changehp != 0)
    {
        m_owner.changeHpAndNotify(nullptr, changehp, changehp > 0 ? HPChangeType::recovery : HPChangeType::normal);
        return;
    }

    int32_t changemp = 0;
    changemp += buffPtr->recovery_mp;
    changemp -= buffPtr->damage_mp;
    if(buffPtr->recovery_mp_percent)
        changemp += buffPtr->recovery_mp_percent * m_owner.getMaxMp();
    else if(buffPtr->damage_mp_percent)
        changemp -= buffPtr->damage_mp_percent * m_owner.getMaxMp();
    if(changemp != 0)
        m_owner.changeMpAndNotify(changemp);
}

void BuffManager::clearShield()
{
    if(!m_owner.m_pkstate.issetStatus(visual_status::shield))
        return;
    for(TplId buffId = SpecialBuffID::shield_start; buffId <= SpecialBuffID::shield_end; ++buffId)
        unshowBuff(buffId);
}

void BuffManager::unHide()
{
    if(!m_owner.m_pkstate.issetStatus(visual_status::hide))
        return;
    for(TplId buffId = SpecialBuffID::hide_start; buffId <= SpecialBuffID::hide_end; ++buffId)
        unshowBuff(buffId);
}

void BuffManager::addTime(TplId buffId, uint16_t sec)
{
    auto iter = m_buffList.find(buffId);
    if(iter == m_buffList.end())
        return;

    BuffElement& ele = iter->second;
    if(ele.sec)
        ele.sec += sec;
    else
        ele.endtime += sec;

    updateToDB(buffId, ModifyType::modify);
}

}


