/*
 * Author: zhupengfei
 *
 * Created: 2015-07-24 14:30 +0800
 *
 * Modified: 2015-07-24 14:30 +0800
 *
 * Description: 称号相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_TITLE_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_TITLE_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求称号列表
struct RequestTitleList
{
};

//s -> c
//返回称号列表
struct RetTitleList
{
	ArraySize size;
	
	struct TitleItem
	{
		uint32_t titleId;
		uint32_t sec;	//剩余时间 (uint32_t)-1 表示永久
	} data[0];
};

//c -> s
//请求操作普通称号
struct RequestOperateNormalTitle
{
	uint32_t type = 0;		//1佩戴 2取下
	uint32_t titleId = 0;
};

//s -> c
//返回获得的普通称号
struct RetGotNormalTitle
{
	uint32_t titleId = 0;
};


}

#pragma pack()


#endif
