/*
 * Author: zhupengfei
 *
 * Created: 2015-10-27 14::25:00 +0800
 *
 * Modified: 2015-10-27 14::25:00 +0800
 *
 * Description: 加载每日任务限制条件及奖励配置文件
 */

#ifndef PROCESS_WORLD_DAILY_TASK_CONFIG_HPP
#define PROCESS_WORLD_DAILY_TASK_CONFIG_HPP

#include "water/common/roledef.h"
#include "water/componet/xmlparse.h"

#include <vector>
#include <unordered_map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class DailyTaskConfig
{
public:
	~DailyTaskConfig() = default;
	static DailyTaskConfig& me();
private:
	static DailyTaskConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct DailyTask
	{
		void load(componet::XmlParseNode root);
		void clear();

		uint32_t needLevel = 0;
		uint32_t dailyTaskNum = 0;
		uint32_t multiNum = 0;

		uint32_t needMoneyFreshStar = 0;
		uint32_t needMoneyTopStar = 0;
		uint32_t needMoneyFinishAllTask = 0;
		uint32_t needMoneyMultiReward = 0;

		uint32_t rewardId = 0;
		uint32_t multiRewardId = 0;

		struct TaskReward
		{
			uint32_t minLevel;
			uint32_t maxLevel;
			std::vector<std::pair<uint32_t, uint32_t> > rewardVec;	//<TplId, num>
		};

		struct SurpriseReward
		{
			uint32_t day;
			uint32_t prob;
			uint32_t multiProb;
		};

		std::vector<TaskReward> taskRewardVec; 
		std::vector<uint8_t> starProbVec;						//<star> 刷星级概率
		std::unordered_map<uint8_t, uint32_t> starRewardMap;	//<star, percent> 此星级对应的奖励加成
		std::unordered_map<uint32_t, uint32_t> topStarRewardMap; //<num, rewardId> 完成满星任务数量对应的奖励
		std::unordered_map<uint32_t, SurpriseReward> surpriseRewardMap; 

	} m_dailyTaskCfg;
};


}

#endif

