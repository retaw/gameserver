/*
 * Author: zhupengfei
 *
 * Created: 2015-08-26 16:40 +0800
 *
 * Modified: 2015-08-26 16:40 +0800
 *
 * Description: 使用道具相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_USE_OBJECT_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_USE_OBJECT_MSG_HPP

#include "water/common/roledef.h"
#include "water/common/objdef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求使用道具
struct RequestUseObject
{
	SceneItemType sceneItem;
	uint16_t cell;
	uint16_t num;
};

}

#pragma pack()


#endif
