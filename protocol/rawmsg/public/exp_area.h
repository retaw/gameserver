/*
 * Author: zhupengfei
 *
 * Created: 2015-09-15 15:10 +0800
 *
 * Modified: 2015-09-15 15:10 +0800
 *
 * Description: 经验区相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_EXP_AREA_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_EXP_AREA_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求自动加经验列表
struct RequestAutoAddExpList
{
};

//s -> c
//返回自动加经验列表
struct RetAutoAddExpList
{
	ArraySize size = 0;

	struct AutoExpItem
	{
		uint8_t type;
		uint32_t sec;	//剩余秒数
	} data[0];
};

//c -> s
//请求开启自动加经验
struct RequestOpenAutoAddExp
{
	uint8_t type = 0;	//类型
};

//s -> c
//返回成功开启启动加经验
struct RetOpenAutoAddExpSucess
{
	uint8_t type = 0;
};

//c -> s
//请求关闭自动加经验
struct RequestCloseAutoAddExp
{
};

//s -> c
//返回自动加经验数
struct RetAutoAddExp
{
	uint64_t exp = 0;
};

//s -> c
//服务器主动中断自动加经验
struct RetServerBreakAutoAddExp
{
};


}

#pragma pack()


#endif
