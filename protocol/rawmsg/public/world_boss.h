#ifndef PROTOCOL_RAWMSG_PUBLIC_WORLD_BOSS_HPP
#define PROTOCOL_RAWMSG_PUBLIC_WORLD_BOSS_HPP

#include "water/common/commdef.h"
#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求世界boss界面信息
struct ReqWorldBossInfo
{
};

//s -> c
//世界boss界面信息
struct RetWorldBossInfo
{
    uint16_t bossLv;    //boss等级
    uint32_t totalDamage; //自己对boss的累积伤害
};

//c -> s
//请求排行榜信息 (客户端定时向服务器请求数据)
struct RequestDamageRank
{
};

//s -> c
//刷新排行榜
struct RefreshDamageRank
{
    uint16_t selfRank;
    uint32_t selfDamage;

    ArraySize size = 0;
    struct DamageInfo
    {
        char name[MAX_NAME_SZIE+1];
        uint32_t damage;
    } info[0];
};

//c -> s
//请求世界boss伤害奖励领取界面信息
struct ReqDamageAwardInfo
{
};

//c -> s
//请求领取世界boss伤害奖励
struct ReqGetDamageAward
{
    bool all;   //一键全部领取
    uint32_t damageIndex; //某个伤害数值的奖励
};

//s -> c
//刷新领取奖励界面信息
struct RefreshDamageAwardInfo
{
    uint32_t totalDamage;
    ArraySize size = 0;
    struct DamageAwardInfo
    {
        uint32_t damageIndex;
        uint8_t state;  //2:已领取 1:可领取 0:不可领取
    } info[0];
};

//s -> c
//统一通知世界boss活动结束
struct NotifyWorldBossOver
{
    char boxHolder[MAX_NAME_SZIE+1];
};

}
#pragma pack()

#endif
