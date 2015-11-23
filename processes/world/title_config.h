/*
 * Author: zhupengfei
 *
 * Created: 2015-07-24 13::59:00 +0800
 *
 * Modified: 2015-07-24 13::59:00 +0800
 *
 * Description: 加载称号配置文件
 */

#ifndef PROCESS_WORLD_TITLE_CONFIG_HPP
#define PROCESS_WORLD_TITLE_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class TitleConfig
{
public:
	~TitleConfig() = default;
	static TitleConfig& me();
private:
	static TitleConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Title 
	{
		void load(componet::XmlParseNode root);
		
		struct TitleItem
		{
			uint32_t titleId;
			uint8_t	 titleType;
			uint32_t lastSec;
			uint32_t priority;
			std::vector<std::pair<PropertyType, uint32_t> > rewardPropVec;
		};

		std::map<uint32_t, TitleItem> m_titleMap;	//<titleId, TitleItem>    

		uint32_t getPriority(uint32_t titleId) const;
		std::vector<std::pair<PropertyType, uint32_t> > getRewardPropVec(uint32_t titleId) const;

	} titleCfg;
};


}

#endif

