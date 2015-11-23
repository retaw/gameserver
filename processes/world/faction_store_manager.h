/*
 * Author: LiZhaojia
 *
 * Created: 2015-06-16 14:30 +0800
 *
 * Modified: 2015-06-16 14:30 +0800
 *
 * Description:
 */

#ifndef PROCESS_WORLD_STORE_MANAGER_H
#define PROCESS_WORLD_STORE_MANAGER_H

#include <unordered_map>
#include "store.h"

namespace world{

class StoreMgr
{
private:
    StoreMgr();

public:
    ~StoreMgr() = default;

public:
    static StoreMgr& me();

    void loadConfig(const std::string& cfgdir);
    void regMsgHandler();

    //所扣道具折算成元宝价格
    uint64_t objPrice(TplId objId);

private:
    void servermsg_RoleRequestBuyGoods(const uint8_t* msgData, uint32_t msgSize, RoleId rid);
    void clientmsg_RetCheckObjRecord(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    bool buyGoods(Role::Ptr role, const PublicRaw::RoleRequestBuyGoods* rev);

private:
    std::unordered_map<uint16_t, Store::Ptr> m_storeSet;

private:
    /**需要特殊逻辑的商店接口start**/
    //帮派商店需要的特殊数据或者额外配置的数据以及消息接口
    void clientmsg_RequestFactionShop(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_RequestBuyFactionObject(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RefreshFactionShop(const uint8_t* msgData, uint32_t msgSize);

    uint32_t m_openFactionLevel;
    std::unorderd_map<uint16_t, uint64_t> m_refreshCost;
    struct SelectShop
    {
        uint16_t shopId;
        uint16_t normalTabId;
        uint16_t specialTabId;
        uint16_t counter;
    }m_selcetCfg;
    /**需要特殊逻辑的商店接口end**/
};



}

#endif

