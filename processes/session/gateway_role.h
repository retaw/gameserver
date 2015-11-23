/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-25 13:54 +0800
 *
 * Modified: 2015-03-25 13:54 +0800
 *
 * Description: 
 */

#ifndef GATE_ROLE_H
#define GATE_ROLE_H

#include "common/commdef.h"
#include "common/role_container.h"
#include "water/process/process_id.h"
#include "water/net/packet_connection.h"
#include "water/componet/datetime.h"
#include "water/componet/spinlock.h"


#include <atomic>
#include <list>
#include <mutex>

namespace gateway{

using water::process::ProcessIdentity;

class GateRole
{
private:
    friend class GateRoleManager;
    GateRole(LoginId loginId, RoleId id, const std::string& name, const std::string& account);

public:
    TYPEDEF_PTR(GateRole)
    ~GateRole() = default;

    RoleId id() const;
    const std::string& name() const;
    const std::string& account() const;
    LoginId loginId() const;
    water::net::PacketConnection::Ptr clientConn() const;

    ProcessIdentity scenesServerId() const;;
    void setScenesServerId(ProcessIdentity scenesServerId);

    bool sendToMe(const uint8_t* msg, uint32_t msgSize) const;

    bool relayToScene(const uint8_t* msg, uint32_t msgSize) const;


private:
    LoginId m_loginId;

    RoleId m_id;
    const std::string m_name;
    const std::string m_account;
    
    ProcessIdentity m_scenesServerId;
};

/**************************************************/
class GateRoleManager
{
public:
    ~GateRoleManager() = default;
    void onClientDisconnect(LoginId loginId);
    GateRole::Ptr newLoginRole(LoginId loginId,
                               RoleId id, 
                               const std::string& name, 
                               const std::string& account);
    void timerExec(const water::componet::TimePoint& now);

private:
    GateRoleManager() = default;
    GateRole::Ptr getByLoginId(LoginId loginId) const;

private:
    //处理下线的消息
    void servermsg_RetRoleOffline(const uint8_t* msgData, uint32_t msgSize);

private:
    RoleId m_lastTempRoleId;
    std::unordered_map<LoginId, RoleId> m_loginId2RId;
    RoleContainer<GateRole::Ptr> m_roles;

    typedef std::lock_guard<water::componet::Spinlock> LockGuard;
    //除了这个删除列表, 其它的数据都仅在主定时器线程中被访问
    water::componet::Spinlock m_disconnectedLoginIdsLock;
    std::list<LoginId> m_disconnectedLoginIds;

public:
    static GateRoleManager& me();
};

}

#endif
