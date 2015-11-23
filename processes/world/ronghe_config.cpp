#include "ronghe_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"

namespace world{

RongheConfig RongheConfig::m_me;

RongheConfig& RongheConfig::me()
{
	return m_me;
}

void RongheConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/ronghe.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	rongheCfg.load(root);
}

void RongheConfig::Ronghe::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();
	
	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		RongheItem temp;
		temp.level = node.getChildNodeText<uint8_t>("level");
		temp.needTplId = node.getChildNodeText<uint32_t>("needTplId");
		temp.needTplNum = node.getChildNodeText<uint16_t>("needTplNum");
		temp.needMoneyType = static_cast<MoneyType>(node.getChildNodeText<uint32_t>("needMoneyType"));
		temp.needMoneyNum = node.getChildNodeText<uint32_t>("needMoneyNum");
		temp.prob = node.getChildNodeText<uint32_t>("prob");

		m_rongheMap.insert(std::make_pair(temp.level, temp));
	}

	return;
}

void RongheConfig::Ronghe::clear()
{
	m_rongheMap.clear();
}

}
