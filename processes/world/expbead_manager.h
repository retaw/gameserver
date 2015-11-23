/*
 * 经验珠
 */
#ifndef PROCESS_EXPBEAD_MANAGER_H
#define PROCESS_EXPBEAD_MANAGER_H

#include <string>
#include <vector>

#include "water/componet/exception.h"
#include "water/common/objdef.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/expbead.h"

namespace world{

DEFINE_EXCEPTION(ExpBeadException, water::componet::ExceptionBase)
struct Vip
{
    uint32_t level = 0;
    uint16_t num = 0;
};
struct StarLevel
{
    uint16_t level = 0;
    uint32_t exp = 0;
    uint16_t addLevelRandom = 0;
};
struct Type
{
    uint16_t ratio = 0;
    uint32_t costVcoin = 0;
    uint64_t objectId = 0;
    uint32_t objectNum = 0;
    uint16_t random = 0;//获得物品的几率?/1000
    Bind objBind = Bind::none;
    uint32_t vip = 0;//到达此等级才可使用该倍数
};

struct Config
{
    uint32_t openLevel = 0;
    uint64_t refreshCostMoney = 0;
    uint64_t buyRefreshCostVcoin = 0;
    uint16_t refreshUpTimes = 0;
    uint16_t maxGetTimes = 0;
    uint64_t buyGetTimesCost = 0;
    std::vector<Vip> vipVec;
    std::vector<StarLevel> starLevelVec;
    std::map<uint16_t, Type> typeMap;
};

class ExpBeadManager
{
public:
    ~ExpBeadManager() =default;
    static ExpBeadManager& me();

    void regMsgHandler();
    void loadConfig(const std::string& cfgDir);
    void clear();//清空

private:
    ExpBeadManager() = default;

    void clientmsg_ExpBeadShow(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ExpBeadMainWindow(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ExpBeadGet(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ExpBeadRefresh(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_BuyGetTimes(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
    void sendMainWindowToClient(Role::Ptr role);
    bool fillMainWindow(Role::Ptr role, PublicRaw::RetExpBeadMaiWindow& send);
    bool getExpBead(Role::Ptr role, const uint16_t ratio);
    bool addExpAndSetInfo(Role::Ptr role, const uint16_t ratio);
    bool expBeadRefresh(Role::Ptr role);
    bool buyGetTimes(Role::Ptr role);
    bool initBeanInfo(Role::Ptr role);
    bool costVcoin(Role::Ptr role, const uint64_t num, const std::string& reference);//消耗非绑定元宝

private:
    Config m_cfg;//配置
    struct ExpBeadInfo{
        uint32_t starLevel;
        uint16_t refreshTimes;//当前剩余刷新次数
        uint16_t getTimes;//当前剩余领取次数
        uint16_t buyGetTimes;//已购买领取次数
    };
    std::unordered_map<RoleId, ExpBeadInfo> m_expBeadInfo;//使用过就保存，未使用的为默认值,第二天凌晨清空
};

}
#endif
