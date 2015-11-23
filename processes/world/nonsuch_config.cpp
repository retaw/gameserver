#include "nonsuch_config.h"

#include "water/componet/logger.h"
#include "water/componet/exception.h"
#include "water/componet/string_kit.h"

namespace world{

NonsuchConfig NonsuchConfig::m_me;

NonsuchConfig& NonsuchConfig::me()
{
	return m_me;
}

void NonsuchConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using water::componet::XmlParseDoc;

	const std::string cfgFile = configDir + "/nonsuch.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	nonsuchCfg.load(root);
}

void NonsuchConfig::Nonsuch::load(XmlParseNode root)
{
	if(!root)
		return;

	clear();

	auto strToProbList = [](std::vector<uint32_t>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ";");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, ",");
			if(propItems.size() < 2)
				continue;

			const uint32_t section = propItems[1];
			for(uint32_t i = 0; i < section; ++i)
			{
				ret->push_back(propItems[0]);
			}
		}
	};

	for(XmlParseNode node = root.getChild("skilltype").getChild("item"); node; ++node)
	{
		uint32_t nonsuchId = node.getAttr<uint32_t>("nonsuchId");
		std::vector<uint32_t> skillTypeVec;
		strToProbList(&skillTypeVec, node.getAttr<std::string>("skill_type_list"));

		m_skillTypeMap.insert(std::make_pair(nonsuchId, skillTypeVec));
	}

	for(XmlParseNode node = root.getChild("skill").getChild("item"); node; ++node)
	{
		uint32_t skillTypeId = node.getAttr<uint32_t>("skillTypeId");
		std::vector<uint32_t> skillVec;
		strToProbList(&skillVec, node.getAttr<std::string>("skill_list"));

		m_skillMap.insert(std::make_pair(skillTypeId, skillVec));
	}

	return;
}

void NonsuchConfig::Nonsuch::clear()
{
	m_skillTypeMap.clear();
	m_skillMap.clear();
}


}
