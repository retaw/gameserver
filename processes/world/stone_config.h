/*
 * Author: zhupengfei
 *
 * Created: 2015-07-13 +0800
 *
 * Modified: 2015-07-13 +0800
 *
 * Description: 加载宝石等级加成配置文件
 */

#ifndef PROCESS_WORLD_STONE_CONFIG_HPP
#define PROCESS_WORLD_STONE_CONFIG_HPP

#include "world.h"
#include "water/componet/xmlparse.h"

#include <vector>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class StoneConfig
{
public:
	~StoneConfig() = default;
	static StoneConfig& me();
private:
	static StoneConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Stone 
	{
		void load(componet::XmlParseNode root);
		
		struct StoneItem
		{
			uint8_t job;
			uint32_t levelMin;
			uint32_t levelMax;

			uint32_t p_attackMin;	//物攻Min
			uint32_t p_attackMax;	//物攻Max
			uint32_t m_attackMin;	//魔攻Min
			uint32_t m_attackMax;	//魔攻Max
			uint32_t witchMin;		//道术Min
			uint32_t witchMax;		//道术Max
			uint32_t p_defenceMin;	//物防Min
			uint32_t p_defenceMax;	//物防Max
			uint32_t m_defenceMin;	//魔防Min
			uint32_t m_defenceMax;	//魔防Max

			uint32_t hp;			//生命
			uint32_t mp;			//魔法
			uint32_t shot;			//命中
			uint32_t p_escape;		//物闪
			uint32_t m_escape;		//魔闪
			uint32_t crit;			//暴击
			uint32_t antiCrit;		//防爆
			uint32_t lucky;			//幸运
			uint32_t evil;			//诅咒
			uint32_t critDamage;	//暴伤
			uint32_t shotRatio;		//命中率
			uint32_t escapeRatio;	//闪避率
			uint32_t critRatio;		//暴击率

			uint32_t hpLv;			//生命等级
			uint32_t mpLv;			//魔法等级
			uint32_t damageAdd;		//增伤
			uint32_t damageReduce;	//减伤
			uint32_t damageAddLv;	//增伤等级
			uint32_t damageReduceLv;//减伤等级
			uint32_t antiDropEquip; //防爆(装备掉落)
		};

		std::vector<StoneItem> m_stoneVec;    

	} stoneCfg;
};


}

#endif

