/*
 * Author: zhupengfei
 *
 * Created: 2015-05-19 +0800
 *
 * Modified: 2015-05-19 +0800
 *
 * Description: 加载死亡复活配置文件
 */

#ifndef PROCESS_WORLD_RELIVE_CONFIG_HPP
#define PROCESS_WORLD_RELIVE_CONFIG_HPP

#include "world.h"
#include "water/componet/xmlparse.h"
#include "water/common/scenedef.h"
#include "water/componet/coord.h"

#include <map>

class XmlParseNode;

namespace world{

using water::componet::Coord2D;
class ReliveConfig
{
public:
	~ReliveConfig() = default;
	static ReliveConfig& me();
private:
	static ReliveConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Relive 
	{
		uint32_t mapId = 0;
		uint16_t posX = 0;
		uint16_t posY = 0;
		uint32_t sec = 0;

		struct OldPlaceRelive
		{
			uint8_t type;		//原地复活类型
			uint32_t tplId;
			uint32_t needNum;
			uint32_t needMoney;
			uint32_t percent;
		};

		std::map<uint32_t, OldPlaceRelive> reliveMap;

		void load(componet::XmlParseNode root);

	} reliveCfg;

    //主城id
    SceneId cityId() const;
    //主城复活点坐标
    Coord2D cityRelivePos() const;

};


}

#endif

