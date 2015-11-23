#ifndef PROCESS_WORLD_MAP_BASE_H
#define PROCESS_WORLD_MAP_BASE_H

#include "water/common/scenedef.h"
#include "water/componet/class_helper.h"
#include "pkdef.h"
#include "water/common/roledef.h"

#include <unordered_map>

namespace world{

//副本类型
enum CopyMap : uint8_t
{
    none            = 0, //表示静态地图
    world_boss      = 1, //世界boss
	bubble_point	= 2, //激情泡点
	first_pk		= 3, //天下第一
    private_boss    = 4, //个人boss
    boss_home       = 5, //boss之家
    shabake         = 6, //沙巴克
};

//地图配置表
struct MapTpl
{
    TYPEDEF_PTR(MapTpl)
    CREATE_FUN_MAKE(MapTpl)

    MapId  id;
    CopyMap type;    //
    uint8_t crime;      //0:非法攻击不增加罪恶值 1:增加
    attack_mode mode;   //强制攻击模式
    uint8_t ride;       //0:可以骑乘 1:不可以骑乘
    uint8_t dropEquip;  //0:pk中死亡不爆装备  1:爆装备
    uint16_t reliveyb;  //出生点复活消耗的元宝
    TurnLife roleMinTurnLife;   //进入地图的转生等级限制
    uint32_t roleMinLevel;  //进入该地图的最低等级要求
    uint32_t objTplId;  //进入该地图的消耗道具
    uint16_t objNum;    //进入该地图消耗的道具数量
};


class MapBase 
{
public:
    ~MapBase() = default;

private:
    MapBase() = default;

public:
    void loadConfig(const std::string& cfgdir);
    MapTpl::Ptr getMapTpl(MapId);

    static MapBase& me();

private:
    std::unordered_map<MapId, MapTpl::Ptr> m_mapTpls;
};

}

#endif

