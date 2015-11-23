/*
 * Author: zhupengfei
 *
 * Created: 2015-10-27 17:30 +0800
 *
 * Modified: 2015-10-27 17:30 +0800
 *
 * Description: 日常任务相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_DAILY_TASK_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_DAILY_TASK_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求日常任务信息
struct RequestDailyTaskInfo
{
};

//s -> c
//返回日常任务信息
struct RetDailyTaskInfo
{
	TaskId taskId;
	TaskState state;
	uint32_t day;				//连续完成日常任务的天数
	uint32_t finishTopStarNum;	//完成满星任务的数量
	uint32_t count;				//第几个任务

	ArraySize size = 0;
	struct TopStarReward
	{
		uint16_t num;			//满星任务数
		Reward rewardState;		//0没有奖励 1可领取 2已领取
	} data[0];
};

//s -> c
//返回日常任务星级及对应奖励信息
struct RetDailyTaskStarInfo
{
	TaskId taskId;
	uint8_t star;

	ArraySize size;
	struct RewardItem
	{
		TplId tplId;
		uint32_t num;
	} data[0];
};

//c -> s
//请求接受日常任务
struct RequestAcceptDailyTask
{
	TaskId taskId;
};

//s -> c
//返回日常任务接受成功
struct RetAcceptDailyTaskSucess
{
	TaskId taskId;
};

//c -> s
//请求刷新任务星级
struct ReuqestRefreshDailyTaskStar
{
};

//c -> s
//请求一键满星
struct RequestDailyTaskTopStar
{
};

//c -> s
//请求领取奖励
struct RequestGetDailyTaskReward
{
	bool isMulti = false;	//是否多倍领取
};

//c -> s
//请求一键全部完成任务
struct RequestFinishAllDailyTask
{
	bool isMulti = false;
};

//c -> s
//请求领取满星任务奖励
struct RequestGetTopStarTaskNumReward
{
	uint32_t num;	//N个满星任务
};


}

#pragma pack()


#endif
