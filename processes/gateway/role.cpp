#include "role.h"
#include "gateway.h"
#include "water/componet/format.h"

namespace gateway{

/****************non-static***************/
Role::Role(LoginId loginId, RoleId id, const std::string& name, const std::string& account)
: m_loginId(loginId), m_id(id), m_name(name), m_account(account)
{
}

LoginId Role::loginId() const
{
    return m_loginId;
}

RoleId Role::id() const
{
    return m_id;
}

const std::string& Role::name() const
{
    return m_name;
}

const std::string& Role::account() const
{
    return m_account;
}

std::string Role::toString() const
{
    return water::componet::format("[{}, {}, {}]", id(), name(), account());
}

ProcessIdentity Role::worldId() const
{
    return m_worldId;
}

void Role::setWorldId(ProcessIdentity pid)
{
    m_worldId = pid;
}

bool Role::sendToMe(TcpMsgCode code, const void* msg, uint32_t msgSize) const
{
    return Gateway::me().sendToClient(m_loginId, code, msg, msgSize);
}

bool Role::relayToWorld(TcpMsgCode code, const void* msg, uint32_t msgSize) const
{
    return Gateway::me().relayToPrivate(m_id, m_worldId, code, msg, msgSize);
}

bool Role::relayToFunc(TcpMsgCode code, const void* msg, uint32_t msgSize) const
{
    return Gateway::me().relayToPrivate(m_id, ProcessIdentity("func", 1), code, msg, msgSize);
}

}//end namespace
