#include "zhuansheng_config.h"

#include "water/common/commdef.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace world{

ZhuanshengConfig ZhuanshengConfig::m_me;

ZhuanshengConfig& ZhuanshengConfig::me()
{
	return m_me;
}

void ZhuanshengConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/zhuansheng.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	zhuanshengCfg.load(root);
}

void ZhuanshengConfig::Zhuansheng::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();
	
	auto strToLimitList = [](std::vector<std::pair<LimitType, uint32_t> >* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() != 2)
				continue;

			ret->push_back(std::make_pair(static_cast<LimitType>(propItems[0]), propItems[1]));
		}
	};

	auto strToPropList = [](std::vector<std::pair<PropertyType, uint32_t> >* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() != 2)
				continue;

			ret->push_back(std::make_pair(static_cast<PropertyType>(propItems[0]), propItems[1]));
		}
	};

	//转生条件
	for(XmlParseNode node = root.getChild("base").getChild("item"); node; ++node)
	{
		BaseItem temp;
		temp.level = node.getAttr<uint8_t>("level");
		temp.needMoneyType = static_cast<MoneyType>(node.getAttr<uint8_t>("needMoneyType"));
		temp.needMoneyNum = node.getAttr<uint32_t>("needMoneyNum");
		strToLimitList(&temp.limitVec, node.getAttr<std::string>("limit_list"));
		
		m_baseMap.insert(std::make_pair(temp.level, temp));
	}

	//角色转生等级属性加成
	for(XmlParseNode node = root.getChild("reward_role").getChild("item"); node; ++node)
	{
		Job job = static_cast<Job>(node.getAttr<uint8_t>("job"));
		RewardRole temp;
		temp.level = node.getAttr<uint8_t>("level");
		strToPropList(&temp.rewardPropVec, node.getAttr<std::string>("reward_prop_list"));

		m_roleRewardMap[job].insert(std::make_pair(temp.level, temp));
	}

	//英雄转生等级属性加成
	for(XmlParseNode node = root.getChild("reward_hero").getChild("item"); node; ++node)
	{
		Job job = static_cast<Job>(node.getAttr<uint8_t>("job"));
		RewardHero temp;
		temp.level = node.getAttr<uint8_t>("level");
		strToPropList(&temp.rewardPropVec, node.getAttr<std::string>("reward_prop_list"));

		m_heroRewardMap[job].insert(std::make_pair(temp.level, temp));
	}

	return;
}

void ZhuanshengConfig::Zhuansheng::clear()
{
	m_baseMap.clear();	
	m_roleRewardMap.clear();
	m_heroRewardMap.clear();
}

}
