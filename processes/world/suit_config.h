/*
 * Author: zhupengfei
 *
 * Created: 2015-06-04 +0800
 *
 * Modified: 2015-06-04 +0800
 *
 * Description: 加载套装属性配置文件
 */

#ifndef PROCESS_WORLD_SUIT_CONFIG_HPP
#define PROCESS_WORLD_SUIT_CONFIG_HPP

#include "water/componet/xmlparse.h"

#include <vector>
#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class SuitConfig
{
public:
	~SuitConfig() = default;
	static SuitConfig& me();
private:
	static SuitConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Suit
	{
		void load(XmlParseNode root);

		struct SuitProp
		{
			uint8_t suitNum;
			uint32_t skillId;
		};

		std::map<uint32_t, std::vector<SuitProp> > m_suitPropMap;   //<suitId, SuitProp>

	} suitCfg;
};


}

#endif
