#ifndef PROCESS_WORLD_TRIGGER_MANAGER_H
#define PROCESS_WORLD_TRIGGER_MANAGER_H

#include "trigger.h"
#include "water/common/roledef.h"
#include <unordered_map>

namespace world{

using water::componet::TimePoint;

class TriggerManager
{
private:
    TriggerManager();

public:
    ~TriggerManager() = default;

public:
    static TriggerManager& me();

    void regMsgHandler();
    void regTimer();
    void timerLoop();

    TriggerId allocId();
    bool insert(Trigger::Ptr trigger);
    void erase(Trigger::Ptr trigger);
    Trigger::Ptr getById(TriggerId id) const;

    //机关触发
    void touchTrigger(RoleId roleId, TriggerId id);

private:
    void clientmsg_TouchTrigger(const uint8_t* msgData, uint32_t size, uint64_t rid);

private:
    std::unordered_map<TriggerId, Trigger::Ptr> m_triggers;
    TriggerId   m_lastTriggerId;
};

}

#endif
