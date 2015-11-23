#ifndef PROTOCOL_RAWMSG_PUBLIC_BOSS_HOME_H
#define PROTOCOL_RAWMSG_PUBLIC_BOSS_HOME_H

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//1.世界boss界面
//c->s
struct BossHome
{
};
//s->c
struct RetBossHome
{
    uint32_t mohun; //魔魂值 
    ArraySize size;
    struct BossHomeData
    {
        uint32_t bossId;
        uint16_t refreshSeconds;            //refreshSeconds后刷新
    }data[0];
};

//2.bosss刷新
//s->c
struct RefreshBossHome
{
    uint32_t bossId;
};
//击杀boss
struct KillBossHome
{
    RoleId roleId;  //64
    uint32_t bossId;
    uint16_t refreshSeconds;            //refreshSeconds后刷新
    char killerName[NAME_BUFF_SZIE];    //33字节
    ArraySize size;                     //16位
    uint32_t objId[0];                  //boss死亡掉落物品
};

//3.传送到下一层
//c->s
struct TransFerToNextBossHome
{
};
//传送到boss之家
struct TransFerToBossHome
{
    uint32_t bossId;
};
//如果成功返回(这条已经做过，不许再做)
//struct TransFerToFieldBossSuccess

//4.离开boss之家
//c->s
struct LeaveBossHome
{};


//5.右侧列表,只有进入场景的时候给一次，其他时候有更新根据boss死亡和boss刷新消息更新。暂时自己过来拉（服务器也可推送）
//c->s
struct BossHomeInCurrentScene
{};

//s->c
struct RetBossHomeInCurrentScene
{
    ArraySize size;
    struct BossHomeInCurrentSceneData
    {
        uint32_t bossId;
        uint16_t refreshSeconds;            //refreshSeconds后刷新
    }data[0];
};

}
#pragma pack()

#endif
