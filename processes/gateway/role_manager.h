/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-14 14:19 +0800
 *
 * Modified: 2015-04-14 14:19 +0800
 *
 * Description:  role管理器, 并处理role相关的业务逻辑
 */

#include "role.h"
#include "water/process/tcp_message.h"

#include <atomic>
#include <list>
#include <mutex>

namespace gateway{

using water::process::TcpMsgCode;

class RoleManager
{
public:
    ~RoleManager() = default;
    Role::Ptr newLoginRole(LoginId loginId,
                               RoleId id, 
                               const std::string& name, 
                               const std::string& account);

    void onClientDisconnect(LoginId loginId);
    void timerExec(const water::componet::TimePoint& now);

    Role::Ptr getByLoginId(LoginId loginId) const;
    Role::Ptr getById(LoginId loginId) const;
    void erase(Role::Ptr role);

    void regMsgHandler();

private:
    RoleManager() = default;
    //处理下线的消息
    void servermsg_RoleOffline(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_RelayMsgToClient(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);
    void servermsg_UpdateRoleWorldId(const uint8_t* msgData, uint32_t msgSize);

    void regRoleMsgRelay();

    void relayRoleMsgToWorld(TcpMsgCode msgCode, const uint8_t* msgData, uint32_t msgSize, LoginId loginId);
    void relayRoleMsgToFunc(TcpMsgCode msgCode, const uint8_t* msgData, uint32_t msgSize, LoginId loginId);


private:
    RoleId m_lastTempRoleId;

    typedef std::lock_guard<water::componet::Spinlock> LockGuard;
    mutable water::componet::Spinlock m_rolesLock;
    std::unordered_map<LoginId, RoleId> m_loginId2RId;
    RoleContainer<Role::Ptr> m_roles;

public:
    static RoleManager& me();
};


}
