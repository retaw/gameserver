#include "role_manager.h"

#include "gateway.h"
#include "protocol/rawmsg/rawmsg_manager.h"

#include "protocol/rawmsg/private/login.h"
#include "protocol/rawmsg/private/login.codedef.private.h"

#include "protocol/rawmsg/public/login.codedef.public.h"
#include "protocol/rawmsg/public/login.h"

#include "protocol/rawmsg/private/relay.codedef.private.h"
#include "protocol/rawmsg/private/relay.h"

#include "water/componet/logger.h"

namespace gateway{

RoleManager& RoleManager::me()
{
    static RoleManager me;
    return me;
}

void RoleManager::regMsgHandler()
{
    regRoleMsgRelay();
    using namespace std::placeholders;
//    REG_RAWMSG_PRIVATE(RoleOffline, std::bind(&RoleManager::servermsg_RoleOffline, this, _1, _2));
//    REG_RAWMSG_PRIVATE(RelayMsgToClient, std::bind(&RoleManager::servermsg_RelayMsgToClient, this, _1, _2, _3));
//    REG_RAWMSG_PRIVATE(UpdateRoleWorldId, std::bind(&RoleManager::servermsg_UpdateRoleWorldId, this, _1, _2));
}

Role::Ptr RoleManager::newLoginRole(LoginId loginId,
                                    RoleId id, 
                                    const std::string& name, 
                                    const std::string& account)
{
    Role::Ptr role = getById(id);
    if(role != nullptr)
    {//erase old role
        PrivateRaw::RoleOffline offlineMsg;
        offlineMsg.rid     = role->id();
        offlineMsg.loginId = role->loginId();
        offlineMsg.type    = PrivateRaw::OfflineType::replace; //下线

        ProcessIdentity pid("session", 1);
        Gateway::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RoleOffline), &offlineMsg, sizeof(offlineMsg));

        PublicRaw::UnexpectedLogout send;
        send.code = PublicRaw::UnexpectedLogout::Code::repetionLogin;
        role->sendToMe(RAWMSG_CODE_PUBLIC(UnexpectedLogout), &send, sizeof(send));

        LOG_DEBUG("角色被挤下线, role=({}, {}, {})", role->id(), role->name(), role->account());

        //删掉被挤下线的角色
        erase(role);
        Gateway::me().eraseClientConn(role->loginId());
    }

    {//insert new role
        LockGuard lock(m_rolesLock);

        role = Role::create(loginId, id, name, account);
        if(!m_roles.insert(role))
            return nullptr;
        m_loginId2RId[loginId] = id;
    }

    //通知session有角色上线, session将启动进场景流程
    LOG_DEBUG("登录, 通知session有role上线, roleInfo=[ {}, {}, {} ]", id, name, account);
    PrivateRaw::RoleOnline send;
    send.loginId = loginId;
    send.rid     = id;
    ProcessIdentity receiver("session", 1);
    Gateway::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(RoleOnline), &send, sizeof(send));

    return role;
}

Role::Ptr RoleManager::getByLoginId(LoginId loginId) const
{
    LockGuard lock(m_rolesLock);
    auto itOfLoginId2RId = m_loginId2RId.find(loginId);
    if(itOfLoginId2RId == m_loginId2RId.end())
        return nullptr;

    return m_roles.getById(itOfLoginId2RId->second);
}

Role::Ptr RoleManager::getById(RoleId rid) const
{
    LockGuard lock(m_rolesLock);
    return m_roles.getById(rid);
}

void RoleManager::erase(Role::Ptr role)
{
    if(role == nullptr)
        return;

    LockGuard lock(m_rolesLock);
    m_loginId2RId.erase(role->loginId());
    m_roles.erase(role);
}

void RoleManager::onClientDisconnect(LoginId loginId)
{
    LOG_DEBUG("客户端断开连接, loginId={}", loginId);

    Role::Ptr role = getByLoginId(loginId);
    if(role == nullptr)
        return;

    PrivateRaw::RoleOffline offlineMsg;
    offlineMsg.rid     = role->id();
    offlineMsg.loginId = role->loginId();
    offlineMsg.type    = PrivateRaw::OfflineType::logout; //正常下线

    ProcessIdentity pid("session", 1);
    Gateway::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RoleOffline), &offlineMsg, sizeof(offlineMsg));

    LOG_DEBUG("角色下线, role={}", *role);
    erase(role);
}

void RoleManager::timerExec(const water::componet::TimePoint& now)
{
}

void RoleManager::servermsg_RoleOffline(const uint8_t* msgData, uint32_t msgSize)
{
    LOG_DEBUG("role, session的下线消息");
    auto rev = reinterpret_cast<const PrivateRaw::RoleOffline*>(msgData);

    Role::Ptr role = getById(rev->rid);
    if(role == nullptr)
        return;

    //删掉client, 然后会自动触发socket断开的调用
    LOG_DEBUG("服务器踢出角色{}", *role);
    Gateway::me().eraseClientConn(role->loginId());
    return;

}

void RoleManager::servermsg_RelayMsgToClient(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::RelayMsgToClient*>(msgData);
    Role::Ptr role = getById(rev->rid);
    if(role == nullptr)
    {
        LOG_TRACE("转发消息到client, role没找到, pid={}, roleId={}, code={}, size={}", pid, rev->rid, rev->msgCode, rev->msgSize); 
        return;
    }

    role->sendToMe(rev->msgCode, rev->msgData, rev->msgSize);
    LOG_DEBUG("转发消息到client, pid={}, roleId={}, code={}, size={}", pid, rev->rid, rev->msgCode, rev->msgSize); 
}

void RoleManager::servermsg_UpdateRoleWorldId(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateRoleWorldId*>(msgData);

    ProcessIdentity worldId(rev->worldId);

    LOG_DEBUG("更新角色的worldId, roleId={}, worldId={}", rev->rid, worldId);

    Role::Ptr role = getById(rev->rid);
    if(role == nullptr)
    {
        LOG_TRACE("更新角色的worldId, 角色没找到, roleId={}, worldId={}", rev->rid, worldId);
        return;
    }

    role->setWorldId(worldId);
}

void RoleManager::relayRoleMsgToWorld(TcpMsgCode msgCode, const uint8_t* msgData, uint32_t msgSize, LoginId loginId)
{
    Role::Ptr role = getByLoginId(loginId);
    if(role == nullptr)
        return;

    role->relayToWorld(msgCode, msgData, msgSize);
}

void RoleManager::relayRoleMsgToFunc(TcpMsgCode msgCode, const uint8_t* msgData, uint32_t msgSize, LoginId loginId)
{
    Role::Ptr role = getByLoginId(loginId);
    if(role == nullptr)
        return;

    role->relayToFunc(msgCode, msgData, msgSize);
}


}

