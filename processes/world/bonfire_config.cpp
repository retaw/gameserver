#include "bonfire_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

BonfireConfig BonfireConfig::m_me;

BonfireConfig& BonfireConfig::me()
{
	return m_me;
}

void BonfireConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/bonfire.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	bonfireCfg.load(root);
}

void BonfireConfig::Bonfire::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();

	drinkRewardId = root.getChild("drink").getAttr<uint32_t>("drink_rewardId");
	for(XmlParseNode nodeBonfire = root.getChild("reward").getChild("bonfire"); nodeBonfire; ++nodeBonfire)
	{
		uint32_t tplId = nodeBonfire.getAttr<uint32_t>("tplId");
		uint32_t needLevel = nodeBonfire.getAttr<uint32_t>("needLevel");
		m_levelMap[tplId] = needLevel;

		for(XmlParseNode node = nodeBonfire.getChild("item"); node; ++node)
		{
			RewardItem temp;
			temp.minLevel = node.getAttr<uint32_t>("minLevel");
			temp.maxLevel = node.getAttr<uint32_t>("maxLevel");
			temp.addExp = node.getAttr<uint32_t>("addExp");

			m_rewardMap[tplId].push_back(temp);
		}
	}
	
	for(XmlParseNode node = root.getChild("team").getChild("item"); node; ++node)
	{
		uint8_t count = node.getAttr<uint8_t>("count");
		uint32_t percent = node.getAttr<uint32_t>("percent");

		m_teamMap.insert(std::make_pair(count, percent));
	}

	for(XmlParseNode node = root.getChild("drink").getChild("item"); node; ++node)
	{
		DrinkItem temp;
		temp.type = node.getAttr<uint8_t>("type");
		temp.percent = node.getAttr<uint32_t>("percent");
		temp.name = node.getAttr<std::string>("name");

		m_drinkMap.insert(std::make_pair(temp.type, temp));
	}

	for(XmlParseNode node = root.getChild("vip").getChild("item"); node; ++node)
	{
		uint32_t level = node.getAttr<uint8_t>("level");
		uint32_t limitCount = node.getAttr<uint32_t>("limit_count");

		m_vipMap.insert(std::make_pair(level, limitCount));
	}

	return;
}

void BonfireConfig::Bonfire::clear()
{
	m_levelMap.clear();
	m_rewardMap.clear();
	m_teamMap.clear();
	m_drinkMap.clear();
	m_vipMap.clear();
}


}
