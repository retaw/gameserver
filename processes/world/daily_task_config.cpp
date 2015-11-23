#include "daily_task_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

DailyTaskConfig DailyTaskConfig::m_me;

DailyTaskConfig& DailyTaskConfig::me()
{
	return m_me;
}

void DailyTaskConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/daily_task.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	m_dailyTaskCfg.load(root);
}

void DailyTaskConfig::DailyTask::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();

	needLevel = root.getChild("task_reward").getAttr<uint32_t>("needLevel");
	dailyTaskNum = root.getChild("task_reward").getAttr<uint32_t>("daily_task_num");
	multiNum = root.getChild("task_reward").getAttr<uint32_t>("multi_num");

	needMoneyFreshStar = root.getChild("star").getAttr<uint32_t>("fresh_star_needMoney");
	needMoneyTopStar = root.getChild("star").getAttr<uint32_t>("top_star_needMoney");
	needMoneyFinishAllTask = root.getChild("star").getAttr<uint32_t>("finish_all_task_needMoney");
	needMoneyMultiReward = root.getChild("star").getAttr<uint32_t>("multi_reward_needMoney");

	rewardId = root.getChild("surprise_reward").getAttr<uint32_t>("rewardId");
	multiRewardId = root.getChild("surprise_reward").getAttr<uint32_t>("multi_rewardId");


	auto strToList = [](std::vector<std::pair<uint32_t, uint32_t> >* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() != 2)
				continue;

			ret->push_back(std::make_pair(propItems[0], propItems[1]));
		}
	};

	for(XmlParseNode node = root.getChild("task_reward").getChild("item"); node; ++node)
	{
		TaskReward temp;
		temp.minLevel = node.getAttr<uint32_t>("minLevel");
		temp.maxLevel = node.getAttr<uint32_t>("maxLevel");
		strToList(&temp.rewardVec, node.getAttr<std::string>("reward_list"));

		taskRewardVec.push_back(temp);
	}
	
	for(XmlParseNode node = root.getChild("star").getChild("item"); node; ++node)
	{
		uint8_t star = node.getAttr<uint8_t>("star");
		uint32_t prob = node.getAttr<uint32_t>("prob");
		uint32_t percent = node.getAttr<uint32_t>("percent");

		starRewardMap.insert(std::make_pair(star, percent));
		for(uint32_t section = 0; section < prob; ++section)
		{
			starProbVec.push_back(star);
		}
	}

	for(XmlParseNode node = root.getChild("top_star_reward").getChild("item"); node; ++node)
	{
		uint32_t num = node.getAttr<uint32_t>("num");
		uint32_t rewardId = node.getAttr<uint32_t>("rewardId");

		topStarRewardMap.insert(std::make_pair(num, rewardId));
	}

	for(XmlParseNode node = root.getChild("surprise_reward").getChild("item"); node; ++node)
	{
		SurpriseReward temp;
		temp.day = node.getAttr<uint32_t>("day");
		temp.prob = node.getAttr<uint32_t>("prob");
		temp.multiProb = node.getAttr<uint32_t>("multiProb");

		surpriseRewardMap.insert(std::make_pair(temp.day, temp));
	}

	return;
}

void DailyTaskConfig::DailyTask::clear()
{
	taskRewardVec.clear();
	starProbVec.clear();
	starRewardMap.clear();
	topStarRewardMap.clear();
	surpriseRewardMap.clear();
}


}
