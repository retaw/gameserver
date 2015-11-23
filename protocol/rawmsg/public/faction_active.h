#ifndef PROTOCOL_RAWMSG_PUBLIC_FACTIONACTIVE_H
#define PROTOCOL_RAWMSG_PUBLIC_FACTIONACTIVE_H

#include "water/common/taskdef.h"

#pragma pack(1)

namespace PublicRaw{
//c->s
//1.派活动大厅
struct FactionActive
{};
//s->c
//帮派活动大厅回复
struct RetFactionActive
{
    uint16_t taskNum = 0;   //任务总数
    uint16_t finishedTaskNum = 0;   //未完成的任务数
    uint16_t copyMapNum = 0;
    uint16_t finishedCopyMapNum = 0;
    uint16_t guardDragonNum = 0;
    uint16_t finishedGuardDragonNum = 0;
    uint16_t expHolyFlam = 0;
    uint16_t finishedExpHolyFlam = 0;

};

struct FinishedTaskNum
{
    uint16_t num;
};

//c->s
//2.帮派任务
struct FactionTask
{
};
//s->c
//帮派任务回复
struct RetFactionTask
{
   TaskId taskId;
   TaskState state;    //(1:可接受，2:已经接受，3:已完成)
   FactionTaskReward reward;    //(角色经验，英雄经验，帮贡，帮派经验，帮派经验，物品id，物品数量全部是32位)
};

//c->s
//3.接受帮派任务
struct AcceptFactionTask
{
};
//s->c
//接受帮派任务回复
//RefreshTaskState

//c->s
//4.完成帮派任务
struct FinishFactionTask
{};
//s->c
//完成回复
//RefreshTaskState

//c->s
//5.一折领取礼包,在特权完成动画后再发送此消息,客户端判断任务是否完成判断是否播放动画，然后发送
struct FactionTaskBuyVipGift
{
};
//s->c
//一折领取回复(失败原因走频道,格子是否满客户端需要判断？随意)
//发一条RefreshTaskState通知当前接受的任务已提交

//s->c
//6.刷新已接受任务的详细信息
//RefreshAcceptedTaskDetails

//c->s
//7.放弃任务
struct QuitFactionTask
{};
//RefreshTaskState,状态为放弃
}
#pragma pack()

#endif
