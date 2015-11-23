#include "reward_random_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

RewardRandomConfig RewardRandomConfig::m_me;

RewardRandomConfig& RewardRandomConfig::me()
{
	return m_me;
}

void RewardRandomConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/reward_random.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	m_rewardRandomCfg.load(root);
}

void RewardRandomConfig::RewardRandom::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();

	auto strToProbList = [](std::vector<uint32_t>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() < 2)
				continue;

			const uint32_t section = propItems[1];
			for(uint32_t i = 0; i < section; ++i)
			{
				ret->push_back(propItems[0]);
			}
		}
	};

	//礼包池配置(礼包池的内容是具体的奖励物品或数值配置)
	for(XmlParseNode nodePool = root.getChild("reward_pool_list").getChild("pool"); nodePool; ++nodePool)
	{
		const uint32_t poolId = nodePool.getAttr<uint32_t>("poolId");	
		if(poolMap.find(poolId) != poolMap.end())
		{
			LOG_ERROR("奖励, 随机奖励, 配置错误! poolId重复! poolId={}", poolId);
			return;
		}

		for(XmlParseNode node = nodePool.getChild("item"); node; ++node)
		{
			RewardData temp;
			temp.tplId = node.getAttr<uint32_t>("tplId");
			temp.num = node.getAttr<uint16_t>("num");
			temp.bind = static_cast<Bind>(node.getAttr<uint8_t>("bind"));
			temp.job = node.getAttr<uint8_t>("job");
			temp.minLevel = node.getAttr<uint32_t>("minLevel");
			temp.maxLevel = node.getAttr<uint32_t>("maxLevel");

			poolMap[poolId].push_back(temp);		
		}
	}

	//礼包配置(礼包的内容是礼包池ID)
	for(XmlParseNode nodeGift = root.getChild("reward_list").getChild("reward"); nodeGift; ++nodeGift)
	{
		const uint32_t rewardId = nodeGift.getAttr<uint32_t>("id");	
		if(rewardMap.find(rewardId) != rewardMap.end())
		{
			LOG_ERROR("奖励, 随机奖励, 配置错误! id重复! id={}", rewardId);
			return;
		}
	
		for(XmlParseNode node = nodeGift.getChild("item"); node; ++node)
		{
			std::vector<uint32_t> poolVec;
			strToProbList(&poolVec, node.getAttr<std::string>("pool_prob_list"));

			rewardMap[rewardId].push_back(poolVec);
		}
	}

	return;
}

void RewardRandomConfig::RewardRandom::clear()
{
	rewardMap.clear();
	poolMap.clear();
}

}
