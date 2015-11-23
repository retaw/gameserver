/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-16 10:28 +0800
 *
 * Modified: 2015-05-06 11:21 +0800
 *
 * Description:  角色管理器
 */

#ifndef PROCESSES_FUNC_ROLE_MANAGER_H
#define PROCESSES_FUNC_ROLE_MANAGER_H


#include "role.h"
#include "water/common/role_container.h"
#include "water/componet/datetime.h"

namespace PrivateRaw { struct SyncOnlineRoleInfoToFunc; }

namespace func{


class RoleManager : public RoleContainer<Role::Ptr>
{
public:
    ~RoleManager() = default;
    void regMsgHandler();
    void regTimer();

    void timerExec(const water::componet::TimePoint& now);
    void deleteofflnInTeamRole(water::componet::TimePoint now);
    std::vector<Role::Ptr> getSimilarMylevelAndUpOnelevel(uint32_t myLevel, uint32_t oneLevel, RoleId roleId);

private:
    RoleManager() = default;
    void trySyncFromSession();
    void updateOnlineRoleInfo(const PrivateRaw::SyncOnlineRoleInfoToFunc& roleData);
    
private:
    void servermsg_RoleOffline(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_SyncOnlineRoleInfoToFunc(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_SessionSyncAllOnlineRoleInfoToFunc(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_UpdateFuncRoleLevel(const uint8_t* msgData, uint32_t msgSize);

private:
    struct roleAndTime
    {
        Role::Ptr role;
        water::componet::TimePoint time;
    };
    bool m_synced = false;
    std::unordered_map<RoleId, roleAndTime> m_offlnInTeamRole;

public:
    static RoleManager& me();
};


}

#endif
