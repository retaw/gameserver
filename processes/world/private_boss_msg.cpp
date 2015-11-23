#include "private_boss_msg.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/private_boss.codedef.public.h"
#include "protocol/rawmsg/public/private_boss.h"
#include "protocol/rawmsg/public/field_boss.codedef.public.h"
#include "protocol/rawmsg/public/field_boss.h"


namespace world{

PrivateBossMsg& PrivateBossMsg::me()
{
    static PrivateBossMsg me;
    return me;
}

void PrivateBossMsg::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(PrivateBoss, std::bind(&PrivateBossMsg::clientmsg_PrivateBoss, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(TransFerToPrivateBoss, std::bind(&PrivateBossMsg::clientmsg_TransFerToPrivateBoss, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(LeavePrivateBoss, std::bind(&PrivateBossMsg::clientmsg_LeavePrivateBoss, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqRefreshPrivateBoss, std::bind(&PrivateBossMsg::clientmsg_ReqRefreshPrivateBoss, this, _1, _2, _3));
}

void PrivateBossMsg::clientmsg_PrivateBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    role->m_privateBoss.retPrivateBoss();
}

void PrivateBossMsg::clientmsg_TransFerToPrivateBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto rev = reinterpret_cast<const PublicRaw::TransFerToPrivateBoss*>(msgData);

    role->m_privateBoss.transFerToPrivateBoss(rev->bossId);
}

void PrivateBossMsg::clientmsg_LeavePrivateBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    role->m_privateBoss.leave();
}

void PrivateBossMsg::clientmsg_ReqRefreshPrivateBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    role->m_privateBoss.sendRefreshPrivateBoss();
    role->m_privateBoss.sendRemainSeconds();
}

}
