#include "dragon_heart.h"
#include "dragon_heart_cfg.h"
#include "role.h"
#include "object_config.h"
#include "massive_config.h"
#include "world.h"

#include "protocol/rawmsg/public/dragon_heart.h"
#include "protocol/rawmsg/public/dragon_heart.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

DragonHeart::DragonHeart(Role& me)
: m_owner(me)
{
}

void DragonHeart::setDragonPara(const std::vector<std::pair<DragonSkillProp, uint16_t>>& para)
{
    for(const auto& iter : para)
    {
        switch(iter.first)
        {
        case DragonSkillProp::roleSkillPower:
            m_dragonPara.roleSkillPower = iter.second;
            break;
        case DragonSkillProp::roleSkillDamage:
            m_dragonPara.roleSkillDamage = iter.second;
            break;
        case DragonSkillProp::rolePveConstDamage:
            m_dragonPara.rolePveConstDamage = iter.second;
            break;
        case DragonSkillProp::rolepveDamageAdd:
            m_dragonPara.rolepveDamageAdd = iter.second;
            break;
        case DragonSkillProp::rolePvpConstDamage:
            m_dragonPara.rolePvpConstDamage = iter.second;
            break;
        case DragonSkillProp::rolePvpDamageAdd:
            m_dragonPara.rolePvpDamageAdd = iter.second;
            break;
        case DragonSkillProp::heroSkillPower:
            m_dragonPara.heroSkillPower = iter.second;
            break;
        case DragonSkillProp::heroSkillDamage:
            m_dragonPara.heroSkillDamage = iter.second;
            break;
        case DragonSkillProp::heroPveConstDamage:
            m_dragonPara.heroPveConstDamage = iter.second;
            break;
        case DragonSkillProp::heropveDamageAdd:
            m_dragonPara.heropveDamageAdd = iter.second;
            break;
        case DragonSkillProp::heroPvpConstDamage:
            m_dragonPara.heroPvpConstDamage = iter.second;
            break;
        case DragonSkillProp::heroPvpDamageAdd:
            m_dragonPara.heroPvpDamageAdd = iter.second;
            break;
        case DragonSkillProp::reduceEnergeCost:
            m_dragonPara.reduceEnergeCost = iter.second;
            break;
        case DragonSkillProp::addEnergeLimit:
            m_dragonPara.addEnergeLimit = iter.second;
            break;
        case DragonSkillProp::extraAnger:
            m_dragonPara.extraAnger = iter.second;
            break;
        case DragonSkillProp::extendReadyTime:
            m_dragonPara.extendReadyTime = iter.second;
            break;
        default:
            break;
        }
    }
}

uint16_t DragonHeart::roleSkillPower() const
{
    return m_dragonPara.roleSkillPower;
}

uint16_t DragonHeart::roleSkillDamage() const
{
    return m_dragonPara.roleSkillDamage;
}

uint16_t DragonHeart::rolePveConstDamage() const
{
    return m_dragonPara.rolePveConstDamage;
}

uint16_t DragonHeart::rolepveDamageAdd() const
{
    return m_dragonPara.rolepveDamageAdd;
}

uint16_t DragonHeart::rolePvpConstDamage() const
{
    return m_dragonPara.rolePvpConstDamage;
}

uint16_t DragonHeart::rolePvpDamageAdd() const
{
    return m_dragonPara.rolePvpDamageAdd;
}

uint16_t DragonHeart::heroSkillPower() const
{
    return m_dragonPara.heroSkillPower;
}

uint16_t DragonHeart::heroSkillDamage() const
{
    return m_dragonPara.heroSkillDamage;
}

uint16_t DragonHeart::heroPveConstDamage() const
{
    return m_dragonPara.heroPveConstDamage;
}

uint16_t DragonHeart::heropveDamageAdd() const
{
    return m_dragonPara.heropveDamageAdd;
}

uint16_t DragonHeart::heroPvpConstDamage() const
{
    return m_dragonPara.heroPvpConstDamage;
}

uint16_t DragonHeart::heroPvpDamageAdd() const
{
    return m_dragonPara.heroPvpDamageAdd;
}

uint16_t DragonHeart::reduceEnergeCost() const
{
    return m_dragonPara.reduceEnergeCost;
}

uint16_t DragonHeart::addEnergeLimit() const
{
    return m_dragonPara.addEnergeLimit;
}

uint16_t DragonHeart::extraAnger() const
{
    return m_dragonPara.extraAnger;
}

uint16_t DragonHeart::extendReadyTime() const
{
    return m_dragonPara.extendReadyTime;
}

void DragonHeart::retDragonSkillList() const
{
    std::vector<uint8_t> buf;
    buf.reserve(64);
    buf.resize(sizeof(PublicRaw::RetDragonSkills));
    auto msg = reinterpret_cast<PublicRaw::RetDragonSkills*>(buf.data());
    msg->energeLimit = m_owner.energeLimit();
    msg->energe = m_owner.m_roleSundry.m_energe;
    for(const auto& iter : m_owner.m_roleSundry.m_dragonSkills)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetDragonSkills::DragonSkillInfo));
        auto msg = reinterpret_cast<PublicRaw::RetDragonSkills*>(buf.data());
        msg->data[msg->size].id = iter.first;
        msg->data[msg->size].level = iter.second;
        ++msg->size;
    }
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetDragonSkills), buf.data(), buf.size());
}

void DragonHeart::refreshDragonSkill(uint16_t id, uint16_t level) const
{
    PublicRaw::RefreshDragonSkill ret;
    ret.id = id;
    ret.level = level;
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshDragonSkill), &ret, sizeof(ret));
}

bool DragonHeart::checkCondition(uint32_t dragonSoul, uint32_t roleLevel) const
{
    if(m_owner.level() < roleLevel)
    {
        m_owner.sendSysChat("等级不够");
        return false;
    }

    if(m_owner.getMoney(MoneyType::money_10) < dragonSoul)
    {
        m_owner.sendSysChat("龙魂不够");
        return false;
    }

    return true;
}

void DragonHeart::upgradeDragonSkill(uint16_t id)
{
    uint16_t level = 1;
    DragonHeartCfg::Ptr dragonCfg;
    if(m_owner.m_roleSundry.m_dragonSkills.find(id) == m_owner.m_roleSundry.m_dragonSkills.end())
    {
        //初次学习
        dragonCfg = DragonHeartBase::me().getCfg(id, level);
        if(nullptr == dragonCfg)
        {
            LOG_DEBUG("龙心, 学习技能, 找不到配置, id={}, level=1", id);
            return;
        }
    }
    else
    {
        level += m_owner.m_roleSundry.m_dragonSkills[id];
        dragonCfg = DragonHeartBase::me().getCfg(id, level);
        if(nullptr == dragonCfg)
        {
            m_owner.sendSysChat("技能已满级");
            return;
        }
    }

    if(!checkCondition(dragonCfg->costDragonSoul, dragonCfg->roleLevel))
    {
        LOG_DEBUG("龙心, 学习技能, 条件不满足");
        return;
    }

    m_owner.reduceMoney(MoneyType::money_10, dragonCfg->costDragonSoul, "");
    m_owner.m_roleSundry.m_dragonSkills[id] = level;
    refreshDragonSkill(id, level);
    setDragonPara(dragonCfg->dragonSkillProps);
    m_owner.m_roleSundry.saveSundry();
}

}

