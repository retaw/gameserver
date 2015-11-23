#ifndef PROCESS_WORLD_FACTIONACTIVE_MANAGER_H
#define PROCESS_WORLD_FACTIONACTIVE_MANAGER_H

#include "factiontask.h"
#include "water/common/taskdef.h"
#include "role_manager.h"
#include "water/common/roledef.h"

namespace world{

struct FactionTaskRewardConf
{
    uint32_t minLevel;
    uint32_t roleExp;
    uint32_t heroExp;
    uint32_t banggong;
    uint32_t factionExp;
    uint32_t factionResource;
    uint32_t objTplId;
    uint32_t objNum;
    Bind bind;
};

class FactionActiveManager
{
    friend class FactionTask;
public:
    ~FactionActiveManager() = default;

    static FactionActiveManager& me();

    void regMsgHandler();

    void loadConfig(const std::string& cfgdir);

    void fillFactionTaskReward(Role& role);

private:
    FactionActiveManager() = default;

    void clientmsg_FactionActive(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_FactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_AcceptFactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_FinishFactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_FactionTaskBuyVipGift(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_QuitFactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);


private:
    //void sendRetFactionTask(Role& role);
    bool factionTaskStart(Role::Ptr role);

//task
public:
    uint16_t m_taskNum = 20;
    uint64_t m_vipBuyPriceNum = 0;
    MoneyType m_vipBuyMoneytype;
private:
    uint32_t m_taskStartLevel = 1;
    uint32_t m_rewardId = 0;
    
    std::vector<std::vector<FactionTaskRewardConf>> m_rewardConfs;
};


}
#endif
