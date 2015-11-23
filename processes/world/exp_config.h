/*
 * Author: zhupengfei
 *
 * Created: 2015-05-18 +0800
 *
 * Modified: 2015-05-18 +0800
 *
 * Description: 加载角色及英雄对应的等级经验配置
 */

#ifndef PROCESS_WORLD_EXP_CONFIG_HPP
#define PROCESS_WORLD_EXP_CONFIG_HPP


#include "water/common/roledef.h"
#include "water/componet/xmlparse.h"

#include <map>

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class ExpConfig
{
public:
	~ExpConfig() = default;
	static ExpConfig& me();
private:
	static ExpConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Exp
	{
		void load(componet::XmlParseNode root);

		struct ExpItem
		{	
			uint32_t level;
			TurnLife needTurnLife;	//需要转生等级
			uint64_t needExp_role;	//此等级角色需要的累计经验值
			uint64_t needExp_hero;	//此等级英雄需要的累计经验值
		};
	
		std::map<uint32_t, ExpItem> m_expMap;		//<level, ExpItem>

	} expCfg;

};

#endif

}
