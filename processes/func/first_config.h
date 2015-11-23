/*
 * Author: zhupengfei
 *
 * Created: 2015-10-08 15::40:00 +0800
 *
 * Modified: 2015-10-08 15::40:00 +0800
 *
 * Description: 加载天下第一配置文件
 */

#ifndef PROCESS_FUNC_FIRST_CONFIG_HPP
#define PROCESS_FUNC_FIRST_CONFIG_HPP

#include "water/componet/xmlparse.h"

#include <map>
#include <vector>

class XmlParseNode;

namespace func{

using namespace water;
using namespace water::componet;
using water::componet::XmlParseNode;

class FirstConfig
{
public:
	~FirstConfig() = default;
	static FirstConfig& me();
private:
	static FirstConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct First
	{
		void load(componet::XmlParseNode root);
		void clear();
		
		uint32_t mapId = 0;
		uint32_t needLevel = 0;
		uint32_t basePoint = 0;
		uint32_t kickoutSec = 0;
		uint32_t randomRewardId = 0;
		uint32_t winnerGiftId = 0;
		uint32_t winnerTitleId = 0;

		struct ActionItem
		{
			std::string applyBeginTime;
			std::string applyEndTime;
			std::string readyTime;
			std::string beginTime;
			std::string endTime;
		};

		struct RewardItem
		{
			uint8_t type;
			uint32_t minPoint;
			uint32_t maxPoint;
			uint32_t giftId;
			uint32_t zhangong;
			std::string name;
		};

		std::map<uint8_t, ActionItem> m_actionMap;	//<wday, ActionItem>
		std::vector<RewardItem> m_rewardVec;

		bool getRewardItemByPoint(RewardItem& rewardItem, uint32_t point) const;

	} firstCfg;
};


}

#endif

