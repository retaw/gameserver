/*
 * Author: zhupengfei
 *
 * Created: 2015-08-10 16::25:00 +0800
 *
 * Modified: 2015-08-10 16::25:00 +0800
 *
 * Description: 加载装备融合配置文件
 */

#ifndef PROCESS_WORLD_RONGHE_CONFIG_HPP
#define PROCESS_WORLD_RONGHE_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"
#include "water/common/roledef.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class RongheConfig
{
public:
	~RongheConfig() = default;
	static RongheConfig& me();
private:
	static RongheConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Ronghe
	{
		void load(componet::XmlParseNode root);
		void clear();

		struct RongheItem
		{
			uint8_t level;
			uint32_t needTplId;
			uint16_t needTplNum;
			MoneyType needMoneyType;
			uint32_t needMoneyNum;
			uint32_t prob;
		};

		std::map<uint8_t, RongheItem> m_rongheMap;	//<level, RongheItem>    

	} rongheCfg;
};


}

#endif

