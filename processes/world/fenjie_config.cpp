#include "fenjie_config.h"

#include "water/common/commdef.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace world{

FenjieConfig FenjieConfig::m_me;

FenjieConfig& FenjieConfig::me()
{
	return m_me;
}

void FenjieConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/fenjie.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	fenjieCfg.load(root);
}

void FenjieConfig::Fenjie::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();
	
	auto parseStrList = [](std::vector<RewardItem>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() != 3)
				continue;

			RewardItem temp;
			temp.tplId = propItems[0];
			temp.num = propItems[1];
			temp.bind = static_cast<Bind>(propItems[2]);

			ret->push_back(temp);
		}
	};

	//强化装备分解产出
	for(XmlParseNode node = root.getChild("strong").getChild("item"); node; ++node)
	{
		uint8_t level = node.getAttr<uint8_t>("level");

		std::vector<RewardItem> rewardVec;
		parseStrList(&rewardVec, node.getAttr<std::string>("reward_list"));
		
		m_strongRewardMap.insert(std::make_pair(level, rewardVec));
	}

	//武器幸运分解产出
	for(XmlParseNode node = root.getChild("reward_role").getChild("item"); node; ++node)
	{
		uint8_t level = node.getAttr<uint8_t>("level");

		std::vector<RewardItem> rewardVec;
		parseStrList(&rewardVec, node.getAttr<std::string>("reward_list"));
		
		m_luckyRewardMap.insert(std::make_pair(level, rewardVec));
	}

	return;
}

void FenjieConfig::Fenjie::clear()
{
	m_strongRewardMap.clear();	
	m_luckyRewardMap.clear();
}

}
