#include "exp_area_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

ExpAreaConfig ExpAreaConfig::m_me;

ExpAreaConfig& ExpAreaConfig::me()
{
	return m_me;
}

void ExpAreaConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/exp_area.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	m_expAreaCfg.load(root);
}

void ExpAreaConfig::ExpArea::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();

	multiple = root.getChild("action").getAttr<uint32_t>("multiple");

	for(XmlParseNode node = root.getChild("action").getChild("item"); node; ++node)
	{
		ActionItem temp;
		temp.notifyTime = node.getAttr<std::string>("notifyTime");
		temp.beginTime = node.getAttr<std::string>("beginTime");
		temp.endTime = node.getAttr<std::string>("endTime");

		actionVec.push_back(temp);
	}

	for(XmlParseNode node = root.getChild("reward").getChild("item"); node; ++node)
	{
		RewardItem temp;
		temp.minLevel = node.getAttr<uint32_t>("minLevel");
		temp.maxLevel = node.getAttr<uint32_t>("maxLevel");
		temp.addExp = node.getAttr<uint32_t>("addExp");

		rewardVec.push_back(temp);
	}

	for(XmlParseNode node = root.getChild("bubble").getChild("item"); node; ++node)
	{
		uint8_t type = node.getAttr<uint8_t>("type");
		uint32_t percent = node.getAttr<uint32_t>("percent");

		bubbleMap.insert(std::make_pair(type, percent));
	}

	for(XmlParseNode node = root.getChild("vip").getChild("item"); node; ++node)
	{
		VipItem temp;
		temp.level = node.getAttr<uint8_t>("level");
		temp.percent = node.getAttr<uint32_t>("percent");
		temp.limitCount = node.getAttr<uint32_t>("limit_count");

		vipMap.insert(std::make_pair(temp.level, temp));
	}

	return;
}

void ExpAreaConfig::ExpArea::clear()
{
	actionVec.clear();
	rewardVec.clear();
	bubbleMap.clear();
	vipMap.clear();
}


}
