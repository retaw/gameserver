#ifndef PROCESSES_FUNC_TEAM_MANAGER_H
#define PROCESSES_FUNC_TEAM_MANAGER_H

#include "team.h"
#include "protocol/rawmsg/public/team.h"
namespace func{

class TeamManager
{
public:
    ~TeamManager() = default;

    void regMsgHandler();
    static TeamManager& me();

    void renewRoleOnlnInteam(Role::Ptr role, const TeamId teamId);
    std::vector<Role::Ptr> getTeamMembers(const RoleId roleId);

    void loadConfig(const std::string& cfgDir);
private:
    TeamManager() = default;

    //for reg
    void clientmsg_CreateTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_BreakTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_LeaveTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_KickOutTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ChangeCaptain(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ApplyJoinToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RetApplyJoinToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_InviteJoinToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RetInviteJoinToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_TeamMembers(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_NearbyTeams(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_FormTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
    void createTeam(Role::Ptr captain);
    void breakTeam(Role::Ptr captain);
public:
    void leaveTeam(Role::Ptr role);
private:
    void kickOutTeam(Role::Ptr role, Role::Ptr kickRole);
    void changeCaptain(Role::Ptr role, Role::Ptr newCaptain);
    void automicChangeCaptain(Role::Ptr role);
    void applyJoinToC(Role::Ptr role, const TeamId teamId);
    void retApplyJoinToC(Role::Ptr captain, Role::Ptr role, const bool acpt);
    void inviteJoinToC(Role::Ptr captain, Role::Ptr role);
    void retInviteJoinToC(Role::Ptr role, const TeamId teamId, const bool acpt);
    void retTeamMembers(Role::Ptr role);
    void retNearbyTeams(Role::Ptr role, const PublicRaw::NearbyTeams* rev);
    void dealFormTeam(const RoleId AcRoleId, const RoleId PasRoleId);

    //
    bool isCaptain(Role::Ptr role);
private:
    uint32_t m_teamStartLevel = 0;
    TeamId m_teamId =1000;    //记录最大的teamId,1000开始

    std::unordered_map<TeamId, Team::Ptr> m_teams;

};

}

#endif
