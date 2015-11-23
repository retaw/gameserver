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
#include "common/roledef.h"
#include "common/role_container.h"
#include "water/process/process_id.h"
#include "water/process/tcp_message.h"
#include "water/net/packet_connection.h"
#include "water/componet/datetime.h"
#include "water/componet/spinlock.h"


namespace gateway{

using water::process::ProcessIdentity;
using water::process::TcpMsgCode;

class Role
{
public:
    TYPEDEF_PTR(Role)
    CREATE_FUN_NEW(Role);
    ~Role() = default;
    LoginId loginId() const;
    RoleId id() const;

    const std::string& name() const;
    const std::string& account() const;

//    water::net::PacketConnection::Ptr clientConn() const;

    std::string toString() const;

    ProcessIdentity worldId() const;;
    void setWorldId(ProcessIdentity pid);

    bool sendToMe(TcpMsgCode code, const void* msg, uint32_t msgSize) const;

    bool relayToWorld(TcpMsgCode code, const void* msg, uint32_t msgSize) const;
    bool relayToFunc(TcpMsgCode code, const void* msg, uint32_t msgSize) const;

private:
    Role(LoginId loginId, RoleId id, const std::string& name, const std::string& account);

private:
    LoginId m_loginId;
    RoleId m_id;
    const std::string m_name;
    const std::string m_account;

    ProcessIdentity m_worldId;
};


}

#endif
