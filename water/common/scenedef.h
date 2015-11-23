/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-09 19:43 +0800
 *
 * Modified: 2015-04-09 19:43 +0800
 *
 * Description: 场景相关的一些类型和常量定义
 */

#ifndef WATER_COMPONET_SCENEDEF_H
#define WATER_COMPONET_SCENEDEF_H

#include "water/componet/coord.h"

#include <stdint.h>
#include <string>

#pragma pack(1)


typedef uint16_t MapId;
typedef uint64_t SceneId;
// dynamic sceneId:  |- unixtime 32bits  -|-  incnum 16bits -|- mapId 16bits -|
//  static sceneId:  |-  all 0 32bits    -|- worldnum 16bits-|- mapId 16bits -|
//  这样, static sceneId 的值就等于mapId
const SceneId INVALID_SCENE_ID = 0;

inline bool isDynamicSceneId(SceneId sceneId)
{
    return (sceneId & 0xffffffffffff0000) != 0;
}

inline bool isStaticSceneId(SceneId sceneId)
{
    return (sceneId & 0xffffffffffff0000) == 0;
}

inline MapId sceneId2MapId(SceneId sceneId)
{
    return MapId(sceneId & 0xffff);
}

using water::componet::Coord1D;


struct BasicMapInfo
{
    MapId id = 0;
    std::string name;
    int16_t width = 0;
    int16_t height = 0;
};

enum class AreaType : uint16_t
{
    normal        = 0x0000,
    collision     = 0x0001,
    security      = 0x0002,
	exp			  = 0x0004,
};


#pragma pack()

#endif


