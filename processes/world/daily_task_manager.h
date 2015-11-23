/*
 * Author: zhupengfei
 *
 * Created: 2015-10-27 17:50 +0800
 *
 * Modified: 2015-10-27 17:50 +0800
 *
 * Description: 处理日常任务相关消息
 */

#ifndef PROCESS_WORLD_DAILY_TASK_MANAGER_HPP
#define PROCESS_WORLD_DAILY_TASK_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>

namespace world{

using namespace water;
using water::process::ProcessIdentity;


class DailyTaskManager
{
public:
	DailyTaskManager();
	~DailyTaskManager() = default;
    static DailyTaskManager& me();
private:
	static DailyTaskManager m_me;

public:
    void regMsgHandler();

private:
	//请求日常任务信息
	void clientmsg_RequestDailyTaskInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求接受日常任务 
	void clientmsg_RequestAcceptDailyTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求刷新任务星级
	void clientmsg_ReuqestRefreshDailyTaskStar(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求一键满星
	void clientmsg_RequestDailyTaskTopStar(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求领取奖励
	void clientmsg_RequestGetDailyTaskReward(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求一键全部完成任务
	void clientmsg_RequestFinishAllDailyTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求领取满星任务奖励
	void clientmsg_RequestGetTopStarTaskNumReward(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

};


}

#endif
