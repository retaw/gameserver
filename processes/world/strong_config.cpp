#include "strong_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

StrongConfig StrongConfig::m_me;

StrongConfig& StrongConfig::me()
{
	return m_me;
}

void StrongConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/strong.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	strongCfg.load(root);
}

void StrongConfig::Strong::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	m_strongMap.clear();

	auto strToProbList = [](std::map<PropertyType, uint32_t>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() < 2)
				continue;

			ret->insert(std::make_pair(static_cast<PropertyType>(propItems[0]), propItems[1]));
		}
	};

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		StrongItem temp;
		temp.level = node.getChildNodeText<uint8_t>("level");
		temp.prob = node.getChildNodeText<uint32_t>("prob");
		temp.reduceLevel = node.getChildNodeText<uint8_t>("reduceLevel");
		temp.needStoneTplId = node.getChildNodeText<uint32_t>("needStoneTplId");
		temp.needStoneNum = node.getChildNodeText<uint16_t>("needStoneNum");
		temp.needMoneyType = node.getChildNodeText<uint8_t>("needMoneyType");
		temp.needMoneyNum = node.getChildNodeText<uint32_t>("needMoneyNum");
		temp.needProTplId = node.getChildNodeText<uint32_t>("needProTplId");
		temp.needProNum = node.getChildNodeText<uint16_t>("needProNum");
		temp.rewardMoney = node.getChildNodeText<uint32_t>("rewardMoney");
		temp.maxStrongValue = node.getChildNodeText<uint32_t>("maxStrongValue");
		strToProbList(&temp.rewardPropMap, node.getChildNodeText<std::string>("reward_prop_list"));
		
		m_strongMap.insert(std::make_pair(temp.level, temp));
	}
	
	return;
}


}
