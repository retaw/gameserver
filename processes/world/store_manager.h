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

    //道具折算成元宝价格
    uint64_t objPrice(TplId objId);

/*通用的普通购买接口*/
private:
    void clientmsg_RoleRequestBuyGoods(const uint8_t* msgData, uint32_t msgSize, RoleId rid);
    void servermsg_RetCheckObjRecord(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    bool buyGoods(Role::Ptr role, const PublicRaw::RoleRequestBuyGoods* rev);

    //如果num=0,则是从itemconf里读取objnum，此字段用来限定每人每次购买次数
    bool buyGoods(Role::Ptr role, uint16_t shopId, uint16_t tabId, uint32_t goodsId, uint16_t num =0);

private:
    std::unordered_map<uint16_t, Store::Ptr> m_storeSet;

/*帮派商店接口*/
    void servermsg_RequestFactionShop(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_RefreshFactionShop(const uint8_t* msgData, uint32_t msgSize);
    void clientmsg_RequestBuyFactionObject(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void sendFactionShop(Role::Ptr role);
    uint32_t m_openFactionLevel;
    std::map<uint32_t, uint16_t> m_gezi;
    std::unordered_map<uint16_t, uint64_t> m_refreshCost;
    struct
    {
        uint16_t normalTabId;
        uint16_t specialTabId;
        uint16_t counter;
    }m_selectShopCfg;
    uint32_t m_refreshSeconds = 60*100;
    uint16_t m_factionshopId = 6;
};



}

#endif

