/*
 * Author: zhupengfei
 *
 * Created: 2015-07-22 14:05 +0800
 *
 * Modified: 2015-07-22 14:05 +0800
 *
 * Description: 官职相关消息
 */

#ifndef PROTOCOL_RAWMSG_PRIVATE_GUANZHI_MSG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_GUANZHI_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

//word -> db
//更新官职等级
struct UpdateGuanzhiLevelToDB
{
	RoleId roleId = 0;
	uint8_t level = 0;
};

}

#pragma pack()


#endif
