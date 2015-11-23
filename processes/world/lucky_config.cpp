#include "lucky_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

LuckyConfig LuckyConfig::m_me;

LuckyConfig& LuckyConfig::me()
{
	return m_me;
}

void LuckyConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/lucky.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	luckyCfg.load(root);
}

void LuckyConfig::Lucky::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	m_luckyMap.clear();

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		LuckyItem temp;
		temp.level = node.getChildNodeText<uint8_t>("level");
		temp.prob = node.getChildNodeText<uint32_t>("prob");
		temp.reduceLevel = node.getChildNodeText<uint8_t>("reduceLevel");
		temp.needObjTplId = node.getChildNodeText<uint32_t>("needObjTplId");
		temp.needObjNum = node.getChildNodeText<uint16_t>("needObjNum");
		temp.needMoneyType = node.getChildNodeText<uint8_t>("needMoneyType");
		temp.needMoneyNum = node.getChildNodeText<uint32_t>("needMoneyNum");
		temp.needProTplId = node.getChildNodeText<uint32_t>("needProTplId");
		temp.needProNum = node.getChildNodeText<uint16_t>("needProNum");
		temp.rewardLuckyNum = node.getChildNodeText<uint32_t>("rewardLuckyNum");
		
		m_luckyMap.insert(std::make_pair(temp.level, temp));
	}
	
	return;
}


}
