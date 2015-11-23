/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-25 13:54 +0800
 *
 * Modified: 2015-04-16 10:13 +0800
 *
 * Description: session上需要知道的角色信息
 */

#ifndef PROCESSES_SESSION_ROLE_H
#define PROCESSES_SESSION_ROLE_H

#include "water/common/commdef.h"
#include "water/common/scenedef.h"
#include "water/common/role_container.h"
#include "water/process/process_id.h"
#include "water/process/tcp_message.h"
#include "water/net/packet_connection.h"
#include "water/componet/datetime.h"
#include "water/componet/coord.h"


#include <atomic>
#include <list>
#include <mutex>

namespace PrivateRaw{ struct SyncOnlineRoleInfoToFunc; }

namespace session{


using water::process::ProcessIdentity;
using water::process::TcpMsgCode;
using water::componet::Coord2D;

class Role
{
public:
    TYPEDEF_PTR(Role)
    CREATE_FUN_MAKE(Role)

    Role(RoleId id, const std::string& name, const std::string& account, uint32_t level, Job job);
    ~Role() = default;

    RoleId id() const;
    const std::string& name() const;
    const std::string& account() const;
    uint32_t level();
    void setLevel(uint32_t level);

    Job job() const;

    std::string toString() const;

    ProcessIdentity worldId() const;
    void setWorldId(ProcessIdentity pid);

    ProcessIdentity gatewayId() const;
    void setGatewayId(ProcessIdentity pid);

    SceneId sceneId() const;
    void setSceneId(SceneId sceneId);

    bool sendToMe(TcpMsgCode msgCode) const;
    bool sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const;
    bool sendToWorld(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const;

    void fillFuncSyncData(PrivateRaw::SyncOnlineRoleInfoToFunc* data);

    //切换地图
    void gotoOtherScene(SceneId newSceneId, Coord2D newPos);

private:
    const RoleId m_id;
    const std::string m_name;
    const std::string m_account;
    uint32_t m_level;
    Job m_job;
    
    ProcessIdentity m_gatewayId;
    ProcessIdentity m_worldId;

    SceneId m_sceneId;
};


}

#endif
