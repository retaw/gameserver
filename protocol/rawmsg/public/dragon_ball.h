/*
 * Author: zhupengfei
 *
 * Created: 2015-08-25 14:40 +0800
 *
 * Modified: 2015-08-25 14:40 +0800
 *
 * Description: 龙珠相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_DRAGON_BALL_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_DRAGON_BALL_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求龙珠信息
struct RequestDragonBallInfo
{
};

//s -> c
//返回龙珠信息
struct RetDragonBallInfo
{
	ArraySize size = 0;

	struct DragonInfo
	{
		uint8_t type;
		uint8_t level;
		uint32_t exp;
	} data[0];
};

//c -> s
//请求升级龙珠
struct RequestLevelUpDragonBall
{
	uint8_t type = 0;		
};

//s -> c
//返回龙珠升级结果
struct RetDragonBallLevelUpResult
{
	uint8_t type;
	uint8_t oldLevel;
	uint8_t newLevel;
	uint32_t exp;
};


}

#pragma pack()


#endif
