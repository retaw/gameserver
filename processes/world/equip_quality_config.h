/*
 * Author: zhupengfei
 *
 * Created: 2015-08-14 10::20:00 +0800
 *
 * Modified: 2015-08-14 10::20:00 +0800
 *
 * Description: 加载装备升品配置文件
 */

#ifndef PROCESS_WORLD_EQUIP_QUALITY_CONFIG_HPP
#define PROCESS_WORLD_EQUIP_QUALITY_CONFIG_HPP

#include "world.h"
#include "pkdef.h"
#include "water/componet/xmlparse.h"
#include "water/common/roledef.h"

#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class EquipQualityConfig
{
public:
	~EquipQualityConfig() = default;
	static EquipQualityConfig& me();
private:
	static EquipQualityConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct EquipQuality
	{
		void load(componet::XmlParseNode root);
		void clear();

		struct QualityItem
		{
			uint32_t sourceTplId;
			uint32_t destTplId;
			MoneyType needMoneyType;
			uint32_t needMoneyNum;
			uint32_t prob;
			std::vector<std::pair<uint32_t, uint16_t> > needObjVec;	//<needTplId, needNum>
		};

		std::map<uint32_t, QualityItem> m_equipMap;	//<souceTplId, QualityItem>    

	} qualityCfg;
};


}

#endif

