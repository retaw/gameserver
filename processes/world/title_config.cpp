#include "title_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

TitleConfig TitleConfig::m_me;

TitleConfig& TitleConfig::me()
{
	return m_me;
}

void TitleConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/title.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	titleCfg.load(root);
}

void TitleConfig::Title::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	m_titleMap.clear();

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
		TitleItem temp;
		temp.titleId = node.getChildNodeText<uint32_t>("titleId");
		temp.titleType = node.getChildNodeText<uint8_t>("titleType");
		temp.lastSec = node.getChildNodeText<uint32_t>("lastSec");
		temp.priority = node.getChildNodeText<uint32_t>("priority");
		strToProbList(&temp.rewardPropVec, node.getChildNodeText<std::string>("reward_prop_list"));
		
		m_titleMap.insert(std::make_pair(temp.titleId, temp));
	}
	
	return;
}

uint32_t TitleConfig::Title::getPriority(uint32_t titleId) const
{
	auto pos = m_titleMap.find(titleId);
	if(pos == m_titleMap.end())
		return 0; 

	return pos->second.priority;
}

std::vector<std::pair<PropertyType, uint32_t> > TitleConfig::Title::getRewardPropVec(uint32_t titleId) const
{
	std::vector<std::pair<PropertyType, uint32_t> > temp;
	temp.clear();

	auto pos = m_titleMap.find(titleId);
	if(pos == m_titleMap.end())
		return temp;

	return pos->second.rewardPropVec;
}

}
