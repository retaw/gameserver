/*
 * Author: zhupengfei
 *
 * Created: 2015-07-16 +0800
 *
 * Modified: 2015-07-16 +0800
 *
 * Description: 加装备强化配置文件
 */

#ifndef PROCESS_WORLD_STRONG_CONFIG_HPP
#define PROCESS_WORLD_STRONG_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class StrongConfig
{
public:
	~StrongConfig() = default;
	static StrongConfig& me();
private:
	static StrongConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Strong 
	{
		void load(componet::XmlParseNode root);
		
		struct StrongItem
		{
			uint8_t  level;
			uint32_t prob;
			uint8_t  reduceLevel;
			uint32_t needStoneTplId;
			uint16_t needStoneNum;
			uint8_t  needMoneyType;
			uint32_t needMoneyNum;
			uint32_t needProTplId;
			uint16_t needProNum;
			uint32_t rewardMoney;
			uint32_t maxStrongValue;
			std::map<PropertyType, uint32_t> rewardPropMap;
		};

		std::map<uint8_t, StrongItem> m_strongMap;	//<level, StrongItem>    

	} strongCfg;
};


}

#endif

