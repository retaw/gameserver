#ifndef PROTOCOL_PRIVATE_WORLD_BOSS_HPP
#define PROTOCOL_PRIVATE_WORLD_BOSS_HPP

#include "water/common/commdef.h"

#pragma pack(1)

namespace PrivateRaw{

//func -> world
//开启世界boss
struct StartWorldBoss
{
    uint16_t bossLv;
    uint32_t sec;   //活动持续时长(秒)
};


//func -> world
//结束世界boss
struct EndWorldBoss
{
};

//world -> func
//杀死boss
struct KillWorldBoss
{
    char killer[MAX_NAME_SZIE+1];
};

//world -> func
//宝箱归属
struct BossBoxBelonged
{
    char holder[MAX_NAME_SZIE+1];
};

//func -> world
//玩家请求世界boss信息 func转发world
struct TransitWorldBossInfo
{
    PKId id;
    uint16_t bossLv;
};

}

#pragma pack()

#endif
