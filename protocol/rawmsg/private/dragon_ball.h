/*
 * Author: zhupengfei
 *
 * Created: 2015-08-26 09:35 +0800
 *
 * Modified: 2015-08-26 09:35 +0800
 *
 * Description: 龙珠相关消息
 */

#ifndef PROTOCOL_RAWMSG_PRIVATE_DRAGON_BALL_MSG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_DRAGON_BALL_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

//world -> db
//更新或插入龙珠数据
struct UpdateOrInsertDragonBallExp
{
	RoleId roleId;
	uint8_t dragonType;
	uint32_t exp;
};


}

#pragma pack()


#endif
