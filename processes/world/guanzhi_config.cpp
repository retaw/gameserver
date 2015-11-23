#include "guanzhi_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

GuanzhiConfig GuanzhiConfig::m_me;

GuanzhiConfig& GuanzhiConfig::me()
{
	return m_me;
}

void GuanzhiConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/guanzhi.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	guanzhiCfg.load(root);
}

void GuanzhiConfig::Guanzhi::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	m_guanzhiMap.clear();

	auto strToProbList = [](std::vector<std::pair<PropertyType, uint32_t> >* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() < 2)
				continue;

			ret->push_back(std::make_pair(static_cast<PropertyType>(propItems[0]), propItems[1]));
		}
	};

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		GuanzhiItem temp;
		temp.level = node.getChildNodeText<uint8_t>("level");
		temp.name = node.getChildNodeText<std::string>("name");
		temp.titleId = node.getChildNodeText<uint32_t>("titleId");
		temp.needLevel = node.getChildNodeText<uint32_t>("needLevel");
		temp.needZhangong = node.getChildNodeText<uint64_t>("needZhangong");
		temp.needGold = node.getChildNodeText<uint32_t>("needGold");
		temp.dailyRewardId = node.getChildNodeText<uint32_t>("dailyRewardId");
		temp.levelUpRewardId = node.getChildNodeText<uint32_t>("levelUpRewardId");
		strToProbList(&temp.rewardPropVec, node.getChildNodeText<std::string>("reward_prop_list"));
		
		m_guanzhiMap.insert(std::make_pair(temp.level, temp));
	}
	
	return;
}


}
