#ifndef PROTOCOL_RAWMSG_PRIVATE_FIELD_BOSS_H
#define PROTOCOL_RAWMSG_PRIVATE_FIELD_BOSS_H

#pragma pack(1)

namespace PrivateRaw{

//world->func
//同步死亡时间到func
struct FieldBossToDie
{
    uint32_t bossId;
    RoleId killerId;
    ArraySize size;
    uint32_t objId[0];  //死亡时掉落的物品id
};
//同步刷新到func
struct RefreshFieldBoss
{
    uint32_t bossId;
};
//world或者func重启后主动同步野外boss数据
//暂时没有

}

#pragma pack()
#endif
