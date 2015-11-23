/*
 * Author: LiZhaojia
 *
 * Created: 2015-06-16 16:04 +0800
 *
 * Modified: 2015-06-16 16:04 +0800
 *
 * Description: 商城
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_STORE_HPP
#define PROTOCOL_RAWMSG_PUBLIC_STORE_HPP

#pragma pack(1)

namespace PublicRaw{


//c -> s
//请求购买某个商品
struct RoleRequestBuyGoods
{
    uint16_t shopId      = 0;//商城类型id
    uint8_t tabId        = 0;
    uint32_t goodsId     = 0;
    uint32_t goodsNum    = 0;
};


//c -> s
//端请求限量道具剩余数量
struct RoleRequestLimitObjTabNum
{
    uint16_t shopId     = 0;
    uint8_t tabId       = 0;
};


//s -> c
//返回限量道具剩余数量
struct RetLimitObjTabNum
{
    uint16_t shopId     = 0;
    uint8_t tabId       = 0;
    ArraySize size      = 0;
    struct SellObjInfo
    {
        uint32_t goodsId;
        uint32_t goodsNum;
    } info[0];
};

//s -> c
//刷新单个限量道具剩余数量
struct RefreshObjLimitNum
{
    uint16_t shopId     = 0;
    uint8_t tabId       = 0;
    uint32_t goodsId    = 0;
    uint32_t leftnum    = 0;
};

/*帮派商店*/
//c->s
//帮派商店物品请求
struct RequestFactionShop
{};

//s->c
//帮派商店物品回复
struct RetFactionShop
{
    uint64_t banggong;
    uint32_t refreshCost;   //刷新消耗帮贡
    uint32_t seconds;       //还有多少秒免费刷新
    ArraySize size;
    struct Goods
    {
        uint16_t geziId;
        uint16_t num;
        uint32_t objId;     //物品id
        uint32_t costNum;   //消耗货币数量
        MoneyType type;     //货币类型
        bool discount;      //是否打折
        bool ifCanBuy;      //是否可购买
    }data[0];
};

//c->s
//购买帮会商店物品
struct RequestBuyFactionObject
{
    uint16_t geziId;
};

//s->c
//回复帮派购买,失败不回复，成功回去各自id
struct RetBuyFactionObject
{
    uint16_t geziId;
};

//c->s
//刷新
struct RefreshFactionShop
{};
//s->c,成功RetFactionShop，失败走频道通知

}

#pragma pack()

#endif

