#ifndef WATER_COMMON_TASK_DEF_H
#define WATER_COMMON_TASK_DEF_H

#include "commdef.h"


enum class TaskType : uint8_t
{
	none		= 0,
    main        = 1,    //主线
    branch      = 2,    //支线
    faction     = 3,    //帮派
	daily		= 4,	//日常
};

enum class TaskState : uint8_t
{
    no          = 0,    //不可接取
    acceptable  = 1,    //可接取任务
    accepted    = 2,    //已接取还未完成
    finished    = 3,    //已完成未提交
    over        = 4,    //完成并提交
    quit        = 5,    //放弃任务
};

//任务内容类型
enum TaskContent : uint8_t
{
    visit_npc   = 1,    //找npc
    talk_npc    = 2,    //与npc对话
    kill_npc    = 3,    //击杀npc
    transfer_obj= 4,    //传递信物
    pass_copy   = 5,    //通关副本
    collection  = 6,    //采集
};


#pragma pack(1)

struct TaskStep
{
    TaskContent content;
    uint8_t state;      //本任务步骤完成与否标志
    uint16_t count;     //击杀npc数量
    uint16_t reserve;   //预留
};

struct FactionTaskReward
{
    uint32_t roleExp;
    uint32_t heroExp;
    uint32_t banggong;
    uint32_t factionExp;
    uint32_t factionResource;
    uint32_t objTplId;
    uint32_t objNum;
};

#pragma pack()


#endif
