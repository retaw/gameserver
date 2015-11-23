#ifndef PROTOCOL_RAWMSG_PUBLIC_FACTION_HPP
#define PROTOCOL_RAWMSG_PUBLIC_FACTION_HPP

#include "water/common/roledef.h"
#include "water/common/factiondef.h"
#include "water/common/commdef.h"

#pragma pack(1)
namespace PublicRaw{
//c->s
//1.帮派大厅请求
struct ReqFactionHall
{};

//帮派成员
struct FactionMember
{
    RoleId roleId;
    FactionPosition position;  //0：未定义，1：会长，2：副会长，3：战士首席，4：法师首席，5：道士首席，6：普通成员, 7：首席，不指定首席职业
    char name[NAME_BUFF_SZIE];
    uint32_t level;
    Job job;
    uint64_t banggong;
    uint32_t offlnTime; //不在线的时间长（s）
};

//s->c
//自己的帮派大厅
struct RetFactionHall
{
    char notice[FACTION_NOTICE_SIZE];//500字节,暂定
    uint32_t level;
    uint64_t exp;
    uint64_t addLevelexp;
    char name[NAME_BUFF_SZIE];
    uint64_t resource;
    uint32_t maxMemberNum;//帮派当前可容纳人数
    uint32_t memberNum;
    uint64_t banggong;
    //FactionMember members[0];
};

//c->s
//客户端请求成员列表
struct ReqFactionMembers
{};
//s->c
//回复帮派成员列表
//c->s
struct RetFactionMembers
{
    ArraySize size;
    FactionMember members[0];
};

//1.1帮派列表请求
struct ReqFactionList
{};

struct FactionList
{
    FactionId factionId;//64位
    char name[NAME_BUFF_SZIE];
    uint32_t level;
    char leaderName[NAME_BUFF_SZIE];
    uint32_t memberNum;
    uint32_t maxMemberNum;
    bool beApplied;
};
//s->c
//帮派列表回复
struct RetFactionList
{
    ArraySize size;
    FactionList data[0];
};

//c->s
//1.1.1列表申请之后,客户端发角色自己的申请列表请求
struct ReqMyApplyRecord
{};
//s->c
struct RetMyApplyRecord
{
    ArraySize size;
    RoleId roleId[0];
};

//c->s
//2.帮派创建
struct CreateFaction
{
    char name[NAME_BUFF_SZIE];
};
//s->c
struct RetCreateFaction
{
    CreateFactionRet ret;   //0：未定义，1；成功，2：名字已经存在，。。。
    //RetFactionHall factionHall;    //帮派大厅
};

//c->s
//3.申请入帮
struct ApplyJoinFactionToS
{
    ArraySize size;
    char factionId[0]; 
};

//s->c
//如果成功了，给客户端发送反馈，客户端将该帮会的申请改为取消申请；如果不成功则不反馈
struct RetApplyJoinFactionToC
{
    FactionId factionId;
};

//s->c
//如果成功了，给帮派的领导们发消息，通知有新的申请
struct NewApplyJoinFaction
{
    uint16_t size;
};

//c->s
//不主动向leaders推送，等leaders自己拉申请列表
struct ApplyList
{};
struct RetApplyList
{
    ArraySize size;
    struct Member
    {
        RoleId roleId;
        char name[NAME_BUFF_SZIE];
        uint32_t time;
    }member[0];
};

//c->s
//leaders回复server是否同意,即对申请列表的处理
struct DealApplyRecord
{
    RoleId roleId;
    bool accept;    //同意与拒绝
};

//s->c
//如果被拒绝告诉申请者，为了申请者把取消申请按钮变回申请
struct RefuseApplyInFaction
{
    FactionId factionId;
};

//s->c
//处理之后返回消息，通知客户端(leaders)去掉该记录（处理之后将删除该记录）
struct RetDealApplyRecord
{
    RoleId roleId;  
};

//如果同意了，给所有帮派成员发送添加成员消息
struct AddFactionMem
{
   FactionMember member; 
};

//c->s
//4.取消某条申请
struct CancelApplyRecord
{
    FactionId factionId;
};

struct RetCancelApplyRecord
{
    FactionId factionId;
};

//c->s
//5.任命职位（副帮主、首席、庶民）
struct AppointLeader
{
    RoleId roleId;
    FactionPosition position;
};

//s->c
//告诉帮派所有成员任命谁为某职务
struct RetAppointLeader
{
    RoleId roleId;
    FactionPosition position;
};

//c->s
//6.邀请入帮
struct InviteJoinFactionToS
{
    RoleId roleId;
};
//s->c
//将邀请信息发给被邀请者的客户端(下线清空,对客户端来说,服务器不再保存这些数据)
struct InviteJoinFactionToC
{
    FactionId factionId;
    char factionName[NAME_BUFF_SZIE];
    char name[NAME_BUFF_SZIE];//邀请者名字
};
//c->s
//被邀请者是否同意发给服务端
struct RetInviteJoinFactionToS
{
    bool accept;    //1,同意。2,拒绝
    ArraySize size; //拒绝可以是多个,同意只能一个
    FactionId factionId[0];
};
//s->c
//不同意则被邀请者客户端自己删除该记录，服务器不维护该记录

//同意后，发给所有帮派成员更新消息（如果邀请者相关面板没打开，可忽略该消息）AddFactionMem
//发给被邀请者自己的帮派大厅，如果没打开相关面板可以忽略该消息 FactionHall,同时,客户端清空自己的被邀请记录

//c->s,s->c
//7.踢出帮派,客户端发送与所有客户端接收是同一个消息（被踢出者判断roleId与自己相同时，知道自己被踢出，做相应处理; 不是被提出者则相当于减少一个帮派成员）
struct KickOutFromFaction
{
    RoleId roleId;
};

//c->s
//8.退出帮派，如果成功返回本消息，如果失败则该角色是帮主，会发频道通知
struct LeaveFaction
{
};
//c->s
//如果失败则没有返回消息
//如果成功，返回删减成员消息（判断是不是自己，是自己则相应处理）
struct SubFactionMem
{
    RoleId roleId;
};

//c->s
//9.编辑公告
struct SaveNotice
{
    char notice[FACTION_NOTICE_SIZE];
};
//s->c
struct RetSaveNotice
{
    bool success;   //成功或失败
};

//c->s,是全部（200）发送还是一页一页请求,暂时是全部;是直接发字符串还是只发变量,暂时是只发变量
//10.帮派日志
struct ReqFactionLog
{};
struct FactionLog
{
    uint32_t time;
    LogType type;   //什么日志（1：加入帮派、2：离开帮派、3：贡献资源、4：帮派升级、5：帮派活动开启）
    uint32_t value; //升了多少级或者贡献了多少资源 
    char name[NAME_BUFF_SZIE];  //名字为玩家名字或者帮派名字
};
//s->c
struct RetFactionLog
{
    ArraySize size;
    FactionLog data[0];
};
//c->s,s->
//11.升级
struct FactionLevelUp
{};
//s->c
//给升级人发送帮派大厅RetFactionHall，用于更新帮派面板，其他帮派成员会受到频道通知
}

#pragma pack()
#endif
