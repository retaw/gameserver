#include "stall_msg.h"
#include "role_manager.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/stall_log.h"
#include "protocol/rawmsg/private/stall_log.codedef.private.h"

#include "protocol/rawmsg/public/stall.h"
#include "protocol/rawmsg/public/stall.codedef.public.h"

namespace world{

StallMsg& StallMsg::me()
{
    static StallMsg me;
    return me;
}

void StallMsg::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(DBRetStallLog, std::bind(&StallMsg::servermsg_DBRetStallLog, this, _1, _2));

    REG_RAWMSG_PUBLIC(ReqStallSellLog, std::bind(&StallMsg::clientmsg_ReqStallSellLog, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqOpenStall, std::bind(&StallMsg::clientmsg_ReqOpenStall, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqCloseStall, std::bind(&StallMsg::clientmsg_ReqCloseStall, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqBuyStallObj, std::bind(&StallMsg::clientmsg_ReqBuyStallObj, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqWatchOthersStall, std::bind(&StallMsg::clientmsg_ReqWatchOthersStall, this, _1, _2, _3));
}

void StallMsg::servermsg_DBRetStallLog(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::DBRetStallLog*>(msgData);
    auto role = RoleManager::me().getById(rev->roleId);
    if(nullptr == role)
        return;

    std::string log("");
    log.append(rev->logs, rev->size);
    role->m_roleStall.loadLog(log);
    role->m_roleStall.retStallLog();
}

void StallMsg::clientmsg_ReqStallSellLog(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    role->m_roleStall.retStallLog();
}

void StallMsg::clientmsg_ReqOpenStall(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::ReqOpenStall*>(msgData);
    if(sizeof(*rev) > size)
    {
        LOG_DEBUG("摆摊, 请求摆摊消息长度非法, recv_size:{}, size:{}", sizeof(*rev), size);
        return;
    }
    role->m_roleStall.openStall(rev);
}

void StallMsg::clientmsg_ReqCloseStall(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    role->m_roleStall.closeStall();
}

void StallMsg::clientmsg_ReqBuyStallObj(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::ReqBuyStallObj*>(msgData);
    role->m_roleStall.buyStallObj(rev->seller, rev->index);
}

void StallMsg::clientmsg_ReqWatchOthersStall(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::ReqWatchOthersStall*>(msgData);
    role->m_roleStall.watchOthersStall(rev->targetRoleId);
}

}

