#ifndef PROCESS_RELATIONS_MANAGER_H
#define PROCESS_RELATIONS_MANAGER_H

#include "protocol/rawmsg/private/team.h"
#include "protocol/rawmsg/private/friend.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "role_manager.h"

namespace world{

class RelationsManager
{
    const float teamMemberAddExpRatio = 0.05;
public:
    ~RelationsManager() = default;
    void regMsgHandler();
    static RelationsManager& me();

    std::vector<RoleId> getMemberVecByTeamId(TeamId teamId);
    float getTeamMeberExtraAddExpRatio(Role::Ptr role);

private:
    RelationsManager() = default;

    //更新组队信息
    void servermsg_UpdateTeamInfo(const uint8_t* msgData, uint32_t msgSize);
    void updateTeamInfo(const PrivateRaw::UpdateTeamInfo* rev);

    //更新帮派信息
    void servermsg_UpdateFacion(const uint8_t* msgData, uint32_t msgSize);
    void updateFaction(const PrivateRaw::UpdateFaction* rev);

    //该同步只是上线的时候会从func收到，类似从dbcached得到任务主数据
    void servermsg_SynBanggong(const uint8_t* msgData, uint32_t msgSize);

    void servermsg_SysnFactionLevel(const uint8_t* msgData, uint32_t msgSize);

    //更新仇人信息
    void servermsg_UpdateWorldEnemy(const uint8_t* msgData, uint32_t msgSize);

    //更新
    void servermsg_UpdateWorldBlack(const uint8_t* msgData, uint32_t msgSize);

    //检查消耗品是否满足创建帮派并消耗后返回消息给func
    void servermsg_CreateFactionCost(const uint8_t* msgData, uint32_t msgSize);
    void createFactionCost(const RoleId roleId, const char* name, const uint64_t propId, const uint32_t propNum, const MoneyType type, const uint32_t moneyNum);


    uint16_t getSameSceneNumByRole(Role::Ptr role);

private:
    //所有world进程都有
    std::unordered_map<TeamId, std::unordered_set<RoleId>> m_teams;
};



}



#endif
