/*
 * Author: zhupengfei
 *
 * Created: 2015-08-06 09::43:00 +0800
 *
 * Modified: 2015-08-06 09::43:00 +0800
 *
 * Description: 加载洗练配置文件
 */

#ifndef PROCESS_WORLD_WASH_CONFIG_HPP
#define PROCESS_WORLD_WASH_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"
#include "water/common/roledef.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class WashConfig
{
public:
	~WashConfig() = default;
	static WashConfig& me();
private:
	static WashConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Wash 
	{
		void load(componet::XmlParseNode root);
		void clear();

		struct BaseItem
		{
			uint8_t washType;
			uint32_t level;
			std::vector<PropertyType> propTypeVec;	//属性类型集合
		};

		struct PropItem
		{
			PropertyType propType;
			std::vector<std::pair<uint32_t, uint32_t> > propValueVec;//此品质可随到的最大属性区间
		};

		struct ConsumeItem
		{
			uint8_t washWay;
			MoneyType needMoneyType;
			uint32_t needMoneyNum;
			uint32_t lockNeedYuanbao;
			std::vector<uint8_t> qualityVec;	//品质集合
		};

		std::map<uint8_t, BaseItem> m_baseMap;	//<WashType, BaseItem>    
		std::map<uint8_t, std::map<PropertyType, PropItem> > m_propMap; 
		std::map<uint8_t, std::map<uint8_t, ConsumeItem> > m_consumeMap; //<washType, <washWay, ConsumeItem> >

	} washCfg;
};


}

#endif

