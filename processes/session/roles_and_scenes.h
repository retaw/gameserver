/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-16 16:30 +0800
 *
 * Modified: 2015-04-16 16:30 +0800
 *
 * Description:  场景和角色的关系
 */

#ifndef SESSION_ROLE_AND_SCENES_H
#define SESSION_ROLE_AND_SCENES_H

#include "water/process/process_id.h"
#include "water/common/roledef.h"


#include <set>

namespace session{

using water::process::ProcessIdentity;

class RolesAndScenes
{
public:
    void regMsgHandler();
    void loadConfig(const std::string& cfgDir);

private:
    void servermsg_RoleOnline(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);
    void servermsg_RetRoleData(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_RetRoleIntoScene(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);
    void servermsg_RoleOffline(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);
    void servermsg_RoleChangeScene(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_FuncQuestSyncAllOnlineRoleInfo();
    void servermsg_UpdateSessionRoleLevel(const uint8_t* msgData, uint32_t msgSize);

    void servermsg_RoleGotoTargetRoleScene(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_WorldRetTargetRoleScenePos(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_WorldCatchRole(const uint8_t* msgData, uint32_t msgSize);

private:
    std::set<LoginId> m_loginingRoles;
    std::set<RoleId> m_changingSceneRoles;
    uint16_t m_mapId = 0;
    uint16_t m_posx = 0;
    uint16_t m_posy = 0;
    uint8_t m_dir = 0;

public:
    static RolesAndScenes& me();
};


}

#endif

