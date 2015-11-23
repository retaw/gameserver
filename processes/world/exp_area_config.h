/*
 * Author: zhupengfei
 *
 * Created: 2015-09-15 11::43:00 +0800
 *
 * Modified: 2015-09-15 1::43:00 +0800
 *
 * Description: 加载经验区(泡点)配置文件
 */

#ifndef PROCESS_WORLD_EXP_AREA_CONFIG_HPP
#define PROCESS_WORLD_EXP_AREA_CONFIG_HPP

#include "world.h"

#include "water/common/roledef.h"
#include "water/componet/xmlparse.h"
#include "water/componet/datetime.h"

#include <unordered_map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;
using water::componet::TimePoint;

class ExpAreaConfig
{
public:
	~ExpAreaConfig() = default;
	static ExpAreaConfig& me();
private:
	static ExpAreaConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct ExpArea
	{
		void load(componet::XmlParseNode root);
		void clear();

		uint32_t multiple = 0;
		struct ActionItem
		{
			std::string notifyTime;
			std::string beginTime;
			std::string endTime;
		};

		struct RewardItem
		{
			uint32_t minLevel;
			uint32_t maxLevel;
			uint32_t addExp;
		};

		struct VipItem
		{
			uint8_t level;
			uint32_t percent;
			uint32_t limitCount;
		};

		std::vector<ActionItem> actionVec;	
		std::vector<RewardItem> rewardVec;	
		std::unordered_map<uint8_t, uint32_t> bubbleMap;
		std::unordered_map<uint8_t, VipItem> vipMap;		//<level, VipItem>

	} m_expAreaCfg;
};


}

#endif

