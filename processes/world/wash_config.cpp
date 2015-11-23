#include "wash_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

WashConfig WashConfig::m_me;

WashConfig& WashConfig::me()
{
	return m_me;
}

void WashConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/wash.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	washCfg.load(root);
}

void WashConfig::Wash::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();
	
	auto strToPropTypeList = [](std::vector<PropertyType>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() != 2)
				continue;

			const uint32_t section = propItems[1];
			for(uint32_t i = 0; i < section; ++i)
			{
				ret->push_back(static_cast<PropertyType>(propItems[0]));
			}
		}
	};

	auto strToPropValueList = [](std::vector<std::pair<uint32_t, uint32_t> >* ret, const std::string& str)
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

	auto strToQualityList = [](std::vector<uint8_t>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() != 2)
				continue;

			const uint32_t section = propItems[1];
			for(uint32_t i = 0; i < section; ++i)
			{
				ret->push_back(propItems[0]);
			}
		}
	};

	//base
	for(XmlParseNode node = root.getChild("base").getChild("item"); node; ++node)
	{
		BaseItem temp;
		temp.washType = node.getAttr<uint8_t>("washType");
		temp.level = node.getAttr<uint32_t>("level");
		strToPropTypeList(&temp.propTypeVec, node.getAttr<std::string>("prop_type_list"));
		
		m_baseMap.insert(std::make_pair(temp.washType, temp));
	}

	//prop
	for(XmlParseNode node = root.getChild("prop").getChild("item"); node; ++node)
	{
		PropItem temp;
		const uint8_t washType = node.getAttr<uint8_t>("washType");
		temp.propType = static_cast<PropertyType>(node.getAttr<uint32_t>("propType"));
		strToPropValueList(&temp.propValueVec, node.getAttr<std::string>("quality_prop_list"));

		auto& iter = m_propMap[washType];
		iter.insert(std::make_pair(temp.propType, temp));
	}

	//consume
	for(XmlParseNode node = root.getChild("consume").getChild("item"); node; ++node)
	{
		ConsumeItem temp;
		const uint8_t washType = node.getAttr<uint8_t>("washType");
		temp.washWay = node.getAttr<uint8_t>("washWay");
		temp.needMoneyType = static_cast<MoneyType>(node.getAttr<uint32_t>("needMoneyType"));
		temp.needMoneyNum = node.getAttr<uint32_t>("needMoneyNum");
		temp.lockNeedYuanbao = node.getAttr<uint32_t>("lockNeedYuanbao");
		strToQualityList(&temp.qualityVec, node.getAttr<std::string>("quality_prob_list"));

		auto& iter = m_consumeMap[washType];
		iter.insert(std::make_pair(temp.washWay, temp));
	}


	return;
}

void WashConfig::Wash::clear()
{
	m_baseMap.clear();
	m_propMap.clear();
	m_consumeMap.clear();
}

}
