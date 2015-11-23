/*
 * Author: zhupengfei
 *
 * Created: 2015-10-14 14:30 +0800
 *
 * Modified: 2015-10-14 14:30 +0800
 *
 * Description: 篝火相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_BONFIRE_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_BONFIRE_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求篝火队列信息
struct RequestBonfireTeamInfo
{
	TriggerId triggerId;
};

//s -> c
//返回篝火队列信息
struct RetBonfireTeamInfo
{
	TriggerId triggerId;
	uint32_t sec;		//剩余时间
	uint8_t wine;		//0未喝酒、1十年 2百年 3千年女儿红

	ArraySize size = 0;
	struct TeamItem
	{	
		Job job;
		Sex sex;
		char name[NAME_BUFF_SZIE];
	} data[0];
};

//c -> s
//请求篝火主人名字
struct RequestBonfireOwnerName
{
	TriggerId triggerId;
};

//s -> c
//返回篝火注意名字
struct RetBonfireOwnerName
{
	TriggerId triggerId;
	char ownerName[NAME_BUFF_SZIE];
};

//c -> s
//请求加入篝火
struct RequestJoinBonfire
{
	TriggerId triggerId;
};

//s ->c
//通知玩家加入的篝火已结束
struct NotifyBonfireEnd
{
	bool end;
};

//s ->c
//返回喝酒结果
struct RetDrinkWineResult
{
	uint8_t wine;
};


}

#pragma pack()


#endif
