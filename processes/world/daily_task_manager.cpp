#include "daily_task_manager.h"
#include "role_manager.h"
#include "world.h"

#include "protocol/rawmsg/public/daily_task.h"
#include "protocol/rawmsg/public/daily_task.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

DailyTaskManager::DailyTaskManager()
{
}

DailyTaskManager DailyTaskManager::m_me;

DailyTaskManager& DailyTaskManager::me()
{
	return m_me;
}


void DailyTaskManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestDailyTaskInfo, std::bind(&DailyTaskManager::clientmsg_RequestDailyTaskInfo, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestAcceptDailyTask, std::bind(&DailyTaskManager::clientmsg_RequestAcceptDailyTask, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(ReuqestRefreshDailyTaskStar, std::bind(&DailyTaskManager::clientmsg_ReuqestRefreshDailyTaskStar, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestDailyTaskTopStar, std::bind(&DailyTaskManager::clientmsg_RequestDailyTaskTopStar, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestGetDailyTaskReward, std::bind(&DailyTaskManager::clientmsg_RequestGetDailyTaskReward, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestFinishAllDailyTask, std::bind(&DailyTaskManager::clientmsg_RequestFinishAllDailyTask, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestGetTopStarTaskNumReward, std::bind(&DailyTaskManager::clientmsg_RequestGetTopStarTaskNumReward, this, _1, _2, _3));
}

//请求日常任务信息
void DailyTaskManager::clientmsg_RequestDailyTaskInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestDailyTaskInfo*>(msgData);
	if(!rev)
		return;

	role->m_dailyTask.requestDailyTaskInfo();
	return;
}

//请求接受日常任务
void DailyTaskManager::clientmsg_RequestAcceptDailyTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestAcceptDailyTask*>(msgData);
	if(!rev)
		return;

	role->m_dailyTask.requestAcceptTask(rev->taskId);
	return;
}

//请求刷新任务星级
void DailyTaskManager::clientmsg_ReuqestRefreshDailyTaskStar(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::ReuqestRefreshDailyTaskStar*>(msgData);
	if(!rev)
		return;

	role->m_dailyTask.requestRefreshDailyTaskStar();
	return;
}

//请求一键满星
void DailyTaskManager::clientmsg_RequestDailyTaskTopStar(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestDailyTaskTopStar*>(msgData);
	if(!rev)
		return;

	role->m_dailyTask.requestDailyTaskTopStar();
	return;
}

//请求领取奖励
void DailyTaskManager::clientmsg_RequestGetDailyTaskReward(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestGetDailyTaskReward*>(msgData);
	if(!rev)
		return;

	role->m_dailyTask.requestGetDailyTaskReward(rev->isMulti);
	return;
}

//请求一键全部完成任务
void DailyTaskManager::clientmsg_RequestFinishAllDailyTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestFinishAllDailyTask*>(msgData);
	if(!rev)
		return;

	role->m_dailyTask.requestFinishAllDailyTask(rev->isMulti);
	return;
}

//请求领取满星任务奖励
void DailyTaskManager::clientmsg_RequestGetTopStarTaskNumReward(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestGetTopStarTaskNumReward*>(msgData);
	if(!rev)
		return;

	role->m_dailyTask.requestGetTopStarTaskNumReward(rev->num);
	return;
}

}
