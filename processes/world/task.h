#ifndef PROCESS_WORLD_TASK_H
#define PROCESS_WORLD_TASK_H

#include "task_base.h"
#include "water/common/roledef.h"
#include <unordered_map>
#include <set>
#include <vector>

namespace world{


class Role;
class RoleFactionTask;
class RoleTask
{
    friend class RoleFactionTask;
public:
    explicit RoleTask(Role& me);
    ~RoleTask() = default;

public:
    //接取任务, autoAccept==true系统自动接取任务
    void acceptTask(TaskId taskId, PKId npcId, bool autoAccept=false);

    //客服端通知server 携带任务到达指定npc周边完成任务
    void arriveTargetTaskNpc(PKId npcId);

    //提交任务
    void submitTask(TaskId taskId, PKId npcId);

    //任务调度总接口
    void dispatch(TaskContent content, TaskParam param);

    //检查是否有新的任务可接取
    void checkAndUnlockTask(bool refresh=true);

    //刷新单个已接任务详细信息
    void refreshAcceptedTaskDetails(TaskId taskId, bool modify=true);

    //得到帮派任务的状态（只有接受或者完成的任务会找到，其它返回none）
    TaskState getFactionTaskState(TaskId taskId) const;


    //重做任务
    void redoTask(TaskId taskId);

public:
    void afterEnterScene();
    void beforeLeaveScene() const;

    void changeTaskState(TaskId taskId, TaskState state);
    //刷新单个任务状态给前端
    void refreshTaskState(TaskId taskId, TaskState state);

private:
    //
    void notifyNextUnacceptableMainTask();

private:
    //点击npc(include visit_npc, talk_npc, transfer_obj)
    void onVisitNpc(TaskParam param);
    //击杀npc
    void onKillNpc(TaskParam param);
    //通关副本
    void onPassCopy(TaskParam param);
    //采集
    void onCollection(TaskParam param);

    //下发任务奖励
    void giveTaskAward(TaskTpl::Ptr taskPtr);

public:
    void loadFromDB(const std::string& taskStr);
	void updateAllTaskInfoToDB() const;

public:
	void setDailyTaskStar(TaskId taskId, uint8_t star);

	void clearDailyTaskTopStarReward();
	void setDailyTaskTopStarRewardState(uint32_t num, Reward rewardState);
	const std::unordered_map<uint32_t, Reward>& getDailyTaskTopStarReward() const;
	
private:
    Role&   m_owner;
    std::set<TaskId> m_acceptableTasks;  //可接取的任务
    TaskId m_nextUnacceptableMainTask;  //下一个条件不满足的主线任务

private:
	//以下数据需要序列化
    std::unordered_map<TaskId, TaskInfo> m_acceptedTasks;	//已接取的任务
    std::unordered_map<TaskId, TaskInfo> m_finishedTasks;	//已完成未提交的任务
    std::set<TaskId>					 m_overTasks;		//完成并提交了的任务

	std::unordered_map<TaskId, TaskInfo> m_acceptableDailyTasks;	//可接受的日常任务
	std::unordered_map<uint32_t, Reward> m_dailyTaskTopStarReward;  //满星日常任务奖励状态
};

}

#endif
