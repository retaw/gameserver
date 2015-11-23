#include "role.h"
#include "session.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace session{


Role::Role(RoleId id, const std::string& name, const std::string& account, uint32_t level, Job job)
: m_id(id), m_name(name), m_account(account), m_level(level), m_job(job)
{
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

uint32_t Role::level()
{
    return m_level;
}

void Role::setLevel(uint32_t level)
{
    m_level = level;
}

Job Role::job() const
{
    return m_job;
}

std::string Role::toString() const
{
    return water::componet::format("[{}, {}, {}]", m_id, m_name, m_account);
}

ProcessIdentity Role::worldId() const
{
    return m_worldId;
}

void Role::setWorldId(ProcessIdentity pid)
{
    m_worldId = pid;
}

ProcessIdentity Role::gatewayId() const
{
    return m_gatewayId;
}

void Role::setGatewayId(ProcessIdentity pid)
{
    m_gatewayId = pid;
}

bool Role::sendToMe(TcpMsgCode msgCode) const
{
    return false;
}

bool Role::sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
    return false;
}

bool Role::sendToWorld(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
    return Session::me().sendToPrivate(worldId(), msgCode, msg, msgSize);
}

SceneId Role::sceneId() const
{
    return m_sceneId;
}

void Role::setSceneId(SceneId sceneId)
{
    m_sceneId = sceneId;
}

void Role::fillFuncSyncData(PrivateRaw::SyncOnlineRoleInfoToFunc* data)
{
    data->id = id();

    name().copy(data->name, NAME_BUFF_SZIE);
    data->name[name().size()] = 0;

    account().copy(data->account, ACCOUNT_BUFF_SZIE);
    data->account[name().size()] = 0;

    data->gatewayId = gatewayId().value();

    data->worldId = worldId().value();
    data->level = level();
    data->job = job();
	data->sceneId = sceneId();
}

void Role::gotoOtherScene(SceneId newSceneId, Coord2D newPos)
{
    PrivateRaw::SessionFuncRoleReqChangeScene send;
    send.rid = id();
    send.newSceneId = newSceneId;
    send.newPosx = newPos.x;
    send.newPosy = newPos.y;

    sendToWorld(RAWMSG_CODE_PRIVATE(SessionFuncRoleReqChangeScene), &send, sizeof(send));
}

}
