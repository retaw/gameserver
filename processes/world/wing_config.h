/*
 * Author: zhupengfei
 *
 * Created: 2015-08-15 09::43:00 +0800
 *
 * Modified: 2015-08-15 09::43:00 +0800
 *
 * Description: 加载翅膀配置文件
 */

#ifndef PROCESS_WORLD_WING_CONFIG_HPP
#define PROCESS_WORLD_WING_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"
#include "water/common/roledef.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class WingConfig
{
public:
	~WingConfig() = default;
	static WingConfig& me();
private:
	static WingConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Wing
	{
		void load(componet::XmlParseNode root);
		void clear();

		//翅膀晋阶
		struct LevelUpItem
		{
			uint32_t sourceTplId;
			uint8_t level;
			uint32_t destTplId;
			TurnLife needTurnLifeLevel;
			uint32_t needLevel;
			MoneyType needMoneyType;
			uint32_t needMoneyNum;
			uint32_t prob;
			uint32_t needYuanbao;	//确保进阶成功所需的元宝数
			std::vector<std::pair<uint32_t, uint16_t> > needObjVec;	
		};

		//不同方式的注灵消耗
		struct ConsumeItem
		{
			uint8_t type;
			uint32_t needTplId;
			uint32_t needTplNum;
			MoneyType needMoneyType;
			uint32_t needMoneyNum;
			uint32_t rewardLingli;
		};

		//注灵等级与属性加成
		struct RewardItem
		{
			uint8_t level;
			uint32_t needLingli;
			uint32_t addPropPercent;
		};

		std::map<uint32_t, LevelUpItem> m_levelUpMap;	//<sourceTplId, LevelUpItem>    
		std::map<uint8_t, ConsumeItem> m_consumeMap;	//<type, ConsumeItem>
		std::map<uint32_t, RewardItem> m_rewardMap;		//<level, RewardItem>

	} wingCfg;
};


}

#endif

