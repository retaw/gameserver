#ifndef PROTOCOL_RAWMSG_PUBLIC_EXPBEAD_HPP
#define PROTOCOL_RAWMSG_PUBLIC_EXPBEAD_HPP

#include "water/common/roledef.h"

#pragma pack(1)
namespace PublicRaw{

//1.主界面是否显示，只用于图标变化（上线、vip的时候客户端主动发送此消息）
//client->server
struct ExpBeadShow
{
};
//s->c
struct RetExpBeadShow
{
    bool show;//1,显示；0，不显示。
};

//2.点击经验珠系统界面(点击领主界面、刷新或者当面板打开时可能造成面板数据更新的情况发送此消息)
//client->server
struct ExpBeadMainWindow
{};
//s->c
struct RetExpBeadMaiWindow
{
    uint16_t starLevel;//星级
    uint64_t starLevelExp;//当前星级的基础经验
    uint64_t starUpLevelExp;//满级(6级)经验
    uint16_t getTimes;//当前剩余领取次数
    uint16_t maxGetTimes;//领取次数上限(如果是vip则忽略该数据)
    uint16_t buyGetTimes;//已购买的领取次数
    uint16_t refreshTimes;//当前剩余刷新次数
    uint16_t refreshMaxTimes;//刷新上限
};

//3.领取
//client->server
struct ExpBeadGet
{
    uint16_t ratio;//领取倍数
};
//s->c(成功返回RetExpBeadMaiWindow)

//4.刷新
//client->server
struct ExpBeadRefresh
{};
//s->c
//回复RetExpBeadMaiWindow

//5.client->server
//购买领取次数（一次只买一次领取机会）
struct BuyGetTimes
{};
//s->c
struct RetBuyGetTimes
{
    uint16_t getTimes;//当前可领取次数
    uint16_t buyGetTimes;//已购买的领取次数
};

}
#pragma pack()

#endif
