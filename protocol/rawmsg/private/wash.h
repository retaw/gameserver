/*
 * Author: zhupengfei
 *
 * Created: 2015-08-07 15:43 +0800
 *
 * Modified: 2015-08-07 14:43 +0800
 *
 * Description: 洗练相关消息
 */

#ifndef PROTOCOL_RAWMSG_PRIVATE_WASH_MSG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_WASH_MSG_HPP

#include "water/common/roledef.h"
#include "processes/world/pkdef.h"

#pragma pack(1)

namespace PrivateRaw{

//world -> db
//更新或插入洗练
struct UpdateOrInsertWashProp
{
	RoleId roleId;
	SceneItemType sceneItem;
	uint8_t washType;

	ArraySize size = 0;
	WashPropInfo data[0];
};


}

#pragma pack()


#endif
