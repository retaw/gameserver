#include "relive_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"

namespace world{

ReliveConfig ReliveConfig::m_me;

ReliveConfig& ReliveConfig::me()
{
	return m_me;
}

void ReliveConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/relive.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	reliveCfg.load(root);
}

void ReliveConfig::Relive::load(componet::XmlParseNode root)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	if(!root)
		return;

	XmlParseNode nodeRelive = root.getChild("relive");
	mapId = nodeRelive.getAttr<uint32_t>("mapId");
	posX = nodeRelive.getAttr<uint16_t>("posX");
	posY = nodeRelive.getAttr<uint16_t>("posY");
	sec = nodeRelive.getAttr<uint32_t>("sec");

	for(XmlParseNode node = nodeRelive.getChild("item"); node; ++node)
	{
		Relive::OldPlaceRelive temp;
		temp.type = node.getAttr<uint8_t>("type");
		temp.tplId = node.getAttr<uint32_t>("tplId");
		temp.needNum = node.getAttr<uint32_t>("needNum");
		temp.needMoney = node.getAttr<uint32_t>("needMoney");
		temp.percent = node.getAttr<uint32_t>("percent");

		reliveMap.insert(std::make_pair(temp.type, temp));
	}
	
	return;
}

SceneId ReliveConfig::cityId() const
{
    return reliveCfg.mapId;
}

Coord2D ReliveConfig::cityRelivePos() const
{
    return Coord2D(reliveCfg.posX, reliveCfg.posY);
}


}
