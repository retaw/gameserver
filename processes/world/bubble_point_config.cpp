#include "bubble_point_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

BubblePointConfig BubblePointConfig::m_me;

BubblePointConfig& BubblePointConfig::me()
{
	return m_me;
}

void BubblePointConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/bubble_point.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	bubblePointCfg.load(root);
}

void BubblePointConfig::BubblePoint::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();

	mapId = root.getChild("normal").getAttr<uint32_t>("mapId");
	needLevel = root.getChild("normal").getAttr<uint32_t>("needLevel");
	span = root.getChild("special_point").getAttr<uint32_t>("span");
	kickoutSec = root.getChild("action").getAttr<uint32_t>("kickoutSec");

	for(XmlParseNode node = root.getChild("action").getChild("item"); node; ++node)
	{
		ActionItem temp;
		temp.notifyTime = node.getAttr<std::string>("notifyTime");
		temp.beginTime = node.getAttr<std::string>("beginTime");
		temp.endTime = node.getAttr<std::string>("endTime");

		m_actionVec.push_back(temp);
	}

	for(XmlParseNode node = root.getChild("normal").getChild("item"); node; ++node)
	{
		NormalItem temp;
		temp.minLevel = node.getAttr<uint32_t>("minLevel");
		temp.maxLevel = node.getAttr<uint32_t>("maxLevel");
		temp.addExp = node.getAttr<uint32_t>("addExp");

		m_noramlVec.push_back(temp);
	}

	for(XmlParseNode node = root.getChild("special_point").getChild("item"); node; ++node)
	{
		SpecialItem temp;
		temp.posX = node.getAttr<uint32_t>("posX");
		temp.posY = node.getAttr<uint32_t>("posY");
		temp.buffId = node.getAttr<uint32_t>("buffId");
		temp.percent = node.getAttr<uint32_t>("percent");
		temp.rewardId = node.getAttr<uint32_t>("rewardId");
		temp.name = node.getAttr<std::string>("name");

		m_specialMap.insert(std::make_pair(Coord2D(temp.posX, temp.posY), temp));
	}

	return;
}

void BubblePointConfig::BubblePoint::clear()
{
	m_actionVec.clear();
	m_noramlVec.clear();
	m_specialMap.clear();
}


}
