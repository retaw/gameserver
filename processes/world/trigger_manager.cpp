#include "trigger_manager.h"
#include "role_manager.h"
#include "world.h"
#include "scene.h"
#include "world_boss.h"
#include "bonfire_manager.h"

#include "protocol/rawmsg/public/trigger_scene.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

TriggerManager::TriggerManager()
:m_lastTriggerId(0)
{
}

TriggerManager& TriggerManager::me()
{
    static TriggerManager me;
    return me;
}

void TriggerManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(TouchTrigger, std::bind(&TriggerManager::clientmsg_TouchTrigger, this, _1, _2, _3));
}

void TriggerManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::milliseconds(500), 
                         std::bind(&TriggerManager::timerLoop, this));
}

void TriggerManager::timerLoop()
{
    for(auto it = m_triggers.begin(); it != m_triggers.end(); )
    {
        Trigger::Ptr trigger = it->second;
        if(nullptr == trigger)
        {
            m_triggers.erase(it++);
            continue;
        }

        if(trigger->needErase())
        {
            auto s = trigger->scene();
            if(nullptr != s)
                s->eraseTrigger(trigger);
            m_triggers.erase(it++);
            continue;
        }

        if(0 == trigger->leftTime())
        {
            trigger->markErase();
            switch(trigger->triggerType())
            {
            case TriggerType::box:
                WorldBoss::me().boxBelongRole(nullptr);
                break;
			case TriggerType::bonfire:
				BonfireManager::me().bonfireLeaveScene(trigger->id());
				break;
            default:
                break;
            }
            continue;
        }
        ++it;
    }
}

TriggerId TriggerManager::allocId()
{
    return ++m_lastTriggerId;
}

bool TriggerManager::insert(Trigger::Ptr trigger)
{
    if(nullptr == trigger)
        return false;

    return m_triggers.insert({trigger->id(), trigger}).second;
}

void TriggerManager::erase(Trigger::Ptr trigger)
{
    if(nullptr == trigger)
        return;

    m_triggers.erase(trigger->id());
}

Trigger::Ptr TriggerManager::getById(TriggerId id) const
{
    auto iter = m_triggers.find(id);
    if(iter == m_triggers.end())
        return nullptr;

    return iter->second;
}

void TriggerManager::touchTrigger(RoleId roleId, TriggerId id)
{
    auto trigger = getById(id);
    if(nullptr == trigger)
        return;

    switch(trigger->triggerType())
    {
    case TriggerType::door:
        {
            auto role = RoleManager::me().getById(roleId);
            if(nullptr == role)
                return;
            role->transferByTriggerDoor(trigger->triggerTplId());
        }
        break;
    case TriggerType::box:
        {
            auto role = RoleManager::me().getById(roleId);
            if(nullptr == role)
                return;
            if(trigger->pos() != role->pos())
                return;
            if(trigger->needErase())
                return;
            trigger->markErase();
            role->pickupWorldBossBox();
        }
        break;
    default:
        break;
    }
}

void TriggerManager::clientmsg_TouchTrigger(const uint8_t* msgData, uint32_t size, uint64_t rid)
{
    auto rev = reinterpret_cast<const PublicRaw::TouchTrigger*>(msgData);
    touchTrigger(rid, rev->id);
}

}
