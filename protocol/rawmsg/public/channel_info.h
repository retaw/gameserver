/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-06 17:45 +0800
 *
 * Modified: 2015-05-06 17:45 +0800
 *
 * Description:  频道
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_CHANNEL_INFO_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_CHANNEL_INFO_MSG_H


#include "water/common/commdef.h"
#include "water/common/roledef.h"
#include "water/common/channeldef.h"

#pragma pack(1)

namespace PublicRaw{ 

//c -> s
//端通过非系统频道，请求发送消息
struct NormalChannelMsgFromClient
{
	RoleId listenerId = 0;	//发送私聊消息时，需要赋值接收者
    ChannelType type;
	ArraySize textSize = 0;
	ArraySize propSize = 0;
    char text[0];
	
	struct FromPropList
	{
		TplId     tplId		 = 0;	//物品tplId 
		uint32_t nonsuchProp = 0;	//极品属性
		uint8_t  luckyLevel	 = 0;	//武器幸运等级
		uint8_t  strongLevel = 0;	//强化等级
	} data[0];
};

//s -> c
//返回非系统频道消息
struct NormalChannelMsgToClient
{
    ChannelType type;
    RoleId talkerId;
    char talkerName[NAME_BUFF_SZIE];
    ArraySize textSize = 0;
	ArraySize propSize = 0;
    char text[0];

	struct ToPropList
	{
		TplId     tplId		 = 0;	//物品tplId 
		uint32_t nonsuchProp = 0;	//极品属性
		uint8_t  luckyLevel	 = 0;	//武器幸运等级
		uint8_t  strongLevel = 0;	//强化等级
	} data[0];
};

//s -> c
//返回系统频道消息
struct SystemChannelMsgToClient
{
	ChannelType type;
	ArraySize textSize = 0;
	char text[0];
};

//c -> s
//端发来的GM指令
struct GmMsgFromClient
{
	ArraySize textSize;
	char text[0];
};


}

#pragma pack()



#endif

