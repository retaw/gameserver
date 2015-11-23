#include "relay_msg.h"
#include "water/componet/logger.h"
#include "func.h"

namespace func{

RelayMsg& RelayMsg::me()
{
    static RelayMsg me;
    return me;
}

void RelayMsg::regMsgHandler()
{
    using namespace std::placeholders;
    //faction_shop中继
    REG_RAWMSG_PUBLIC(RequestFactionShop, std::bind(&RelayMsg::clientmsg_RequestFactionShop, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RefreshFactionShop, std::bind(&RelayMsg::clientmsg_RefreshFactionShop, this, _1, _2, _3));
    //REG_RAWMSG_PUBLIC(FactionTask, std::bind(&RelayMsg::clientmsg_FactionTask, this, _1, _2, _3));
}

void RelayMsg::clientmsg_RequestFactionShop(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
        return;

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    PrivateRaw::RequestFactionShop send;
    send.roleId = roleId;
    send.factionLevel = FactionManager::me().getFactionLevel(role->factionId());

    Func::me().sendToPrivate(role->worldId(), RAWMSG_CODE_PRIVATE(RequestFactionShop), (uint8_t*)&send, sizeof(send));
    LOG_DEBUG("中继转发给world, RequestFactionShop");
}

void RelayMsg::clientmsg_RefreshFactionShop(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
        return;

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    PrivateRaw::RefreshFactionShop send;
    send.roleId = roleId;
    send.factionLevel = FactionManager::me().getFactionLevel(role->factionId());

    Func::me().sendToPrivate(role->worldId(), RAWMSG_CODE_PRIVATE(RefreshFactionShop), (uint8_t*)&send, sizeof(send));
    LOG_DEBUG("中继转发给world, RefreshFactionShop");
}
/*
void RelayMsg::clientmsg_FactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
        return;

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    PrivateRaw::FactionTask send;
    send.roleId = roleId;
    send.factionLevel = FactionManager::me().getFactionLevel(role->factionId());

    Func::me().sendToPrivate(role->worldId(), RAWMSG_CODE_PRIVATE(FactionTask), (uint8_t*)&send, sizeof(send));
    LOG_DEBUG("中继转发给world, RefreshFactionShop");
}
*/
}
