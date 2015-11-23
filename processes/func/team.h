#ifndef PROCESSES_FUNC_TEAM_H
#define PROCESSES_FUNC_TEAM_H

#include "water/common/commdef.h"
#include "water/common/frienddef.h"
#include "role.h"

#include <unordered_set>

namespace func{

class Team
{
const uint16_t MAXMEMNUM = 15;
public:
    TYPEDEF_PTR(Team);
    CREATE_FUN_MAKE(Team);
    Team(Role::Ptr captain, TeamId teamid);
    ~Team() = default;

    std::vector<Role::Ptr> membersVec() const;
    Role::Ptr captain() const;
    const uint16_t num() const;
    const TeamId teamId() const;
    bool setCaptain(Role::Ptr captain);
    bool autoChangeCaptain(RoleId& captainId);

    bool insertMember(Role::Ptr role);
    bool eraseMember(Role::Ptr role);

    bool beFull() const;

    
    uint16_t fillMembersInfo(std::vector<uint8_t> &buf) const;
    std::unordered_map<RoleId, Role::Ptr> getMemberMap() const;

    void timerExec();
    void updateTeamInfoToWorld(Role::Ptr role, bool insertOrErase) const;    //入队或者退队给world更新队伍消息
    void updateBreakTeamInfoToWorld() const;
    void renewRoleOnlnInteam(Role::Ptr role);
private:

private:
    Role::Ptr m_captain;    //队长，下线后踢出队伍，所以可以存储Role::Ptr
    TeamId m_teamId = 0;
    std::unordered_map<RoleId, Role::Ptr> m_member; //组内所以成员
    //std::unordered_set<RoleId> inviteRecord;    //记录发出的邀请
};

}

#endif

