#ifndef PROTOCOL_RAWMSG_PUBLIC_PRIVATE_BOSS_H
#define PROTOCOL_RAWMSG_PUBLIC_PRIVATE_BOSS_H

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//1.个人boss界面
//c->s
struct PrivateBoss
{
};
//s->c
struct RetPrivateBoss
{
    uint32_t mohun; //魔魂值 
    ArraySize size;
    struct PrivateBossData
    {
        uint32_t bossId;
        uint16_t canEnterTimes;              //还可击杀次数
    }data[0];
};

//2.传送
//c->s
struct TransFerToPrivateBoss
{
    uint32_t bossId;
};
//如果成功返回(此功能客户端已经做了)
//TransFerToFieldBossSuccess

//3.个人boss右侧面板刷新
//c->s
struct ReqRefreshPrivateBoss
{};

//s->c
struct RefreshPrivateBoss
{
    ArraySize size;
    struct NpcData
    {
        uint32_t npcId;
        uint16_t killNum; //击杀的数量
        uint16_t allNum;    //
    }data[0];
};

//4.离开(离开的时候该领奖就领奖，不该领奖就直接离开)
//c->s
struct LeavePrivateBoss
{
};

//5.推送倒计时时间（秒）
//s->c
struct PrivateBossRemainSeconds
{
    uint32_t bossId;
    uint32_t seconds;
};

//6.击杀个人boss物品掉落广播
struct PrivateBossBroadcastObj
{
    RoleId roleId;
    uint32_t bossId;
    char killerName[NAME_BUFF_SZIE];    //33字节
    ArraySize size;                     //16位
    uint32_t objId[0];                  //boss死亡掉落物品
};

}
#pragma pack()

#endif
