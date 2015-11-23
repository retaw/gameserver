/*
 * Author: zhupengfei
 *
 * Created: 2015-07-01 14:26:00 +0800
 *
 * Modified: 2015-07-01 14:26:00 +0800
 *
 * Description: 加载背包配置文件
 */

#ifndef PROCESS_WORLD_PACKAGE_CONFIG_HPP
#define PROCESS_WORLD_PACKAGE_CONFIG_HPP

#include "water/componet/xmlparse.h"

#include <map>

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class PackageConfig
{
public:
	~PackageConfig() = default;
	static PackageConfig& me();
private:
	static PackageConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Package 
	{
		void load(componet::XmlParseNode root);
		void clear();

		uint32_t sec = 0;	
		uint32_t yuanbao = 0;
		uint32_t needTplId = 0;

		struct Role
		{
			uint16_t cell;
			uint32_t needOnlineSec;
			uint32_t addExp;
		};

		struct Storage
		{
			uint16_t cell;
			uint16_t needNum;
			uint32_t addExp;
		};

		std::map<uint16_t, Role> m_roleMap;			//<cell, Role>    
		std::map<uint16_t, Storage> m_storageMap;	//<cell, Storage>

	} packageCfg;

};

}

#endif

