#include "wing_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

WingConfig WingConfig::m_me;

WingConfig& WingConfig::me()
{
	return m_me;
}

void WingConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/wing.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	wingCfg.load(root);
}

void WingConfig::Wing::load(componet::XmlParseNode root)
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

	//翅膀晋阶
	for(XmlParseNode node = root.getChild("levelup").getChild("item"); node; ++node)
	{
		LevelUpItem temp;
		temp.sourceTplId = node.getAttr<uint32_t>("sourceTplId");
		temp.level = node.getAttr<uint8_t>("level");
		temp.destTplId = node.getAttr<uint32_t>("destTplId");
		temp.needTurnLifeLevel = static_cast<TurnLife>(node.getAttr<uint8_t>("needTurnLifeLevel"));
		temp.needLevel = node.getAttr<uint32_t>("needLevel");
		temp.needMoneyType = static_cast<MoneyType>(node.getAttr<uint8_t>("needMoneyType"));
		temp.needMoneyNum = node.getAttr<uint32_t>("needMoneyNum");
		temp.prob = node.getAttr<uint32_t>("prob");
		temp.needYuanbao = node.getAttr<uint32_t>("needYuanbao");
		strToObjList(&temp.needObjVec, node.getAttr<std::string>("need_obj_list"));
		
		m_levelUpMap.insert(std::make_pair(temp.sourceTplId, temp));
	}

	//不同方式的注灵消耗
	for(XmlParseNode node = root.getChild("consume").getChild("item"); node; ++node)
	{
		ConsumeItem temp;
		temp.type = node.getAttr<uint8_t>("type");
		temp.needTplId = node.getAttr<uint32_t>("needTplId");
		temp.needTplNum = node.getAttr<uint32_t>("needTplNum");
		temp.needMoneyType = static_cast<MoneyType>(node.getAttr<uint8_t>("needMoneyType"));
		temp.needMoneyNum = node.getAttr<uint32_t>("needMoneyNum");
		temp.rewardLingli = node.getAttr<uint32_t>("rewardLingli");
		
		m_consumeMap.insert(std::make_pair(temp.type, temp));
	}

	//注灵等级与属性加成
	for(XmlParseNode node = root.getChild("reward").getChild("item"); node; ++node)
	{
		RewardItem temp;
		temp.level = node.getAttr<uint8_t>("level");
		temp.needLingli = node.getAttr<uint32_t>("needLingli");
		temp.addPropPercent = node.getAttr<uint32_t>("addPropPercent");

		m_rewardMap.insert(std::make_pair(temp.level, temp));
	}

	return;
}

void WingConfig::Wing::clear()
{
	m_levelUpMap.clear();
	m_consumeMap.clear();
	m_rewardMap.clear();
}

}
