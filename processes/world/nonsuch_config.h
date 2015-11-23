/*
 * Author: zhupengfei
 *
 * Created: 2015-06-03 +0800
 *
 * Modified: 2015-06-03 +0800
 *
 * Description: 加载极品装备被动技能配置文件
 */

#ifndef PROCESS_WORLD_NONSUCH_CONFIG_HPP
#define PROCESS_WORLD_NONSUCH_CONFIG_HPP

#include "water/componet/xmlparse.h"

#include <vector>
#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class NonsuchConfig
{
public:
	~NonsuchConfig() = default;
	static NonsuchConfig& me();
private:
	static NonsuchConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Nonsuch 
	{
		void load(XmlParseNode root);
		void clear();
	
		std::map<uint32_t, std::vector<uint32_t> > m_skillTypeMap;  //<nonsuchId, skillTypeId> 
		std::map<uint32_t, std::vector<uint32_t> > m_skillMap;      //<skillTypeId, skillId>

	} nonsuchCfg;

};


}

#endif
