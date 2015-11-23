/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-14 14:19 +0800
 *
 * Modified: 2015-08-17 11:13 +0800
 *
 * Description:  account管理器, 并处理account相关的业务逻辑
 */

#include "user.h"
#include "water/process/tcp_message.h"
#include "protocol/protobuf/proto_manager.h"

#include <atomic>
#include <list>
#include <mutex>

namespace super{

using water::process::TcpMsgCode;
using protocol::protobuf::ProtoMsgPtr;

class ClientManager
{
public:
    ~ClientManager() = default;

    void clientConnReady(LoginId loginId);

    void onClientDisconnect(LoginId loginId);
    void kickClientByLoginId(LoginId loginId);

    void timerExec(const water::componet::TimePoint& now);

    User::Ptr getById(UserId id) const;
    User::Ptr getByLoginId(LoginId loginId) const;

    void regMsgHandler();

private:
    ClientManager() = default;

    void erase(User::Ptr user);

    //处理外部消息
    void clientmsg_Account

    //处理内部的消息
    void servermsg_RetUserIntoWorld(const ProtoMsgPtr& proto, ProcessIdentity pid);
    void servermsg_UserOffline(const ProtoMsgPtr& proto);
    void servermsg_RelayMsgToClient(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);

    void regUserMsgRelay();

    void relayUserMsgToWorld(TcpMsgCode msgCode, const uint8_t* msgData, uint32_t msgSize, LoginId loginId);
    void relayUserMsgToWorldProto(TcpMsgCode msgCode, const ProtoMsgPtr& proto, LoginId loginId);

//    UserId getUserId();

private:
    typedef std::lock_guard<water::componet::Spinlock> LockGuard;
    mutable water::componet::Spinlock m_usersLock;

    std::unordered_map<LoginId, User::

    std::list<LoginId> m_offlineUsers;

    std::unordered_map<LoginId, UserId> m_loginId2Id;

//    UserId m_lastUserId = MINI_ROLE_ID;

public:
    static ClientManager& me();
};


}

