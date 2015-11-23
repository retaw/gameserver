#include "hero_msg_manager.h"
#include "role_manager.h"
#include "scene.h"

#include "protocol/rawmsg/public/hero_scene.h"
#include "protocol/rawmsg/public/hero_scene.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

HeroMsgManager HeroMsgManager::m_me;

HeroMsgManager& HeroMsgManager::me()
{
    return m_me;
}


void HeroMsgManager::regMsgHandler()
{
    REG_RAWMSG_PUBLIC(RequestCreatedHeroList, std::bind(&HeroMsgManager::clientmsg_RequestCreatedHeroList, this, _1, _2, _3));

    REG_RAWMSG_PUBLIC(RequestCreateHero, std::bind(&HeroMsgManager::clientmsg_RequestCreateHero, this, _1, _2, _3));

    REG_RAWMSG_PUBLIC(RequestSummonHero, std::bind(&HeroMsgManager::clientmsg_RequestSummonHero, this, _1, _2, _3));

    REG_RAWMSG_PUBLIC(RequestRecallHero, std::bind(&HeroMsgManager::clientmsg_RequestRecallHero, this, _1, _2, _3));

    REG_RAWMSG_PUBLIC(RequestHeroChangePos, std::bind(&HeroMsgManager::clientmsg_RequestHeroChangePos, this, _1, _2, _3));

    REG_RAWMSG_PUBLIC(RequestSetDefaultCallHero, std::bind(&HeroMsgManager::clientmsg_RequestSetDefaultCallHero, this, _1, _2, _3));

    REG_RAWMSG_PUBLIC(HeroAIMode, std::bind(&HeroMsgManager::clientmsg_HeroAIMode, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(HeroLockOnTarget, std::bind(&HeroMsgManager::clientmsg_HeroLockOnTarget, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RoleLockOnTarget, std::bind(&HeroMsgManager::clientmsg_RoleLockOnTarget, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(HeroDisableSkillList, std::bind(&HeroMsgManager::clientmsg_HeroDisableSkillList, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RetHeroSerializeData, std::bind(&HeroMsgManager::servermsg_RetHeroSerializeData, this, _1, _2));
}

//请求已创建英雄列表
void HeroMsgManager::clientmsg_RequestCreatedHeroList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestCreatedHeroList*>(msgData);
    if(!rev)
        return;

    role->m_heroManager.sendCreatedHeroList();
    return;
}

//请求创建英雄
void HeroMsgManager::clientmsg_RequestCreateHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestCreateHero*>(msgData);
    if(!rev)
        return;

    role->m_heroManager.requestCreateHero(rev->job, rev->sex);
    return;
}

//请求召唤英雄
void HeroMsgManager::clientmsg_RequestSummonHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestSummonHero*>(msgData);
    if(!rev)
        return;

    role->m_heroManager.requestSummonHero(rev->job);
    return;
}

//请求召回英雄
void HeroMsgManager::clientmsg_RequestRecallHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role =   RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    /*
       auto rev = reinterpret_cast<const PublicRaw::RequestRecallHero*>(msgData);
       if(!rev)
       return;
       */

    role->m_heroManager.requestRecallHero();
    return;
}

//请求移动英雄
void HeroMsgManager::clientmsg_RequestHeroChangePos(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    return;
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestHeroChangePos*>(msgData);
    if(!rev)
        return;

    role->m_heroManager.requestHeroChangePos(rev->job, rev->posX, rev->posY, static_cast<componet::Direction>(rev->dir), rev->type);
    return;
}

//请求设置默认召唤英雄
void HeroMsgManager::clientmsg_RequestSetDefaultCallHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestSetDefaultCallHero*>(msgData);
    if(!rev)
        return;

    role->m_heroManager.requestSetDefaultCallHero(rev->job);
    return;
}

void HeroMsgManager::servermsg_RetHeroSerializeData(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RetHeroSerializeData*>(msgData);
    Role::Ptr role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;

    role->m_heroManager.retHeroSerializeData(rev);
    return;
}

void HeroMsgManager::clientmsg_HeroAIMode(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::HeroAIMode*>(msgData);
    auto hero = role->m_heroManager.getDefaultHero();
    if(hero == nullptr)
        return;

    hero->setAIMode(static_cast<Hero::AIMode>(rev->aiMode));
    role->sendToMe(RAWMSG_CODE_PUBLIC(HeroAIMode), msgData, msgSize);

    LOG_DEBUG("设置AI模式, mode={}", rev->aiMode);
}

void HeroMsgManager::clientmsg_HeroLockOnTarget(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto scene = role->scene();
    if(scene == nullptr)
        return;

    auto hero = role->m_heroManager.getDefaultHero();
    if(hero == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::HeroLockOnTarget*>(msgData);
    SceneItemType type = static_cast<SceneItemType>(rev->type);
    PK::Ptr target = scene->getPKByIdAndType(rev->id, type);
    hero->lockOnTarget(target);

    LOG_DEBUG("锁定hero目标, type={}, id={}", rev->type, rev->id);
#ifdef WATER_DEBUG_HEROAI
    role->sendSysChat(ChannelType::screen_right_down, "锁定hero目标, type={}, id={}", type, rev->id);
#endif
}

void HeroMsgManager::clientmsg_RoleLockOnTarget(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto scene = role->scene();
    if(scene == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RoleLockOnTarget*>(msgData);
    SceneItemType type = static_cast<SceneItemType>(rev->type);
    PK::Ptr target = scene->getPKByIdAndType(rev->id, type);
    role->lockOnTarget(target);
    LOG_DEBUG("锁定role目标, type={}, id={}", type, rev->id);
#ifdef WATER_DEBUG_HEROAI
    role->sendSysChat(ChannelType::screen_right_down, "锁定role目标, type={}, id={}", type, rev->id);
#endif
}

void HeroMsgManager::clientmsg_HeroDisableSkillList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto hero = role->m_heroManager.getDefaultHero();
    if(hero == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::HeroDisableSkillList*>(msgData);
    if(rev->binSize() > msgSize)
        return;

    auto job = hero->job();
    auto& setting = role->m_roleSundry.m_heroData.heroSkillSetting;
    setting.clear();
    for(auto i = 0; i < rev->size; ++i)
        setting[job].insert(rev->skillList[i]);
    LOG_DEBUG("设置hero禁用技能列表, job={}, bannedSize={}, disableSkills={}", job, rev->size);//, setting[job]);
}


}
