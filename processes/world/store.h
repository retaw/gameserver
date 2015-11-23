/*
 * Author: LiZhaojia
 *
 * Created: 2015-06-16 14:02 +0800
 *
 * Modified: 2015-06-16 14:02 +0800
 *
 * Description: 商店逻辑类
 */

#ifndef PROCESS_WORLD_STORE_H
#define PROCESS_WORLD_STORE_H

#include "role.h"
#include "water/componet/xmlparse.h"
#include "water/componet/exception.h" 
#include "protocol/rawmsg/public/store.h"
#include "protocol/rawmsg/public/store.codedef.public.h"
#include "protocol/rawmsg/private/store.h"
#include "protocol/rawmsg/private/store.codedef.private.h"

namespace world{

//元宝商城
const uint8_t ybStoreId = 5;
const uint8_t ybTabId = 1;

using namespace water;
using componet::XmlParseDoc;
using componet::XmlParseNode;

enum class BuyRetcode : uint8_t
{
    success     = 0, //成功
    full        = 1, //包裹满
    money       = 2, //money不够
    time        = 3, //不在道具出售时间范围
    serverLimit = 4, //本区该道具已售完
    dayLimit    = 5, //今天该道具已售完
    everyLimit  = 6, //超出今天自身购买上限
    banggong    = 7, //帮贡不足（与money区分开了）
};

struct ItemsConf
{
    uint32_t itemId;
    uint64_t price;
    uint16_t discount;
    MoneyType moneytype = MoneyType::none;
    uint16_t maxnum;    //单次最大购买数量
    uint16_t weight;    //权重
    uint32_t serverLimit = 0;   //全服限制出售数量(以下都是针对单件商品)
    uint32_t dayLimit = 0;      //每天商店限制出售数量
    uint32_t everyLimit = 0;    //每人每天限制购买数
    uint16_t objnum = 0;        //每人一次必须购买数量
    Bind bind = Bind::no;
};

struct TabConf
{
    uint8_t     tabId;
    MoneyType   moneytype = MoneyType::none;      //货币类型
    uint32_t    sellnum;   //限制出售的数量(一般为活动商店所用,结合活动时间)
    TimePoint   startTime = EPOCH;
    TimePoint   endTime = EPOCH;        
    uint32_t    openServerTime = 0;  //距离开服多久
    uint32_t    duration = 0;       //持续时间(天)
    std::unordered_map<uint32_t, ItemsConf> itemConfs; //<itemId, >
    uint16_t addUpWeight = 0;       //累计权重

};

class Store
{
public:
    explicit Store(uint16_t id);
    ~Store() = default;

    TYPEDEF_PTR(Store)
    CREATE_FUN_NEW(Store)

    std::unordered_map<uint8_t, TabConf> tabConfs;    //<tabId, >
private:

    BuyRetcode checkSellGoods(Role::Ptr role, const ItemsConf& itemcfg, uint32_t goodsNum);
    void successSellGoods(Role::Ptr role, uint8_t tabId, const ItemsConf& itemcfg, uint32_t goodsNum);
    void retSellGoods(Role::Ptr role, MoneyType moneytype, BuyRetcode retcode);

    //to func
    void requestFuncCheckObjLimit(RoleId roleId, uint8_t tabId, const ItemsConf& itemcfg, uint16_t goodsNum);
    void updateFuncObjLimit(RoleId roleId, uint8_t tabId, const ItemsConf& itemcfg, uint16_t goodsNum);

public:
    uint64_t price(TplId objId);
    void parse(const XmlParseNode& node, const XmlParseDoc& xml);
    std::vector<uint32_t> getgoodsIdByweight(const uint16_t tabId, const uint16_t num);
    
    void sellGoods(Role::Ptr role, const PublicRaw::RoleRequestBuyGoods* rev);
    void sellGoods(Role::Ptr role, const PrivateRaw::RetCheckObjRecord* rev);
    bool sellGoods(Role::Ptr role, const uint16_t tabId, const uint32_t goodsId, const uint32_t goodsNum);

    //帮派商店



private:
    uint16_t m_id;  //商店类型id
};

}

#endif

