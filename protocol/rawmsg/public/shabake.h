#ifndef PROTOCOL_PUBLIC_SHA_BA_KE_HPP
#define PROTOCOL_PUBLIC_SHA_BA_KE_HPP

#include "water/common/factiondef.h"
#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求沙巴克争夺战界面信息
struct ReqShabakeInfo
{
};

//s -> c
//返回沙巴克界面信息
struct RetShabakeInfo
{
    char factionName[NAME_BUFF_SZIE]; //占领帮派名称
    uint32_t nextActivityTime;  //下次活动时间(秒,正在活动中返回0)
    struct SbkPositionInfo
    {
        char name[MAX_NAME_SZIE + 1];
        Job job;
        Sex sex;
    } info[3];   //下标0:城主  1:左副城主  2:右副城主
};

//s -> c
//返回沙巴克争夺战进度问题
struct RefreshProgressOfShabake
{
    uint32_t starttime; //开始时间
    uint32_t endtime;   //结束时间
    uint32_t wintime;  //帮派占领皇宫时间
    char curWinFaction[NAME_BUFF_SZIE]; //当前占领帮派
};

//c -> s
//请求领取沙巴克争夺战奖励
struct ReqGetShabakeAward
{
    ShabakeAward awardtype;
};

//c -> s
//请求沙巴克领奖界面信息
struct ReqShabakaAwardInfo
{
};

//s -> c
//返回沙巴克奖励领取界面信息
struct RetShabakeAwardInfo
{
    bool kingAward = false;   //城主奖励是否可领
    bool winAward = false;    //占领奖励是否可领
    bool parttake = false;    //参与奖是否可领
};

//c -> s
//请求帮派沙巴克日常奖励信息
struct ReqShabakeDailyAwardInfo
{
};

//s -> c
//返回帮派沙巴克日常奖励信息
struct RetShabakeDailyAwardInfo
{
    bool canGet = false;    //可否领奖
    ShabakePosition position = ShabakePosition::none;
};

//s -> c
//通知玩家争夺战结束
struct NotifyShabakeOver
{
    char factionName[NAME_BUFF_SZIE];   //赢得争夺的帮派名字
    char king[MAX_NAME_SZIE + 1]; //城主名字
};

//c -> s
//请求官职列表
struct ReqShabakePosition
{
};

//s -> c
//刷新官职列表
struct RefreshShabakePosition
{
    uint16_t setLefttime;   //设置倒计时
    ArraySize size = 0;
    struct PositionInfo
    {
        ShabakePosition position;
        Job job;
        Sex sex;
        char name[MAX_NAME_SZIE + 1];
    }info[0];
};

//c -> s
//请求设置官职
struct ReqSetShabakePosition
{
    bool autoflag;      //是否自动设置
    RoleId targetId;    //被设置者id
    ShabakePosition position;  //被设置职位
};

}

#pragma pack()

#endif
