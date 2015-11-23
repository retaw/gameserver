#ifndef PROCESS_WORLD_TASK_BASE_H
#define PROCESS_WORLD_TASK_BASE_H

#include "water/common/roledef.h"
#include "water/common/scenedef.h"
#include "water/componet/class_helper.h"
#include <unordered_map>
#include <map>
#include <functional>

namespace world{

//任务参数数据结构
struct TaskParam
{
    TaskParam()
    {
        memset(this, 0, sizeof(*this));
    }

    union
    {
        //对话, 找npc, 递送, 采集
        struct
        {
            PKId npcId;
        };
        
        //击杀npc
        struct 
        {
            TplId npcTplId;
            uint32_t npclevel;
        };

        //通关副本
        struct 
        {
            MapId mapId;
        };

    }element;
};

//任务奖励
struct TaskAward
{
    TplId   objId;
    uint32_t objNum;
    Bind    bind;
    Job     job;
};


//任务配置内容
struct TaskContentCfg
{
    TaskContent content;    //任务目标类型
    uint32_t    param1;     //参数1
    uint32_t    param2;     //参数2
    uint32_t    param3;     //参数3
};

struct TaskTpl
{
    TYPEDEF_PTR(TaskTpl)
    CREATE_FUN_NEW(TaskTpl)

    TaskId taskId;
    TaskType type;
    TaskId preTaskId;       //前置任务
    uint8_t autoAccept;     //1:自动接取   0:不自动接取
    uint32_t minLevel;      //接取任务需要主角的最低等级
    uint32_t maxLevel;      //最高等级
    uint16_t factionLevel;  //接任务所需帮派等级
    TplId acceptNpc;        //接任务npc
    TplId submitNpc;        //提交任务npc

    TplId taskObj;          //接任务时获得的信物
    uint8_t order;          //0:多步骤任务不必顺序执行  1:必须顺序执行
    std::vector<TaskContentCfg> taskContentCfgs; //任务内容

    std::vector<TaskAward> taskObjAwards;//完成任务道具奖励
};


struct TaskBase
{
private:
    TaskBase();

public:
    ~TaskBase() = default;

public:
    static TaskBase& me();

    void loadConfig(const std::string& cfgdir);
    TaskTpl::Ptr getTaskTpl(TaskId taskId);
    TaskTpl::Ptr getFactionTaskTpl(TaskId taskId);

    void execTaskTpls(std::function<bool (TaskTpl::Ptr)> exec);
	const std::unordered_map<TaskId, TaskTpl::Ptr>& getTaskCfg() const;

private:
    std::unordered_map<TaskId, TaskTpl::Ptr> m_taskCfgs;

public:
    std::unordered_map<TaskId, TaskTpl::Ptr> m_factionTaskCfgs;	//帮派任务
};

}

#endif
