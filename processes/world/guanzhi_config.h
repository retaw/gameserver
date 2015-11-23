/*
 * Author: zhupengfei
 *
 * Created: 2015-07-22 11::32:00 +0800
 *
 * Modified: 2015-07-22 11::32:00 +0800
 *
 * Description: 加载官职配置文件
 */

#ifndef PROCESS_WORLD_GUANZHI_CONFIG_HPP
#define PROCESS_WORLD_GUANZHI_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class GuanzhiConfig
{
public:
	~GuanzhiConfig() = default;
	static GuanzhiConfig& me();
private:
	static GuanzhiConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Guanzhi 
	{
		void load(componet::XmlParseNode root);
		
		struct GuanzhiItem
		{
			uint8_t  level;
			std::string name;
			uint32_t titleId;
			uint32_t needLevel;
			uint64_t needZhangong;
			uint32_t needGold;
			uint32_t dailyRewardId;
			uint32_t levelUpRewardId;
			std::vector<std::pair<PropertyType, uint32_t> > rewardPropVec;
		};

		std::map<uint8_t, GuanzhiItem> m_guanzhiMap;	//<level, GuanzhiItem>    

	} guanzhiCfg;
};


}

#endif

