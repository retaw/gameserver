/*
 * Author: zhupengfei
 *
 * Created: 2015-06-02 10:15 +0800
 *
 * Modified: 2015-06-02 10:15 +0800
 *
 * Description: 
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_OBJECT_SCENE_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_OBJECT_SCENE_MSG_H

#include "water/common/commdef.h"

#pragma pack(1)

namespace PublicRaw{


//s -> c
//更新视野中object的数据
struct ObjectScreenData
{
    ObjectId objId;		//相对于scene唯一的物品Id
    TplId tplId;
    int16_t posX;
    int16_t posY;
};

//s -> c
//周围全部object的9屏数据
struct ObjectsAroundMe
{
    ArraySize size = 1;
    ObjectScreenData objects[1];
};

//s -> c
//object离开role的视野
struct ObjectLeaveInfo
{
    ArraySize size = 1;
    ObjectId objId[1];
};

//c -> s
//请求拾取物品
struct RequestPickupObject
{
	ObjectId objId;
};

}

#pragma pack()

#endif
