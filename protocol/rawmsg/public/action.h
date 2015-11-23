/*
 * Author: zhupengfei
 *
 * Created: 2015-09-25 13:40 +0800
 *
 * Modified: 2015-09-25 13:40 +0800
 *
 * Description: 活动相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_ACTION_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_ACTION_MSG_HPP

#include "water/common/roledef.h"
#include "water/common/actiondef.h"

#pragma pack(1)

namespace PublicRaw{

//s -> c
//返回所有活动的状态
struct RetAllActionState
{
	ArraySize size = 0;
	struct ActionItem
	{
		ActionType type;
		ActionState state;
	} data[0];
};

//c -> s
//请求距离活动结束的时间
struct RequestSpanSecOfActionEnd
{
	ActionType type;
};

//s -> c
//返回距离活动结束的时间
struct RetSpanSecOfActionEnd
{
	ActionType type;
	uint32_t sec;
};

//c -> s
//请求参加活动
struct RequestJoinAction
{
    ActionType type;
};

}

#pragma pack()


#endif
