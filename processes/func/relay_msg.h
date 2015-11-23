/*
 * Author:
 *
 * Created: 2015-09-25 14:18 +0800
 *
 * Modified: 2015-09-25 14:18 +0800
 *
 * Description:客户端小心通过func然后转到world,同时补充一些需要的func特有的数据,如factionLevel
 */
#ifndef PROCESSES_FUNC_RELAY_MSG_H
#define PROCESSES_FUNC_RELAY_MSG_H

#include "faction_manager.h"

#include "protocol/rawmsg/public/store.h"
#include "protocol/rawmsg/public/store.codedef.public.h"
#include "protocol/rawmsg/private/store.h"
#include "protocol/rawmsg/private/store.codedef.private.h"
#include "protocol/rawmsg/private/relay.h"
#include "protocol/rawmsg/private/relay.codedef.private.h"

#include "protocol/rawmsg/public/faction_active.h"
#include "protocol/rawmsg/public/faction_active.codedef.public.h"
namespace func{

class RelayMsg
{
public:
    ~RelayMsg() = default;

    void regMsgHandler();
    static RelayMsg& me();

private:
    RelayMsg() = default;

private:
    //factionshop中继
    void clientmsg_RequestFactionShop(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RefreshFactionShop(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    //void clientmsg_FactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
};

}

#endif
