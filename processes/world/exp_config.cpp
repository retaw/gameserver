#include "exp_config.h"

#include "world.h"

#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"
#include "water/common/commdef.h"

namespace world{

ExpConfig ExpConfig::m_me;

ExpConfig& ExpConfig::me()
{
	return m_me;
}


void ExpConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/exp.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}

	expCfg.load(root);
}

void ExpConfig::Exp::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	m_expMap.clear();

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		ExpItem temp;
		temp.level = node.getChildNodeText<uint32_t>("level");
		temp.needTurnLife = static_cast<TurnLife>(node.getChildNodeText<uint8_t>("needTurnLife"));
		temp.needExp_role = node.getChildNodeText<uint64_t>("needExp_role");
		temp.needExp_hero = node.getChildNodeText<uint64_t>("needExp_hero");

		m_expMap.insert(std::make_pair(temp.level, temp));
	}

	return;
}

}
