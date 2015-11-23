/*
 * Author: zhupengfei
 *
 * Created: 2015-09-06 10::20:00 +0800
 *
 * Modified: 2015-09-06 10::20:00 +0800
 *
 * Description: 加载转生配置文件
 */

#ifndef PROCESS_WORLD_ZHUAN_SHENG_CONFIG_HPP
#define PROCESS_WORLD_ZHUAN_SHENG_CONFIG_HPP

#include "world.h"
#include "pkdef.h"

#include "water/common/roledef.h"
#include "water/componet/xmlparse.h"

#include <vector>
#include <map>

class XmlParseNode;

namespace world{

enum class LimitType : uint8_t
{
	needLevel		= 1,	//等级
	strongLevel		= 2,	//全身装备强化最低等级
	stoneLevel		= 3,	//全身宝石总等级
	horseLevel		= 4,	//坐骑等级
	guanzhiLevel	= 5,	//官职等级
	lingliLevel		= 6,	//翅膀灵力等级
};


using namespace water;
using water::componet::XmlParseNode;

class ZhuanshengConfig
{
public:
	~ZhuanshengConfig() = default;
	static ZhuanshengConfig& me();
private:
	static ZhuanshengConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Zhuansheng
	{
		void load(componet::XmlParseNode root);
		void clear();

		//转生条件
		struct BaseItem
		{
			uint8_t level;
			MoneyType needMoneyType;
			uint32_t needMoneyNum;
			std::vector<std::pair<LimitType, uint32_t> > limitVec;	
		};

		//角色转生等级属性加成
		struct RewardRole
		{
			uint8_t level;
			std::vector<std::pair<PropertyType, uint32_t> > rewardPropVec;
		};

		//英雄转生等级属性加成
		struct RewardHero
		{
			uint8_t level;
			std::vector<std::pair<PropertyType, uint32_t> > rewardPropVec;
		};

		//<level, BaseItem> 
		std::map<uint8_t, BaseItem> m_baseMap;	

		//<job, <level, RewardRole> >
		std::map<Job, std::map<uint8_t, RewardRole> > m_roleRewardMap;
		
		//<job, level, RewardHero> >
		std::map<Job, std::map<uint8_t, RewardHero> > m_heroRewardMap;

	} zhuanshengCfg;
};


}

#endif

