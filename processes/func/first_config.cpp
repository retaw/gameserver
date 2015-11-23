#include "first_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace func{

FirstConfig FirstConfig::m_me;

FirstConfig& FirstConfig::me()
{
	return m_me;
}

void FirstConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/first.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	firstCfg.load(root);
}

void FirstConfig::First::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();

	mapId = root.getChild("action").getAttr<uint32_t>("mapId");
	needLevel = root.getChild("action").getAttr<uint32_t>("needLevel");
	basePoint = root.getChild("action").getAttr<uint32_t>("basePoint");
	kickoutSec = root.getChild("action").getAttr<uint32_t>("kickoutSec");
	randomRewardId = root.getChild("reward").getAttr<uint32_t>("randomRewardId");
	winnerGiftId = root.getChild("reward").getAttr<uint32_t>("winnerGiftId");
	winnerTitleId = root.getChild("reward").getAttr<uint32_t>("winnerTitleId");

	for(XmlParseNode node = root.getChild("action").getChild("item"); node; ++node)
	{
		uint8_t wday = node.getAttr<uint8_t>("wday");
		ActionItem temp;
		temp.applyBeginTime = node.getAttr<std::string>("applyBeginTime");
		temp.applyEndTime = node.getAttr<std::string>("applyEndTime");
		temp.readyTime = node.getAttr<std::string>("readyTime");
		temp.beginTime = node.getAttr<std::string>("beginTime");
		temp.endTime = node.getAttr<std::string>("endTime");

		m_actionMap.insert(std::make_pair(wday, temp));
	}

	for(XmlParseNode node = root.getChild("reward").getChild("item"); node; ++node)
	{
		RewardItem temp;
		temp.type = node.getAttr<uint8_t>("type");
		temp.minPoint = node.getAttr<uint32_t>("minPoint");
		temp.maxPoint = node.getAttr<uint32_t>("maxPoint");
		temp.giftId = node.getAttr<uint32_t>("giftId");
		temp.zhangong = node.getAttr<uint32_t>("zhangong");
		temp.name = node.getAttr<std::string>("name");

		m_rewardVec.push_back(temp);
	}

	return;
}

void FirstConfig::First::clear()
{
	m_actionMap.clear();
	m_rewardVec.clear();
}

bool FirstConfig::First::getRewardItemByPoint(RewardItem& rewardItem, uint32_t point) const
{
	for(auto iter = m_rewardVec.begin(); iter != m_rewardVec.end(); ++iter)
	{
		if(point >= iter->minPoint && point <= iter->maxPoint)
		{
			rewardItem = *iter;
			return true;
		}
		else if(point >= iter->minPoint && 0 == iter->maxPoint)
		{
			rewardItem = *iter;
			return true;
		}
	}

	return false;
}


}
