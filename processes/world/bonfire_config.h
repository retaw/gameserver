/*
 * Author: zhupengfei
 *
 * Created: 2015-10-14 10::55:00 +0800
 *
 * Modified: 2015-10-14 10::55:00 +0800
 *
 * Description: 加载篝火配置文件
 */

#ifndef PROCESS_WORLD_BONFIRE_CONFIG_HPP
#define PROCESS_WORLD_BONFIRE_CONFIG_HPP

#include "world.h"

#include "water/common/roledef.h"
#include "water/componet/xmlparse.h"

#include <unordered_map>
#include <string>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;
using water::componet::TimePoint;

class BonfireConfig
{
public:
	~BonfireConfig() = default;
	static BonfireConfig& me();
private:
	static BonfireConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Bonfire
	{
		void load(componet::XmlParseNode root);
		void clear();

		uint32_t drinkRewardId = 0;

		struct RewardItem
		{
			uint32_t minLevel;
			uint32_t maxLevel;
			uint32_t addExp;
		};

		struct DrinkItem
		{
			uint8_t type;
			uint32_t percent;
			std::string name;
		};

		std::unordered_map<uint32_t, uint32_t> m_levelMap;	//<tplId, needLevel>
		std::unordered_map<uint32_t, std::vector<RewardItem> > m_rewardMap; //<tplId, RewardItem>
		std::unordered_map<uint8_t, uint32_t> m_teamMap;	//<count, percent>
		std::unordered_map<uint32_t, DrinkItem> m_drinkMap;	//<type, DrinkItem>
		std::unordered_map<uint8_t, uint32_t> m_vipMap;		//<level, limit_count>

	} bonfireCfg;
};


}

#endif

