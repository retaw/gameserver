#include "store_manager.h"
#include "role_manager.h"
#include "protocol/rawmsg/private/store.h"
#include "protocol/rawmsg/private/store.codedef.private.h"

#include "protocol/rawmsg/public/store.h"
#include "protocol/rawmsg/public/store.codedef.public.h"
namespace world{

StoreMgr::StoreMgr()
{
}


StoreMgr& StoreMgr::me()
{
    static StoreMgr me;
    return me;
}


void StoreMgr::loadConfig(const std::string& cfgdir)
{
    std::string file = cfgdir + "/shop.xml";
    XmlParseDoc doc(file);
    XmlParseNode root = doc.getRoot();
    if(!root)
    {
        EXCEPTION(componet::ExceptionBase, file + " parse root node failed");
        return;
    }

    for(XmlParseNode shopNode = root.getChild("shop"); shopNode; ++shopNode)
    {
        const uint16_t id = shopNode.getAttr<uint16_t>("id");
        if(0 == id || id >1000)
            continue;
        if(m_storeSet.find(id) != m_storeSet.end())
        {
            EXCEPTION(componet::ExceptionBase, file + " parse failed, 有重复的shopid={} 节点", id);
            return;
        }

        Store::Ptr store = Store::create(id);
        if(nullptr == store)
        {
            EXCEPTION(componet::ExceptionBase, file + " create store失败, shopid={}", id);
            return;
        }

        store->parse(shopNode, doc);
        m_storeSet.insert(std::make_pair(id, store));
    }
    //帮派需要的额外的数据表读取
    //帮派数据表
    file = cfgdir + "/faction_shop.xml";
    XmlParseDoc facdoc(file);
    XmlParseNode facroot = facdoc.getRoot();
    if(!facroot)
    {
        EXCEPTION(componet::ExceptionBase, file + " parse root node failed");
    }
    XmlParseNode refreshSecondsNode = facroot.getChild("refreshSeconds");
    m_refreshSeconds = refreshSecondsNode.getAttr<uint32_t>("seconds");

    XmlParseNode factionShopIdNode = facroot.getChild("factionShopId");
    m_factionshopId = factionShopIdNode.getAttr<uint16_t>("id");

    XmlParseNode openFactionLevelNode = facroot.getChild("openFactionLevel");
    m_openFactionLevel = openFactionLevelNode.getAttr<uint32_t>("level");

    XmlParseNode selectShopNode = facroot.getChild("selectShop");
    m_selectShopCfg.normalTabId = selectShopNode.getAttr<uint16_t>("normalTab");
    m_selectShopCfg.specialTabId = selectShopNode.getAttr<uint16_t>("specialTab");
    m_selectShopCfg.counter = selectShopNode.getAttr<uint16_t>("counter");

    XmlParseNode refreshCostNode = facroot.getChild("refreshtimes");
    for(XmlParseNode itemNode = refreshCostNode.getChild("item"); itemNode; ++itemNode)
    {
        uint16_t num = itemNode.getAttr<uint16_t>("num");
        uint64_t costBanggong = itemNode.getAttr<uint64_t>("costBanggong");
        m_refreshCost.emplace(num, costBanggong);
    }
    //读取与客户端共有的帮派商店配置表
    file = cfgdir + "/faction_shop_server.xml";
    XmlParseDoc facServerdoc(file);
    XmlParseNode facServerroot = facServerdoc.getRoot();
    if(!facServerroot)
    {
        EXCEPTION(componet::ExceptionBase, file + " parse root node failed");
    }
    for(XmlParseNode geziitemNode = facServerroot.getChild("item"); geziitemNode; ++geziitemNode)
    {
        uint32_t geziLevel = geziitemNode.getChildNodeText<uint32_t>("factionLevel");
        uint16_t geziNum = geziitemNode.getChildNodeText<uint32_t>("num");
        m_gezi.emplace(geziLevel, geziNum);
    }
}


uint64_t StoreMgr::objPrice(TplId objId)
{
    if(m_storeSet.find(ybStoreId) == m_storeSet.end())
        return (uint64_t)-1;

    const Store::Ptr store = m_storeSet[ybStoreId];
    if(nullptr == store)
        return (uint64_t)-1;

    return store->price(objId);
}


bool StoreMgr::buyGoods(Role::Ptr role, const PublicRaw::RoleRequestBuyGoods* rev)
{
    if(nullptr == role)
        return false;
    if(m_storeSet.find(rev->shopId) == m_storeSet.end())
    {
        LOG_ERROR("商城, 购买道具, 找不到role={},{}, shopId={}", role->name(), role->id(), rev->shopId);
        return false;
    }

    const Store::Ptr store = m_storeSet[rev->shopId];
    if(nullptr == store)
    {
        LOG_ERROR("商城, 购买道具, store is nullptr, role={},{}, shopId={}", role->name(), role->id(), rev->shopId);
        return false;
    }

    store->sellGoods(role, rev);
    return true;
}

bool StoreMgr::buyGoods(Role::Ptr role, uint16_t shopId, uint16_t tabId, uint32_t goodsId, uint16_t num)
{
    if(role == nullptr)
        return false;

    auto it = m_storeSet.find(shopId);
    if(it == m_storeSet.end())
        return false;
    if(num != 0)
        return it->second->sellGoods(role, tabId, goodsId, num);
    else
        return it->second->sellGoods(role, tabId, goodsId, it->second->tabConfs[tabId].itemConfs[goodsId].objnum);
}

void StoreMgr::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(RoleRequestBuyGoods, std::bind(&StoreMgr::clientmsg_RoleRequestBuyGoods, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RetCheckObjRecord, std::bind(&StoreMgr::servermsg_RetCheckObjRecord, this, _1, _2, _3));
    //帮派商店
    REG_RAWMSG_PRIVATE(RequestFactionShop, std::bind(&StoreMgr::servermsg_RequestFactionShop, this, _1, _2));
    REG_RAWMSG_PRIVATE(RefreshFactionShop, std::bind(&StoreMgr::servermsg_RefreshFactionShop, this, _1, _2));
    REG_RAWMSG_PUBLIC(RequestBuyFactionObject, std::bind(&StoreMgr::clientmsg_RequestBuyFactionObject, this, _1, _2, _3));

}

void StoreMgr::clientmsg_RoleRequestBuyGoods(const uint8_t* msgData, uint32_t msgSize, RoleId rid)
{
    auto role = RoleManager::me().getById(rid);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RoleRequestBuyGoods*>(msgData);
    buyGoods(role, rev);
}


void StoreMgr::servermsg_RetCheckObjRecord(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::RetCheckObjRecord*>(msgData);
    auto role = RoleManager::me().getById(rev->roleId);
    if(nullptr == role)
    {
        LOG_ERROR("商城, func返回限量道具检查结果时, roleId={}已下线", rev->roleId);
        return;
    }
    //这一步其实不可能再出现map里面没有该商店信息的情况,可以屏蔽掉不检查
    if(m_storeSet.find(rev->shopId) == m_storeSet.end())
        return;
    const Store::Ptr& store = m_storeSet[rev->shopId];
    store->sellGoods(role, rev);
}

//factiongshop
void StoreMgr::servermsg_RequestFactionShop(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RequestFactionShop*>(msgData);
    //判断帮派级别
    if(rev->factionLevel < m_openFactionLevel)
        return;
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    sendFactionShop(role);
}

void StoreMgr::sendFactionShop(Role::Ptr role)
{
    //if(role->m_roleSundry.m_facShopTab == 0 || role->m_roleSundry.m_facShopGoodsId.empty())
    {
    } 
    auto storeP = m_storeSet.find(m_factionshopId);
    if(storeP == m_storeSet.end())
        return;
    auto store = storeP->second;

    std::vector<uint8_t*> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::RetFactionShop));
    auto msg = reinterpret_cast<PublicRaw::RetFactionShop*>(buf.data());
    msg->banggong = role->banggong();
    //下次刷新属于的刷新次数
    auto times = role->m_roleCounter.get(CounterType::dayRefreshFacShopTime) + 1;
    auto it = m_refreshCost.find(times);
    if(it == m_refreshCost.end())//认为刷新次数超过最大值
        msg->refreshCost = m_refreshCost[10];
    else
        msg->refreshCost = it->second;
    if(role->m_roleSundry.m_refreshFacShopTime == 0)//未使用过刷新功能
        msg->seconds = 0;
    else
    {
        auto times = time(NULL) - role->m_roleSundry.m_refreshFacShopTime;
        msg->seconds = times >= m_refreshSeconds ? 0 : m_refreshSeconds - times;
    }
    msg->size = role->m_roleSundry.m_facShopGoodsId.size();
    uint16_t i = 0;
    for(auto& goodsId : role->m_roleSundry.m_facShopGoodsId)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetFactionShop::Goods));
        auto msg = reinterpret_cast<PublicRaw::RetFactionShop*>(buf.data());
        msg->data[i].geziId = i;
        uint16_t tabId = role->m_roleSundry.m_facShopTab;
        TabConf& tabconf = store->tabConfs[tabId];
        ItemsConf& itemConf = tabconf.itemConfs[goodsId.first];
        //...检查物品是否还存在，不存在的话重新随一个
        msg->data[i].objId = goodsId.first;
        msg->data[i].num = tabconf.itemConfs[goodsId.first].objnum;
        msg->data[i].costNum = itemConf.price;
        msg->data[i].type = itemConf.moneytype;
        msg->data[i].discount = (bool)itemConf.discount;
        msg->data[i].ifCanBuy = role->m_roleSundry.m_facShopGoodsId[i].second;

        i++;
    }
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionShop), buf.data(), buf.size());
}

void StoreMgr::servermsg_RefreshFactionShop(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RefreshFactionShop*>(msgData);
    //判断帮派级别
    if(rev->factionLevel < m_openFactionLevel)
        return;
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;

    //本次刷新属于的刷新次数
    auto times = role->m_roleCounter.get(CounterType::dayRefreshFacShopTime) + 1;
    time_t now = time(NULL);
    //如果帮贡刷新
    if(role->m_roleSundry.m_refreshedFactionShop && now < role->m_roleSundry.m_refreshFacShopTime + m_refreshSeconds)
    {
        //判断刷新帮贡并花钱
        uint64_t costBanggong;
        auto it = m_refreshCost.find(times);
        if(it == m_refreshCost.end())//认为刷新次数超过最大值
            costBanggong = m_refreshCost[10];
        else
            costBanggong = it->second;
        if(!role->checkBanggong(costBanggong))
        {
            role->sendSysChat("帮贡不足");
            return;
        }
        role->reduceBanggong(costBanggong, "帮派商店刷新");
    }

    //刷新(刷新的是角色自己的格子对应的shopid，tableid、goodsid)
    //...根据当天刷新次数得到tab
    uint16_t tabId;
    if(times % m_selectShopCfg.counter == 0)
        tabId = m_selectShopCfg.specialTabId;
    else
        tabId = m_selectShopCfg.normalTabId;
    //得到格子启动数
    auto gezi = m_gezi.find(rev->factionLevel);
    if(gezi == m_gezi.end())
    {
        LOG_ERROR("帮派商店, 帮派等级对应格子数配置出错");
        return;
    }
    //得到itemId
    auto storeP = m_storeSet.find(m_factionshopId); 
    if(storeP == m_storeSet.end())
    {
        LOG_ERROR("帮派商店, id配置错误");
        return;
    }
    auto& store = storeP->second;
    std::vector<uint32_t> goodsIdVec = store->getgoodsIdByweight(tabId, gezi->second);
    //...角色当天刷新次数
    role->m_roleCounter.add(CounterType::dayRefreshFacShopTime);
    //重置角色的帮派商店数据
    if(role->m_roleSundry.m_refreshedFactionShop && now < role->m_roleSundry.m_refreshFacShopTime + m_refreshSeconds)//帮贡刷新不改变上次刷新时间
        role->m_roleSundry.setFacShop(role->m_roleSundry.m_refreshFacShopTime, tabId, goodsIdVec);
    else
        role->m_roleSundry.setFacShop(now, tabId, goodsIdVec);//免费刷新改变上次刷新时间

    sendFactionShop(role);
}

void StoreMgr::clientmsg_RequestBuyFactionObject(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto rev = reinterpret_cast<const PublicRaw::RequestBuyFactionObject*>(msgData);
    if(msgSize != sizeof(PublicRaw::RequestBuyFactionObject))
        return;

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    if(rev->geziId < 0 || rev->geziId > role->m_roleSundry.m_facShopGoodsId.size() - 1)
        return;

    //是否可购买
    if(!role->m_roleSundry.m_facShopGoodsId[rev->geziId].second)
        return;

    if(!buyGoods(role, m_factionshopId, role->m_roleSundry.m_facShopTab, role->m_roleSundry.m_facShopGoodsId[rev->geziId].first))
        return;
    //把该物品标记为不可购买
    role->m_roleSundry.m_facShopGoodsId[rev->geziId].second = false;
    role->m_roleSundry.saveSundry();

    PublicRaw::RetBuyFactionObject send;
    send.geziId = rev->geziId;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetBuyFactionObject), (uint8_t*)&send, sizeof(send));
}

}
