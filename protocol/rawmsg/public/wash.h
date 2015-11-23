/*
 * Author: zhupengfei
 *
 * Created: 2015-08-05 15:30 +0800
 *
 * Modified: 2015-08-05 15:30 +0800
 *
 * Description: 洗练相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_WASH_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_WASH_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

struct WashPropItem
{
	PropertyType propType = PropertyType::none;
	uint32_t prop = 0;
	uint8_t quality = 0;
};

//c -> s
//请求属性列表
struct RequestPropList
{
	SceneItemType sceneItem;		//1角色 2英雄
	uint8_t type = 0;	//1初 2中 3高 4特 5神
};

//s -> c
//返回属性列表
struct RetPropList
{
	SceneItemType sceneItem;
	uint8_t type = 0;
	
	ArraySize size = 0;
	WashPropItem data[0];
};

//c -> s
//请求锁定或解锁属性
struct RequestLockOrUnlockProp
{
	SceneItemType sceneItem;
	uint8_t type = 0;	
	uint8_t group = 0;
	bool lock = false;
};

//c -> s
//请求洗练
struct RequestWash
{
	SceneItemType sceneItem;
	uint8_t type = 0;	
	uint8_t washWay = 0;	
};

//s -> c
//返回洗练结果
struct RetWashPropResult
{
	SceneItemType sceneItem;
	uint8_t type = 0;
	
	ArraySize size = 0;
	WashPropItem data[0];
};

//c -> s
//请求替换属性
struct RequestReplaceCurProp
{
	SceneItemType sceneItem;
	uint8_t type = 0;
};


}

#pragma pack()


#endif
