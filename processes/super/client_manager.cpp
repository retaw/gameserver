#include "user_manager.h"

#include "super.h"
#include "scene_manager.h"


#include "protocol/rawmsg/rawmsg_manager.h"

#include "protocol/protobuf/private/login.codedef.h"
#include "protocol/protobuf/public/client.codedef.h"

#include "protocol/rawmsg/private/relay.codedef.private.h"
#include "protocol/rawmsg/private/relay.h"

#include "water/componet/logger.h"

static PublicProto::S_ClientLoginReady temp;

namespace super{


ClientManager& ClientManager::me()
{
    static ClientManager me;
    return me;
}
/*
UserId ClientManager::getUserId()
{
    return ++m_lastUserId;
}
*/
void ClientManager::regMsgHandler()
{
    regUserMsgRelay();
    using namespace std::placeholders;
    REG_PROTO_PRIVATE(UserOffline, std::bind(&ClientManager::servermsg_UserOffline, this, _1));
    REG_PROTO_PRIVATE(RetUserIntoWorld, std::bind(&ClientManager::servermsg_RetUserIntoWorld, this, _1, _2));

    REG_RAWMSG_PRIVATE(RelayMsgToClient, std::bind(&ClientManager::servermsg_RelayMsgToClient, this, _1, _2, _3));
}

User::Ptr ClientManager::newConn(LoginId loginId)
{
    {//create and insert new user
        UserId id = loginId;
        LockGuard lock(m_usersLock);

        auto user = User::create(id, loginId, account);
        if(!m_users.insert(user))
        {
            LOG_ERROR("登录, 新连接, 加入userMananger失败, {}", *user);
            kickClientByLoginId(loginId);
            return nullptr;
        }
        m_loginId2Id[loginId] = id;
        LOG_TRACE("登录, 角色上线{}", *user);
    }

    return nullptr;
}

void ClientManager::clientConnReady(LoginId loginId)
{
    User::Ptr user = getByLoginId(loginId);
    if(user == nullptr)
    {
        LOG_ERROR("登录, clientConnReady, loginId={}没找到", loginId);
        return;
    }

    //login_step 2, super建角并通知scene进场景
    auto sceneInfo = SceneManager::me().pickUpScene();
    if(sceneInfo == nullptr)
    {
        LOG_ERROR("登录, 挑选场景失败, loginId={}", loginId);
        return;
    }
    sceneInfo->playerSize += 1;
    user->setSceneId(sceneInfo->sceneId);

    PrivateProto::UserIntoWorld send;
    send.set_rid(user->id());
    send.set_account(user->account());
    send.set_scene_id(sceneInfo->sceneId);
    Super::me().sendToPrivate(sceneInfo->worldId, PROTO_CODE_PRIVATE(UserIntoWorld), send);
    LOG_DEBUG("登录, 发送角色{}信息到{}, sceneId={}", *user, sceneInfo->worldId, sceneInfo->sceneId);
}

User::Ptr ClientManager::getById(UserId id) const
{
    LockGuard lock(m_usersLock);
    return m_users.getById(id);
}

User::Ptr ClientManager::getByLoginId(LoginId loginId) const
{
    auto it = m_loginId2Id.find(loginId);
    if(it == m_loginId2Id.end())
        return nullptr;

    return getById(it->second);
}

void ClientManager::erase(User::Ptr user)
{
    if(user == nullptr)
        return;

    LockGuard lock(m_usersLock);
    m_users.erase(user);
    m_loginId2Id.erase(user->loginId());
}

void ClientManager::onClientDisconnect(LoginId loginId)
{
    {
        LockGuard lock(m_usersLock);
        m_offlineUsers.push_back(loginId);
    }
    LOG_DEBUG("客户端断开连接, loginId={}", loginId);
}

void ClientManager::kickClientByLoginId(LoginId loginId)
{
    Super::me().eraseClientConn(loginId);
}

void ClientManager::timerExec(const water::componet::TimePoint& now)
{
    std::list<LoginId> offlineUsers;
    {
        LockGuard lock(m_usersLock);
        offlineUsers.swap(m_offlineUsers);
    }

    for(LoginId loginId : offlineUsers)
    {
        User::Ptr user = getByLoginId(loginId);
        if(user == nullptr)
            return;

        PrivateProto::UserOffline offlineMsg;
        offlineMsg.set_rid(user->id());
        offlineMsg.set_reason(0);

        ProcessIdentity pid("world", 1);
        Super::me().sendToPrivate(pid, PROTO_CODE_PRIVATE(UserOffline), offlineMsg);

        {
            auto sceneInfo = SceneManager::me().getById(user->sceneId());
            if(sceneInfo != nullptr && sceneInfo->playerSize > 0)
                sceneInfo->playerSize -= 1;
        }

        LOG_DEBUG("角色下线, user={}", *user);
        erase(user);
    }
}

void ClientManager::servermsg_RetUserIntoWorld(const ProtoMsgPtr& proto, ProcessIdentity pid)
{
    auto rev = std::static_pointer_cast<PrivateProto::RetUserIntoWorld>(proto);

    User::Ptr user = getById(rev->rid());
    if(user == nullptr)
        return;

    user->setWorldId(pid);

    PublicProto::S_ClientLoginReady send;
    send.set_uid(user->id());
    user->sendToMe(PROTO_CODE_PUBLIC(S_ClientLoginReady), send);

    LOG_TRACE("登录, 发送角色id给端, {}", *user);
}

void ClientManager::servermsg_UserOffline(const ProtoMsgPtr& proto)
{
    auto rev = std::static_pointer_cast<PrivateProto::UserOffline>(proto);

    User::Ptr user = getById(rev->rid());
    if(user == nullptr)
        return;

    //删掉client, 然后会自动触发socket断开的调用
    kickClientByLoginId(user->loginId());

    LOG_DEBUG("服务器踢出角色{}", *user);
    return;
}

void ClientManager::servermsg_RelayMsgToClient(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::RelayMsgToClient*>(msgData);
    User::Ptr user = getById(rev->rid);
    if(user == nullptr)
    {
        LOG_TRACE("转发消息到client, user没找到, pid={}, userId={}, code={}, size={}", pid, rev->rid, rev->msgCode, rev->msgSize); 
        return;
    }

    user->sendToMe(rev->msgCode, rev->msgData, rev->msgSize);
//    LOG_DEBUG("转发消息到client, pid={}, userId={}, code={}, size={}", pid, rev->rid, rev->msgCode, rev->msgSize); 
}

void ClientManager::relayUserMsgToWorldProto(TcpMsgCode msgCode, const ProtoMsgPtr& proto, LoginId loginId)
{
    User::Ptr user = getByLoginId(loginId);
    if(user == nullptr)
        return;

    user->relayToWorld(msgCode, *proto);
}


void ClientManager::relayUserMsgToWorld(TcpMsgCode msgCode, const uint8_t* msgData, uint32_t msgSize, LoginId loginId)
{
    User::Ptr user = getByLoginId(loginId);
    if(user == nullptr)
        return;

    user->relayToWorld(msgCode, msgData, msgSize);
}


}

