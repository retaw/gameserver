/*
 * Author: zhupengfei
 *
 * Created: 2015-09-22 18::40:00 +0800
 *
 * Modified: 2015-09-22 18::40:00 +0800
 *
 * Description: 加载激情泡点配置文件
 */

#ifndef PROCESS_WORLD_BUBBLE_POINT_CONFIG_HPP
#define PROCESS_WORLD_BUBBLE_POINT_CONFIG_HPP

#include "world.h"

#include "water/common/roledef.h"
#include "water/componet/xmlparse.h"
#include "water/componet/coord.h"

#include <unordered_map>

class XmlParseNode;

namespace world{

using namespace water;
using namespace water::componet;
using water::componet::XmlParseNode;

class BubblePointConfig
{
public:
	~BubblePointConfig() = default;
	static BubblePointConfig& me();
private:
	static BubblePointConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct BubblePoint
	{
		void load(componet::XmlParseNode root);
		void clear();

		uint32_t mapId = 0;
		uint32_t needLevel = 0;
		uint32_t span = 0;			//特殊点随机奖励时间间隔
		uint32_t kickoutSec = 0;	//活动结束后N秒踢人

		struct ActionItem
		{
			std::string notifyTime;
			std::string beginTime;
			std::string endTime;
		};

		struct NormalItem
		{
			uint32_t minLevel;
			uint32_t maxLevel;
			uint32_t addExp;
		};

		struct SpecialItem
		{
			uint32_t posX;
			uint32_t posY;
			uint32_t buffId;
			uint32_t percent;
			uint32_t rewardId;
			std::string name;
		};

		std::vector<ActionItem> m_actionVec;	
		std::vector<NormalItem> m_noramlVec;	
		std::unordered_map<Coord2D, SpecialItem> m_specialMap;		//<Coord2D, SpecialItem>

	} bubblePointCfg;
};


}

#endif

