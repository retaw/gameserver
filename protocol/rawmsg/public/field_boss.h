#ifndef PROTOCOL_RAWMSG_PUBLIC_FIELD_BOSS_H
#define PROTOCOL_RAWMSG_PUBLIC_FIELD_BOSS_H

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//1.世界boss界面
//c->s
struct FieldBoss
{
};
//s->c
struct RetFieldBoss
{
    uint32_t mohun; //魔魂值 
    ArraySize size;
    struct BossData
    {
        uint32_t bossId;
        uint16_t refreshSeconds;            //refreshSeconds后刷新
    }data[0];
};

//2.bosss刷新
//s->c
struct RefreshFieldBoss
{
    uint32_t bossId;
};
//击杀boss
struct KillFieldBoss
{
    RoleId roleId;  //64
    uint32_t bossId;
    uint16_t refreshSeconds;            //refreshSeconds后刷新
    char killerName[NAME_BUFF_SZIE];    //33字节
    ArraySize size;                     //16位
    uint32_t objId[0];                  //boss死亡掉落物品
};

//3.传送
//c->s
struct TransFerToFieldBoss
{
    uint32_t bossId;
};
//如果成功返回
struct TransFerToFieldBossSuccess
{};

}
#pragma pack()

#endif
