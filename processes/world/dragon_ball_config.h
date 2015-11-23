/*
 * Author: zhupengfei
 *
 * Created: 2015-08-25 11::45:00 +0800
 *
 * Modified: 2015-08-25 11::45:00 +0800
 *
 * Description: 加载龙珠配置文件
 */

#ifndef PROCESS_WORLD_DRAGON_BALL_CONFIG_HPP
#define PROCESS_WORLD_DRAGON_BALL_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"
#include "water/common/roledef.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class DragonBallConfig
{
public:
	~DragonBallConfig() = default;
	static DragonBallConfig& me();
private:
	static DragonBallConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct DragonBall
	{
		void load(componet::XmlParseNode root);
		void clear();

		struct DragonItem
		{
			uint8_t level;
			uint32_t nextLevelNeedExp;
			TurnLife needTurnLifeLevel;
			uint32_t needLevel;
			uint32_t needTplId;
			uint32_t needTplNum;
			MoneyType needMoneyType;
			uint32_t needMoneyNum;
			uint32_t rewardExp;
			bool bNotify;
			std::vector<std::pair<PropertyType, uint32_t> > rewardPropVec;	
			std::string name;
		};

		//<type, <level, DragonItem> >
		std::map<uint8_t, std::map<uint8_t, DragonItem> > m_dragonBallMap;	   
	} dragonCfg;
};


}

#endif

