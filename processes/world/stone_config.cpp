#include "stone_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"

namespace world{

StoneConfig StoneConfig::m_me;

StoneConfig& StoneConfig::me()
{
	return m_me;
}

void StoneConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/stone.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	stoneCfg.load(root);
}

void StoneConfig::Stone::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	m_stoneVec.clear();

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		StoneItem temp;
		temp.job = node.getChildNodeText<uint8_t>("job");
		temp.levelMin = node.getChildNodeText<uint32_t>("levelMin");
		temp.levelMax = node.getChildNodeText<uint32_t>("levelMax");

		temp.p_attackMin = node.getChildNodeText<uint32_t>("p_attackMin");
		temp.p_attackMax = node.getChildNodeText<uint32_t>("p_attackMax");
		temp.m_attackMin = node.getChildNodeText<uint32_t>("m_attackMin");
		temp.m_attackMax = node.getChildNodeText<uint32_t>("m_attackMax");
		temp.witchMin = node.getChildNodeText<uint32_t>("witchMin");
		temp.witchMax = node.getChildNodeText<uint32_t>("witchMax");
		temp.p_defenceMin = node.getChildNodeText<uint32_t>("p_defenceMin");
		temp.p_defenceMax = node.getChildNodeText<uint32_t>("p_defenceMax");
		temp.m_defenceMin = node.getChildNodeText<uint32_t>("m_defenceMin");
		temp.m_defenceMax = node.getChildNodeText<uint32_t>("m_defenceMax");

		temp.hp = node.getChildNodeText<uint32_t>("hp");
		temp.mp = node.getChildNodeText<uint32_t>("mp");
		temp.shot = node.getChildNodeText<uint32_t>("shot");
		temp.p_escape = node.getChildNodeText<uint32_t>("p_escape");
		temp.m_escape = node.getChildNodeText<uint32_t>("m_escape");
		temp.crit = node.getChildNodeText<uint32_t>("crit");
		temp.antiCrit = node.getChildNodeText<uint32_t>("antiCrit");
		temp.lucky = node.getChildNodeText<uint32_t>("lucky");
		temp.evil = node.getChildNodeText<uint32_t>("evil");
		temp.critDamage = node.getChildNodeText<uint32_t>("critDamage");
		temp.shotRatio = node.getChildNodeText<uint32_t>("shotRatio");
		temp.escapeRatio = node.getChildNodeText<uint32_t>("escapeRatio");
		temp.critRatio = node.getChildNodeText<uint32_t>("critRatio");

		temp.hpLv = node.getChildNodeText<uint32_t>("hpLv");
		temp.mpLv = node.getChildNodeText<uint32_t>("mpLv");
		temp.damageAdd = node.getChildNodeText<uint32_t>("damageAdd");
		temp.damageReduce = node.getChildNodeText<uint32_t>("damageReduce");
		temp.damageAddLv = node.getChildNodeText<uint32_t>("damageAddLv");
		temp.damageReduceLv = node.getChildNodeText<uint32_t>("damageReduceLv");
		temp.antiDropEquip = node.getChildNodeText<uint32_t>("antiDropEquip");
		
		m_stoneVec.push_back(temp);
	}
	
	return;
}


}
