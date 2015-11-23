#include "suit_config.h"

#include "water/componet/logger.h"
#include "water/componet/exception.h"
#include "water/componet/string_kit.h"

namespace world{

SuitConfig SuitConfig::m_me;

SuitConfig& SuitConfig::me()
{
	return m_me;
}

void SuitConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using water::componet::XmlParseDoc;

	const std::string cfgFile = configDir + "/suit.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	suitCfg.load(root);
}

void SuitConfig::Suit::load(XmlParseNode root)
{
	if(!root)
		return;

	m_suitPropMap.clear();

	auto strToPropList = [](std::vector<SuitProp>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ";");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, ",");
			if(propItems.size() != 2)
				continue;
	
			SuitProp temp;
			temp.suitNum = propItems[0];
			temp.skillId = propItems[1];

			ret->push_back(temp);
		}
	};

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		uint32_t suitId = node.getChildNodeText<uint32_t>("suitId");
		std::vector<SuitProp> suitPropVec;
		strToPropList(&suitPropVec, node.getChildNodeText<std::string>("suit_prop_list"));

		m_suitPropMap.insert(std::make_pair(suitId, suitPropVec));
	}

	return;
}


}
