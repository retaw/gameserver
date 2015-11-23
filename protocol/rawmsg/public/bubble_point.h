/*
 * Author: zhupengfei
 *
 * Created: 2015-09-15 15:10 +0800
 *
 * Modified: 2015-09-15 15:10 +0800
 *
 * Description: 激情泡点相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_BUBBLE_POINT_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_BUBBLE_POINT_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//s -> c
//返回特殊泡点信息
struct RetSpecialPointInfo
{
	ArraySize size = 0;
	struct PointItem
	{
		char pointName[NAME_BUFF_SZIE];
		char roleName[NAME_BUFF_SZIE];
	} data[0];
};

//s -> c
//返回发放特殊奖励所需时间
struct RetGetSpecialRewardNeedSec
{
	uint32_t sec;
};

}

#pragma pack()


#endif
