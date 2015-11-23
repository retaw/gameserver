#include "equip_quality_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

EquipQualityConfig EquipQualityConfig::m_me;

EquipQualityConfig& EquipQualityConfig::me()
{
	return m_me;
}

void EquipQualityConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/equip_quality.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	qualityCfg.load(root);
}

void EquipQualityConfig::EquipQuality::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();
	
	auto strToObjList = [](std::vector<std::pair<uint32_t, uint16_t> >* ret, const std::string& str)
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

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		QualityItem temp;
		temp.sourceTplId = node.getChildNodeText<uint32_t>("sourceTplId");
		temp.destTplId = node.getChildNodeText<uint32_t>("destTplId");
		temp.needMoneyType = static_cast<MoneyType>(node.getChildNodeText<uint32_t>("needMoneyType"));
		temp.needMoneyNum = node.getChildNodeText<uint32_t>("needMoneyNum");
		temp.prob = node.getChildNodeText<uint32_t>("prob");
		strToObjList(&temp.needObjVec, node.getChildNodeText<std::string>("need_obj_list"));

		m_equipMap.insert(std::make_pair(temp.sourceTplId, temp));
	}

	return;
}

void EquipQualityConfig::EquipQuality::clear()
{
	m_equipMap.clear();
}

}
