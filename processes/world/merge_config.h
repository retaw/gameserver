/*
 * Author: zhupengfei
 *
 * Created: 2015-08-13 17::25:00 +0800
 *
 * Modified: 2015-08-13 17::25:00 +0800
 *
 * Description: 加载合成配置文件
 */

#ifndef PROCESS_WORLD_MERGE_CONFIG_HPP
#define PROCESS_WORLD_MERGE_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"
#include "water/common/roledef.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class MergeConfig
{
public:
	~MergeConfig() = default;
	static MergeConfig& me();
private:
	static MergeConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Merge
	{
		void load(componet::XmlParseNode root);
		void clear();

		struct MergeItem
		{
			uint32_t mergeTplId;
			MoneyType needMoneyType;
			uint32_t needMoneyNum;
			uint32_t prob;
			Bind bind;
			std::vector<std::pair<uint32_t, uint16_t> > needObjVec;
		};

		std::map<uint8_t, MergeItem> m_mergeMap;	//<mergeTplId, MergeItem>    

	} mergeCfg;
};


}

#endif

