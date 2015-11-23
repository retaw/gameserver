#include "dragon_ball_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

DragonBallConfig DragonBallConfig::m_me;

DragonBallConfig& DragonBallConfig::me()
{
	return m_me;
}

void DragonBallConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/dragon_ball.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	dragonCfg.load(root);
}

void DragonBallConfig::DragonBall::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();
	
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

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		DragonItem temp;
		const uint8_t type = node.getChildNodeText<uint8_t>("type");
		temp.level = node.getChildNodeText<uint8_t>("level");
		temp.nextLevelNeedExp = node.getChildNodeText<uint32_t>("nextLevelNeedExp");
		temp.needTurnLifeLevel = static_cast<TurnLife>(node.getChildNodeText<uint8_t>("needTurnLifeLevel"));
		temp.needLevel = node.getChildNodeText<uint32_t>("needLevel");
		temp.needTplId = node.getChildNodeText<uint32_t>("needTplId");
		temp.needTplNum = node.getChildNodeText<uint32_t>("needTplNum");
		temp.needMoneyType = static_cast<MoneyType>(node.getChildNodeText<uint8_t>("needMoneyType"));
		temp.needMoneyNum = node.getChildNodeText<uint32_t>("needMoneyNum");
		temp.rewardExp = node.getChildNodeText<uint32_t>("rewardExp");
		temp.bNotify = node.getChildNodeText<bool>("bNotify");
		strToPropList(&temp.rewardPropVec, node.getChildNodeText<std::string>("reward_prop_list"));
		temp.name = node.getChildNodeText<std::string>("name");

		auto& iter = m_dragonBallMap[type];
		iter.insert(std::make_pair(temp.level, temp));
	}

	return;
}

void DragonBallConfig::DragonBall::clear()
{
	m_dragonBallMap.clear();
}

}
