/*
 * Author: zhupengfei
 *
 * Created: 2015-08-01 +0800
 *
 * Modified: 2015-06-01 +0800
 *
 * Description: 加装武器幸运配置文件
 */

#ifndef PROCESS_WORLD_LUCKY_CONFIG_HPP
#define PROCESS_WORLD_LUCKY_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class LuckyConfig
{
public:
	~LuckyConfig() = default;
	static LuckyConfig& me();
private:
	static LuckyConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Lucky 
	{
		void load(componet::XmlParseNode root);
		
		struct LuckyItem
		{
			uint8_t  level;
			uint32_t prob;
			uint8_t  reduceLevel;
			uint32_t needObjTplId;
			uint16_t needObjNum;
			uint8_t  needMoneyType;
			uint32_t needMoneyNum;
			uint32_t needProTplId;
			uint16_t needProNum;
			uint32_t rewardLuckyNum;
		};

		std::map<uint8_t, LuckyItem> m_luckyMap;	//<level, LuckyItem>    

	} luckyCfg;
};


}

#endif

