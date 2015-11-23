#ifndef PROTOCOL_PRIVATE_SHA_BA_KE_HPP
#define PROTOCOL_PRIVATE_SHA_BA_KE_HPP

#include "water/common/roledef.h"
#include "water/common/factiondef.h"

#pragma pack(1)

namespace PrivateRaw{

//func -> world
//开启沙巴克
struct StartShabake
{
    FactionId lastWinFaction; //上一次占领皇宫的帮派
    char winFactionName[NAME_BUFF_SZIE];
    uint32_t sec;   //活动持续时长(秒)
};


//func -> world
//结束
struct EndShabake
{
    char king[MAX_NAME_SZIE + 1];
};

//world -> func
//赢得沙巴克争夺
struct WinShabake
{
    FactionId factionId;
};

//world <-> func
//同步当前沙巴克占领帮派id
struct SyncCurTempWinFaction
{
    FactionId factionId;
    char factionName[NAME_BUFF_SZIE];
};

//world -> func
//同步当前沙巴克场景上角色到func
struct SyncShabakeSceneRoleToFunc
{
    ArraySize size = 0;
    RoleId roleId[0];
};

//func -> world
//通知world发奖励
struct NotifyWorldGiveRoleAward
{
    RoleId roleId;
    bool first; //是否活动首次奖励, 要区分首次城主.非首次城主等
    ShabakePosition position;
    ShabakeAward awardtype;
};

//world -> func
//world反馈func是否发奖励成功
struct WorldRetGiveRoleAward
{
    RoleId roleId;
    ShabakeAward awardtype;
    bool success;    //true:成功  false:不成功
};

//func -> world
//请求日常奖励是否可领取
struct FuncReqDailyAwardInfo
{
    RoleId roleId;
    ShabakePosition position = ShabakePosition::none;
};

//world -> func
//请求城主名字,用于主城雕塑
struct ReqShabakeKing
{
};

//func -> world
//返回城主名字
struct RetShabakeKing
{
    char king[MAX_NAME_SZIE + 1];
};

}

#pragma pack()

#endif
