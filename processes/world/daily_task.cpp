#include "daily_task.h"
#include "daily_task_config.h"
#include "role.h"
#include "role_manager.h"
#include "role_counter.h"
#include "reward_manager.h"
#include "mail_manager.h"

#include "water/componet/logger.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/daily_task.h"
#include "protocol/rawmsg/public/daily_task.codedef.public.h"

#include "protocol/rawmsg/private/exp_area.h"
#include "protocol/rawmsg/private/exp_area.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

DailyTask::DailyTask(Role& owner)
: m_owner(owner)
{
}

void DailyTask::loadFromDB(const std::vector<TaskInfo>& taskInfoVec)
{
	if(taskInfoVec.empty())
	{
		refreshTask();
		return;
	}
	else if(taskInfoVec.size() != 1)
	{
		for(const auto& iter : taskInfoVec)
		{
			m_owner.m_roleTask.changeTaskState(iter.taskId, TaskState::quit);
		}
		refreshTask();
		return;
	}

	m_taskInfo = taskInfoVec[0];

	//判断是否同一天
	if(!componet::inSameDay(Clock::from_time_t(m_taskInfo.time), Clock::now()))
	{
		dealNotSameDay();
		return;
	}
	
	return;
}

void DailyTask::dealNotSameDay()
{
	//放弃正在进行任务
	if(0 != m_taskInfo.taskId)
	{
		m_owner.m_roleTask.changeTaskState(m_taskInfo.taskId, TaskState::quit);
	}

	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	uint32_t count = m_owner.m_roleCounter.get(CounterType::dailyTaskCount);
	if(count != cfg.dailyTaskNum)
	{
		m_owner.m_roleCounter.clear(CounterType::finishAllTaskDayCount);
	}
	else
	{
		//检查连续完成所有日常任务天数，是否超过一天，超过则清零
		uint32_t nextTime = m_taskInfo.time + 24 * 60* 60;
		if(!componet::inSameDay(Clock::from_time_t(nextTime), Clock::now()))
		{
			m_owner.m_roleCounter.clear(CounterType::finishAllTaskDayCount);
		}
	}


	m_owner.m_roleCounter.clear(CounterType::dailyTaskCount);
	m_owner.m_roleCounter.clear(CounterType::finishTopStarTaskNum);
	m_owner.m_roleTask.clearDailyTaskTopStarReward();

	//刷新新任务
	refreshTask();
	sendDailyTaskInfo();
	m_owner.m_roleTask.refreshTaskState(m_taskInfo.taskId, m_taskInfo.state);
	return;
}

void DailyTask::finishTask(TaskId taskId)
{
	if(0 == taskId)
		return;

	if(m_taskInfo.taskId != taskId)
		return;

	m_taskInfo.state = TaskState::finished;

	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(m_taskInfo.star == cfg.starRewardMap.size())
	{
		m_owner.m_roleCounter.add(CounterType::finishTopStarTaskNum, 1);
		checkAndSetTopStarRewardState();
	}
	if(m_owner.m_roleCounter.get(CounterType::dailyTaskCount) == cfg.dailyTaskNum 
	   && m_owner.m_roleCounter.get(CounterType::finishAllTaskDayCount) < 5)
	{
		m_owner.m_roleCounter.add(CounterType::finishAllTaskDayCount, 1);
	}
	return;
}

uint32_t DailyTask::getDailyTaskTime(TaskId taskId) const
{
	if(m_taskInfo.taskId != taskId)
		return 0;

	return m_taskInfo.time;
}

uint8_t DailyTask::getDailyTaskStar(TaskId taskId) const
{
	if(m_taskInfo.taskId != taskId)
		return 0;

	return m_taskInfo.star;
}

void DailyTask::roleLevelUp()
{
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(cfg.needLevel > m_owner.level())
		return;
	
	if(0 != m_taskInfo.taskId)	
		return;

	refreshTask();
	m_owner.m_roleTask.refreshTaskState(m_taskInfo.taskId, m_taskInfo.state);
	return;
}

void DailyTask::afterEnterScene()
{
	if(0 == m_taskInfo.taskId)
		return;

	if(m_taskInfo.state != TaskState::acceptable)
		return;

	m_owner.m_roleTask.refreshTaskState(m_taskInfo.taskId, m_taskInfo.state);    
	return;
}

void DailyTask::requestDailyTaskInfo()
{
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(cfg.needLevel > m_owner.level())
		return;
	
	sendDailyTaskInfo();
	sendDailyTaskStarInfo();
	return;
}

void DailyTask::requestAcceptTask(TaskId taskId)
{
	if(m_taskInfo.taskId != taskId)
	{
		LOG_ERROR("日常任务, 接受任务, 认证失败, name={}, roleId={}, taskId=[{},{}]",
				  m_owner.name(), m_owner.id(), m_taskInfo.taskId, taskId);
		return;
	}

	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(m_owner.m_roleCounter.get(CounterType::dailyTaskCount) > cfg.dailyTaskNum)
	{
		m_owner.sendSysChat("今日日常任务已完成");
		return;
	}

	if(m_taskInfo.state != TaskState::acceptable)
		return;

	m_taskInfo.state = TaskState::accepted;
	m_owner.m_roleTask.changeTaskState(m_taskInfo.taskId, TaskState::accepted);	
	sendAcceptTaskSucess(taskId);
	return;
}

void DailyTask::requestRefreshDailyTaskStar()
{
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(cfg.starProbVec.empty())
		return;

	if(m_owner.m_roleCounter.get(CounterType::dailyTaskCount) > cfg.dailyTaskNum)
		return;

	if(m_taskInfo.state != TaskState::acceptable)
		return;

	if(!m_owner.reduceMoney(MoneyType::money_1, cfg.needMoneyFreshStar, "日常任务刷星"))
		return;

	componet::Random<uint32_t> rand(0, cfg.starProbVec.size() - 1); 
	uint8_t newStar = cfg.starProbVec[rand.get()];
	m_taskInfo.star = newStar;
	m_owner.m_roleTask.setDailyTaskStar(m_taskInfo.taskId, m_taskInfo.star);
	sendDailyTaskStarInfo();
	return;
}

void DailyTask::requestDailyTaskTopStar()
{
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(m_owner.m_roleCounter.get(CounterType::dailyTaskCount) > cfg.dailyTaskNum)
		return;

	if(m_taskInfo.state != TaskState::acceptable)
		return;

	if(!m_owner.reduceMoney(MoneyType::money_4, cfg.needMoneyTopStar, "日常任务一键满星"))
		return;

	uint8_t topStar = cfg.starRewardMap.size();
	m_taskInfo.star = topStar;
	m_owner.m_roleTask.setDailyTaskStar(m_taskInfo.taskId, m_taskInfo.star);
	sendDailyTaskStarInfo();
	return;
}

void DailyTask::requestGetDailyTaskReward(bool isMulti)
{
	if(m_taskInfo.state != TaskState::finished)
	{
		m_owner.sendSysChat("不可领取奖励");
		return;
	}

	if(isMulti)
	{
		const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
		if(!m_owner.reduceMoney(MoneyType::money_4, cfg.needMoneyMultiReward, "日常任务多倍领取奖励"))
			return;
	}

	//发放任务奖励
	giveOutTaskReward(m_taskInfo.star, isMulti);

	//随机惊喜奖励
	randomSurpriseReward(isMulti);

	//领取奖励后将此任务设置为完成并提交, 必须放在refreshTask之前
	m_owner.m_roleTask.changeTaskState(m_taskInfo.taskId, TaskState::over);
	m_taskInfo.state = TaskState::over;
	
	refreshTask();
	sendDailyTaskInfo();
	sendDailyTaskStarInfo();
	m_owner.sendSysChat("奖励领取成功");
	
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(m_owner.m_roleCounter.get(CounterType::dailyTaskCount) <= cfg.dailyTaskNum)
	{
		m_owner.m_roleTask.refreshTaskState(m_taskInfo.taskId, m_taskInfo.state);
	}
	return;
}

void DailyTask::requestFinishAllDailyTask(bool isMulti)
{
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	uint32_t taskCount = m_owner.m_roleCounter.get(CounterType::dailyTaskCount);
	if(taskCount > cfg.dailyTaskNum)
		return;

	uint32_t notFinishedNum = cfg.dailyTaskNum - taskCount + 1;
	if(isMulti)
	{
		uint32_t needYuanbao = cfg.needMoneyFinishAllTask + notFinishedNum * cfg.needMoneyMultiReward; 
		if(!m_owner.reduceMoney(MoneyType::money_4, needYuanbao, "日常任务一键完成且多倍领取"))
			return;
	}
	else
	{
		if(!m_owner.reduceMoney(MoneyType::money_4, cfg.needMoneyFinishAllTask, "日常任务一键完成"))
			return;
	}

	uint8_t topStar = cfg.starRewardMap.size();
	for(uint32_t i = 0; i < notFinishedNum; ++i)
	{
		//发放任务奖励
		giveOutTaskReward(topStar, isMulti);

		//随机惊喜奖励
		randomSurpriseReward(isMulti);

		//完成满星任务数
		m_owner.m_roleCounter.add(CounterType::finishTopStarTaskNum, 1);
		checkAndSetTopStarRewardState();
	}

	m_owner.m_roleCounter.set(CounterType::dailyTaskCount, cfg.dailyTaskNum);
	if(m_owner.m_roleCounter.get(CounterType::finishAllTaskDayCount) < 5)
	{
		m_owner.m_roleCounter.add(CounterType::finishAllTaskDayCount, 1);
	}

	m_taskInfo.state = TaskState::over;
	m_owner.m_roleTask.changeTaskState(m_taskInfo.taskId, TaskState::quit);
	sendDailyTaskInfo();
	return;
}

void DailyTask::requestGetTopStarTaskNumReward(uint32_t num)
{
	if(num > m_owner.m_roleCounter.get(CounterType::finishTopStarTaskNum))
		return;

	const auto& rewardStateMap = m_owner.m_roleTask.getDailyTaskTopStarReward();
	auto pos = rewardStateMap.find(num);
	if(pos == rewardStateMap.end())
		return;

	if(pos->second != Reward::canGet)
		return;

	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	auto iter = cfg.topStarRewardMap.find(num);
	if(iter == cfg.topStarRewardMap.end())
		return;

	std::vector<ObjItem> objVec;
	if(!RewardManager::me().getRandomReward(iter->second, 1, m_owner.level(), m_owner.job(), objVec))
	{
		LOG_ERROR("日常任务, 获取随机奖励失败, 领取满星奖励失败, name={}, roleId={}, level={}, job={}, rewardId={}",
				  m_owner.name(), m_owner.id(), m_owner.level(), m_owner.job(), iter->second);
		return;
	}

	if(!m_owner.checkPutObj(objVec))
	{
		m_owner.sendSysChat("背包空间不足");
		return;
	}
	m_owner.putObj(objVec);
	m_owner.m_roleTask.setDailyTaskTopStarRewardState(num, Reward::got);
	sendDailyTaskInfo();
	m_owner.sendSysChat("奖励领取成功");
	return;
}

void DailyTask::refreshTask()
{
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(cfg.needLevel > m_owner.level())
		return;

	if(m_owner.m_roleCounter.get(CounterType::dailyTaskCount) >= cfg.dailyTaskNum)
		return;

	if(m_taskVec.empty())
	{
		uint32_t level = m_owner.level();
		const auto& taskMap = TaskBase::me().getTaskCfg();
		for(const auto& iter : taskMap)
		{
			if(iter.second == nullptr)
				continue;

			if(iter.second->type != TaskType::daily)
				continue;

			if(level >= iter.second->minLevel && level <= iter.second->maxLevel)
			{
				m_taskVec.push_back(iter.second);
			}
		}
	}

	if(m_taskVec.empty())
	{
		LOG_ERROR("日常任务, 配置错误, 没有配日常任务, name={}, roleId={}",
				  m_owner.name(), m_owner.id());
		return;
	}

	componet::Random<uint32_t> rand(0, m_taskVec.size() - 1); 
	auto& taskPtr = m_taskVec[rand.get()];  
	if(taskPtr == nullptr)
		return;
	
	m_taskInfo.taskId = taskPtr->taskId;
	m_taskInfo.type = TaskType::daily;
	m_taskInfo.state = TaskState::acceptable; 
	m_taskInfo.time = componet::toUnixTime(Clock::now());
	m_taskInfo.star = randomTaskStar();
	m_owner.m_roleCounter.add(CounterType::dailyTaskCount, 1);
	m_owner.m_roleTask.changeTaskState(m_taskInfo.taskId, TaskState::acceptable);
	return;
}

uint8_t DailyTask::randomTaskStar()
{
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(cfg.starProbVec.empty())
		return 0;

	componet::Random<uint32_t> rand(0, cfg.starProbVec.size() - 1);
	return cfg.starProbVec[rand.get()];
}

uint32_t DailyTask::getStarPercent(uint8_t star)
{
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	auto pos = cfg.starRewardMap.find(star);
	if(pos == cfg.starRewardMap.end())
		return 0;

	return pos->second;
}

std::vector<std::pair<uint32_t, uint32_t> > DailyTask::getTaskReward(uint32_t level)
{
	std::vector<std::pair<uint32_t, uint32_t> > rewardVec;
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	for(auto iter = cfg.taskRewardVec.begin(); iter != cfg.taskRewardVec.end(); ++iter)
	{
		if(level >= iter->minLevel && level <= iter->maxLevel)
		{
			rewardVec = iter->rewardVec;
			break;
		}
	}

	return rewardVec;
}

void DailyTask::giveOutTaskReward(uint8_t star, bool isMulti)
{
	uint32_t multiNum = 1;
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	if(isMulti)
	{
		multiNum = cfg.multiNum;
	}

	const auto& rewardVec = getTaskReward(m_owner.level());
	if(rewardVec.empty())
	{
		LOG_ERROR("日常任务, 获取任务奖励失败, 发放奖励失败, name={}, roleId={}, taskId",
				  m_owner.name(), m_owner.id(), m_taskInfo.taskId);
		return;
	}

	uint32_t percent = getStarPercent(star);
	for(auto iter = rewardVec.begin(); iter != rewardVec.end(); ++iter)
	{
		uint32_t num = SAFE_DIV(iter->second * percent * multiNum, 100);
		m_owner.putObj(iter->first, num, Bind::none);
	}

	return;
}

void DailyTask::randomSurpriseReward(bool isMulti)
{
	uint16_t prob = 0;
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	uint32_t day = m_owner.m_roleCounter.get(CounterType::finishAllTaskDayCount);
	auto pos = cfg.surpriseRewardMap.find(day);
	if(pos != cfg.surpriseRewardMap.end())
	{
		if(isMulti)
		{
			prob = pos->second.multiProb;
		}
		else
		{
			prob = pos->second.prob;
		}
	}

	componet::Random<uint32_t> rand(1, 100);
	if(prob < rand.get())
		return;

	uint32_t rewardId = 0;
	if(isMulti)
		rewardId = cfg.multiRewardId;
	else
		rewardId = cfg.rewardId;

	std::vector<ObjItem> objVec;
	if(!RewardManager::me().getRandomReward(rewardId, 1, m_owner.level(), m_owner.job(), objVec))
	{
		LOG_ERROR("日常任务, 获取随机奖励失败, 发放惊喜奖励失败, name={}, roleId={}, level={}, job={}, rewardId={}",
				  m_owner.name(), m_owner.id(), m_owner.level(), m_owner.job(), rewardId);
		return;
	}

	if(objVec.empty())
		return;

	if(!m_owner.checkPutObj(objVec))
	{
		std::string text = "由于背包空间不足, 通过邮件发放日常任务惊喜奖励, 请注意查收";
		MailManager::me().send(m_owner.id(), "日常任务惊喜奖励", text, objVec);
		return;
	}

	m_owner.putObj(objVec);
	return;
}

void DailyTask::checkAndSetTopStarRewardState()
{
	uint32_t num = m_owner.m_roleCounter.get(CounterType::finishTopStarTaskNum);
	const auto& cfg = DailyTaskConfig::me().m_dailyTaskCfg;
	auto pos = cfg.topStarRewardMap.find(num);
	if(pos == cfg.topStarRewardMap.end())
		return;

	if(0 != SAFE_MOD(num, 5))
		return;

	const auto& rewardStateMap = m_owner.m_roleTask.getDailyTaskTopStarReward();
	if(rewardStateMap.find(num) != rewardStateMap.end())
		return;

	m_owner.m_roleTask.setDailyTaskTopStarRewardState(num, Reward::canGet);
	return;
}

void DailyTask::sendDailyTaskInfo()
{
	std::vector<uint8_t> buf;
	buf.reserve(256);
	buf.resize(sizeof(PublicRaw::RetDailyTaskInfo));

	auto* msg = reinterpret_cast<PublicRaw::RetDailyTaskInfo*>(buf.data());
	msg->taskId = m_taskInfo.taskId;
	msg->state = m_taskInfo.state;
	msg->day = m_owner.m_roleCounter.get(CounterType::finishAllTaskDayCount);
	msg->finishTopStarNum = m_owner.m_roleCounter.get(CounterType::finishTopStarTaskNum);
	msg->count = m_owner.m_roleCounter.get(CounterType::dailyTaskCount);
	
	msg->size = 0;
	const auto& rewardStateMap = m_owner.m_roleTask.getDailyTaskTopStarReward();
	for(auto pos = rewardStateMap.begin(); pos != rewardStateMap.end(); ++pos)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetDailyTaskInfo::TopStarReward));
		auto* msg  = reinterpret_cast<PublicRaw::RetDailyTaskInfo*>(buf.data());

		msg->data[msg->size].num = pos->first;
		msg->data[msg->size].rewardState = pos->second;
		
		++msg->size;
	}

	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetDailyTaskInfo), buf.data(), buf.size());		
	return;
}

void DailyTask::sendDailyTaskStarInfo()
{
	std::vector<uint8_t> buf;
	buf.reserve(128);
	buf.resize(sizeof(PublicRaw::RetDailyTaskInfo));

	auto* msg = reinterpret_cast<PublicRaw::RetDailyTaskStarInfo*>(buf.data());
	msg->taskId = m_taskInfo.taskId;
	msg->star = m_taskInfo.star;
	
	msg->size = 0;

	uint32_t percent = getStarPercent(m_taskInfo.star);
	const auto& rewardVec = getTaskReward(m_owner.level());
	for(auto iter = rewardVec.begin(); iter != rewardVec.end(); ++iter)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetDailyTaskStarInfo::RewardItem));
		auto* msg  = reinterpret_cast<PublicRaw::RetDailyTaskStarInfo*>(buf.data());

		uint32_t num = SAFE_DIV(iter->second * percent, 100);
		
		msg->data[msg->size].tplId = iter->first;
		msg->data[msg->size].num = num;

		++msg->size;
	}

	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetDailyTaskStarInfo), buf.data(), buf.size());		
	return;
}

void DailyTask::sendAcceptTaskSucess(TaskId taskId)
{
	if(m_taskInfo.taskId != taskId)
		return;

	if(m_taskInfo.state != TaskState::accepted)
		return;

	PublicRaw::RetAcceptDailyTaskSucess send;
	send.taskId = taskId;
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetAcceptDailyTaskSucess), &send, sizeof(send));
	return;
}


}
