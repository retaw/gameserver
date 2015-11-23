/*
 * Author: zhupengfei
 *
 * Created: 2015-08-31 17::43:00 +0800
 *
 * Modified: 2015-08-31 17::43:00 +0800
 *
 * Description: 随机奖励生成器配置文件
 */

#ifndef PROCESS_WORLD_REWARD_RANDOM_CONFIG_HPP
#define PROCESS_WORLD_REWARD_RANDOM_CONFIG_HPP

#include "water/common/roledef.h"
#include "water/common/objdef.h"
#include "water/componet/xmlparse.h"

#include <unordered_map>
#include <vector>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class RewardRandomConfig
{
public:
	~RewardRandomConfig() = default;
	static RewardRandomConfig& me();
private:
	static RewardRandomConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct RewardRandom
	{
		void load(componet::XmlParseNode root);
		void clear();

		std::unordered_map<uint32_t, std::vector<RewardData> > poolMap; //<poolId, RewardData>

		//<rewardId, std::vector<std::vector<poolId> > >
		std::unordered_map<uint32_t, std::vector<std::vector<uint32_t> > > rewardMap;

	} m_rewardRandomCfg;
};


}

#endif

