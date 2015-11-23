#ifndef PROTOCOL_RAWMSG_PUBLIC_TASK_H
#define PROTOCOL_RAWMSG_PUBLIC_TASK_H

#include "water/common/taskdef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求接取任务
struct RequestAcceptTask
{
    TaskId taskId;
    PKId npcId; //接任务npc唯一id
};

//c -> s
//玩家走到任务目标npc身边通知后端任务完成(专用于找npc、传递信物、npc对话等)
struct NotifyArriveTaskTargetNpc
{
    PKId npcId;
};

//c -> s
//请求提交任务
struct RequestSubmitTask
{
    TaskId taskId;
    PKId npcId;
};

//s -> c
//刷新任务状态
struct RefreshTaskState
{
    bool modify = true; //true:变更 false:进场景
    ArraySize size = 0;
    struct TaskStateEle
    {
        TaskId  taskId;
        TaskState state;
    } data[0];
};

//s -> c
//刷新单个已接取任务详细信息
struct RefreshAcceptedTaskDetails
{
    bool modify = true; //true:变更 false:进场景
    TaskId taskId;
    ArraySize size = 0;
    struct TaskSteps
    {
        TaskContent content;//任务类型
        uint8_t state;      //任务步骤完成与否 0:未完成 1:完成
        uint16_t npcCount;  //击杀的npc数量
        uint16_t reserve;   //预留
    } data[0];
};

//s -> c
//next主线任务(等级条件不满足), 服务器主动下发给客户端
struct NotifyNextUnacceptableMainTask
{
    TaskId taskId;
};

//c -> s
//请求采集
struct RequestCollect
{
    PKId npcId; //采集物唯一id
};

//s -> c
//通知前端可以采集,开始读进度条
struct RetCanCollect
{
    uint16_t sec; //时间
};

//c -> s
//进度条读完, 通知服务端采集完成
struct FinishCollect
{
};

//s -> c
//中断采集(如玩家移动, 被攻击等等)
struct InterruptCollect
{
};

}

#pragma pack()

#endif
