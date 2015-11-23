/*
 * Author: zhupengfei
 *
 * Created: 2015-08-31 09::43:00 +0800
 *
 * Modified: 2015-08-31 09::43:00 +0800
 *
 * Description: 加载固定奖励生成器配置文件
 */

#ifndef PROCESS_WORLD_REWARD_FIXED_CONFIG_HPP
#define PROCESS_WORLD_REWARD_FIXED_CONFIG_HPP

#include "water/common/roledef.h"
#include "water/common/objdef.h"
#include "water/componet/xmlparse.h"

#include <unordered_map>
#include <vector>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class RewardFixedConfig
{
public:
	~RewardFixedConfig() = default;
	static RewardFixedConfig& me();
private:
	static RewardFixedConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Reward
	{
		void load(componet::XmlParseNode root);
		void clear();

		//<rewardId, RewardData>
		std::unordered_map<uint32_t, std::vector<RewardData> > rewardMap;

	} m_fixRewardCfg;
};


}

#endif

