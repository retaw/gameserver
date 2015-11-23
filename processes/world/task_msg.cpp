#include "task_msg.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/task.h"
#include "protocol/rawmsg/public/task.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{


TaskMsg& TaskMsg::me()
{
    static TaskMsg me;
    return me;
}

void TaskMsg::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(RequestAcceptTask, std::bind(&TaskMsg::clientmsg_RequestAcceptTask, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(NotifyArriveTaskTargetNpc, std::bind(&TaskMsg::clientmsg_NotifyArriveTaskTargetNpc, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestSubmitTask, std::bind(&TaskMsg::clientmsg_RequestSubmitTask, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestCollect, std::bind(&TaskMsg::clientmsg_RequestCollect, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(FinishCollect, std::bind(&TaskMsg::clientmsg_FinishCollect, this, _1, _2, _3));
}


void TaskMsg::clientmsg_RequestAcceptTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestAcceptTask*>(msgData);
    role->m_roleTask.acceptTask(rev->taskId, rev->npcId);
}

void TaskMsg::clientmsg_NotifyArriveTaskTargetNpc(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::NotifyArriveTaskTargetNpc*>(msgData);
    role->m_roleTask.arriveTargetTaskNpc(rev->npcId);
}

void TaskMsg::clientmsg_RequestSubmitTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestSubmitTask*>(msgData);
    role->m_roleTask.submitTask(rev->taskId, rev->npcId);
}

void TaskMsg::clientmsg_RequestCollect(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestCollect*>(msgData);
    role->startCollect(rev->npcId);
}

void TaskMsg::clientmsg_FinishCollect(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    role->finishCollect();
}

}
