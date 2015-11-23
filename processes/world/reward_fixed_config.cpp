#include "reward_fixed_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

RewardFixedConfig RewardFixedConfig::m_me;

RewardFixedConfig& RewardFixedConfig::me()
{
	return m_me;
}

void RewardFixedConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/reward_fixed.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	m_fixRewardCfg.load(root);
}

void RewardFixedConfig::Reward::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();
	
	for(XmlParseNode nodeGift = root.getChild("reward"); nodeGift; ++nodeGift)
	{
		const uint32_t rewardId = nodeGift.getAttr<uint32_t>("id");	
		if(rewardMap.find(rewardId) != rewardMap.end())
		{
			LOG_ERROR("奖励, 固定奖励, 配置错误! id重复! id={}", rewardId);
			return;
		}

		for(XmlParseNode node = nodeGift.getChild("item"); node; ++node)
		{
			RewardData temp;
			temp.tplId = node.getAttr<uint32_t>("tplId");
			temp.num = node.getAttr<uint16_t>("num");
			temp.bind = static_cast<Bind>(node.getAttr<uint8_t>("bind"));
			temp.job = node.getAttr<uint8_t>("job");
			temp.minLevel = node.getAttr<uint32_t>("minLevel");
			temp.maxLevel = node.getAttr<uint32_t>("maxLevel");

			rewardMap[rewardId].push_back(temp);		
		}
	}

	return;
}

void RewardFixedConfig::Reward::clear()
{
	rewardMap.clear();
}

}
