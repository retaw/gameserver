#ifndef PROTOCOL_RAWMSG_PUBLIC_TEAM_HPP
#define PROTOCOL_RAWMSG_PUBLIC_TEAM_HPP

#include "water/common/roledef.h"
#include "water/common/commdef.h"
#include "water/common/frienddef.h"

#pragma pack(1)

namespace PublicRaw{

//c->s
//1.我的队伍
struct TeamMembers
{};

struct TeamMember
{
    RoleId roleId;
    char name[NAME_BUFF_SZIE];
    uint32_t level;
    Job job;
    uint16_t mapId;//地图ID~~~~~~
    char factionName[NAME_BUFF_SZIE];//帮会
    bool isCaptain;//是否是队长
    uint32_t vipLevel;//0代表非会员
};
//我的队伍回复
struct RetTeamMembers
{
    ArraySize size;
    TeamMember data[0];//变长
};


//c->s
//7.创建队伍
struct CreateTeam
{
};
//s->c
//回复创建队伍，即回复我的队伍，为RetTeamMembers

//c->s
//8.解散队伍
struct BreakTeam
{
};
//s->c
//回复所有队员解散队伍
struct RetBreakTeam
{
};


//c->s
//9.离开队伍
struct LeaveTeam
{
};
//s->c，给所有队员回复离开信息（客户端要根据roleid判断是否是本人离开）
struct RetLeaveTeam
{
    RoleId roleId;
};
//s->c, 离开的是队长会多发一条消息，通知所有队员更新队长RetChangeCaptain

//c->s
//9.1踢出队伍
struct KickOutTeam
{
    RoleId roleId;
};
//s->c
//踢出队伍回复
struct BeKickOutTeam
{
    RoleId roleId;
};

//c->s
//10.申请入队
struct ApplyJoinToS
{
    TeamId teamId;
};
//s->c
struct ApplyJoinToC
{
    RoleId roleId;
    char name[NAME_BUFF_SZIE];//申请者名字
};
//c->s
struct RetApplyJoinToS
{
    RoleId roleId;
    bool acpt;//1同意，2拒绝
};
//s->c,申请成功
//原有队员收到添加成员
struct AddMember
{
    TeamMember data;
};
//s->c,给申请人发送反馈
struct RetApplyJoinToC
{
    bool status; //0,失败;1,成功
    ApplyJoinTeamFailType type; //1,拒绝;2,队伍满  
};

//c->s
//11.组队邀请
struct InviteJoinToS
{
    RoleId roleId;
};
//s->c
//组队邀请
struct InviteJoinToC
{
    TeamId teamId;   //队长id
    char captainName[NAME_BUFF_SZIE];
};
//c->s
//组队邀请回复
struct RetInviteJoinToS
{
    TeamId teamId;   //队长id
    bool acpt;  //是否同意
};
//s->c,申请成功
//原有队员(包括队长)收到添加成员AddMember

//s->c,
//给被邀请者发送反馈，被邀请者需自己请求RetTeamMembers（失败的情况只有队伍满才发送）
struct RetInviteJoinToPassiveToC
{
    bool status; //0,失败;1,成功（成功需要客户端拉我的队伍消息，失败弹窗）
    ApplyJoinTeamFailType type; //失败一定是队伍满，不存在拒绝(这个可以没有)
};
//给邀请人（队长）发送反馈（失败的情况只有对方拒绝才发送）
struct RetInviteJoinToAcctiveToC
{
    bool status; //0,失败;1,成功 (只有失败,需要弹窗)
    ApplyJoinTeamFailType type; //（只有对方拒绝，队伍满的情况走频道）
};

//12.转移队长
//c->s,客户端（原队长）告诉服务端转移队长
struct ChangeCaptain
{
    RoleId newCaptainId;
};
//s->c,服务回馈诉所有队员队长转移
struct RetChangeCaptain
{
    RoleId newCaptainId;
};
//13.附近队伍
//c->s,客户端发送周围玩家的teamId
struct NearbyTeams
{
    ArraySize size;
    TeamId data[0];
};
//s->c
//附近队伍回复
struct RetNearbyTeams
{
    ArraySize size;
    struct NearbyTeam
    {
        TeamId teamId;
        char captainName[NAME_BUFF_SZIE];
        uint32_t captainLevel;
        Job captainJob; //1.战士;2.法师;3.道士
        uint16_t memNum;
        char captainFaction[NAME_BUFF_SZIE];
    }data[0];
};

//14.组队
//c->s
struct FormTeam
{
    RoleId roleId; 
};

}
#pragma pack()
#endif
