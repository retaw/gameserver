/*
 * Author: zhupengfei
 *
 * Created: 2015-09-08 11::46:00 +0800
 *
 * Modified: 2015-09-08 11::46:00 +0800
 *
 * Description: 加载强化、幸运分解配置文件
 */

#ifndef PROCESS_WORLD_FEN_JIE_CONFIG_HPP
#define PROCESS_WORLD_FEN_JIE_CONFIG_HPP

#include "world.h"

#include "water/common/roledef.h"
#include "water/common/objdef.h"
#include "water/componet/xmlparse.h"

#include <vector>
#include <unordered_map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class FenjieConfig
{
public:
	~FenjieConfig() = default;
	static FenjieConfig& me();
private:
	static FenjieConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Fenjie
	{
		void load(componet::XmlParseNode root);
		void clear();

		struct RewardItem
		{
			TplId tplId;
			uint16_t num;
			Bind bind;
		};

		//<level, RewardItem>
		std::unordered_map<uint8_t, std::vector<RewardItem> > m_strongRewardMap;
		std::unordered_map<uint8_t, std::vector<RewardItem> > m_luckyRewardMap;

	} fenjieCfg;
};


}

#endif

