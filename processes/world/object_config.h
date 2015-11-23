/*
 * Author: zhupengfei
 *
 * Created: 2015-05-23 +0800
 *
 * Modified: 2015-05-23 +0800
 *
 * Description: 加载物品配置文件
 */

#ifndef PROCESS_WORLD_OBJECT_CONFIG_HPP
#define PROCESS_WORLD_OBJECT_CONFIG_HPP

#include "world.h"
#include "water/common/objdef.h"
#include "water/componet/xmlparse.h"

#include <unordered_map>

class XmlParseNode;

namespace world{

class ObjectConfig
{
public:
	~ObjectConfig() = default;
	static ObjectConfig& me();
private:
	static ObjectConfig m_me;

public:
	void loadConfig(const std::string& configDir);
	std::string getName(uint32_t tplId) const;
    bool getObjBasicData(uint32_t tplId, ObjBasicData* data) const;

    //获取物品最大可叠加数量
    uint16_t getMaxStackNum(uint32_t tplId) const;

public:
	struct Object 
	{
		void load(componet::XmlParseNode root);
	
		std::unordered_map<uint32_t, ObjBasicData> m_objBasicDataMap;    //<tplId, ObjBasicData>    

	} objectCfg;

};

}

#endif

