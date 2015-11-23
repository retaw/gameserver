/*
 * Author: zhupengfei
 *
 * Created: 2015-10-28 10:12 +0800
 *
 * Modified: 2015-10-28 10:12 +0800
 *
 * Description: 日常任务
 */

#ifndef PROCESS_WORLD_DAILY_TASK_HPP
#define PROCESS_WORLD_DAILY_TASK_HPP

#include "task_base.h"

#include "water/common/roledef.h"
#include "water/componet/datetime.h"

#include <vector>
#include <unordered_map>

namespace world{

using namespace water;
using water::componet::TimePoint;

class Role;

class DailyTask
{
public:
	explicit DailyTask(Role& owner);
	~DailyTask() = default;

public:
	void loadFromDB(const std::vector<TaskInfo>& taskInfoVec);
	void dealNotSameDay();

	void finishTask(TaskId taskId);
	
	uint32_t getDailyTaskTime(TaskId taskId) const;
	uint8_t getDailyTaskStar(TaskId taskId) const;

	void roleLevelUp();
	void afterEnterScene();

public:
	void requestDailyTaskInfo();
	void requestAcceptTask(TaskId taskId);
	void requestRefreshDailyTaskStar();
	void requestDailyTaskTopStar();
	void requestGetDailyTaskReward(bool isMulti);
	void requestFinishAllDailyTask(bool isMulti);
	void requestGetTopStarTaskNumReward(uint32_t num);

private:
	void refreshTask(); 
	uint8_t randomTaskStar();
	uint32_t getStarPercent(uint8_t star);
	std::vector<std::pair<uint32_t, uint32_t> > getTaskReward(uint32_t level);

	void giveOutTaskReward(uint8_t star, bool isMulti);
	void randomSurpriseReward(bool isMulti);

	void checkAndSetTopStarRewardState(); 

private:
	void sendDailyTaskInfo();
	void sendDailyTaskStarInfo();
	void sendAcceptTaskSucess(TaskId taskId);

private:
	Role& m_owner;

private:
	TaskInfo m_taskInfo;
	std::vector<TaskTpl::Ptr> m_taskVec;	//可随机的日常任务
};


}

#endif
