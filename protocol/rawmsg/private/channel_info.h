#ifndef PROTOCOL_RAWMSG_PRIVATE_CHANNEL_HPP
#define PROTOCOL_RAWMSG_PRIVATE_CHANNEL_HPP

#include "water/common/commdef.h"
#include "water/common/channeldef.h"
#include "water/common/roledef.h"

#include <stdint.h>

#pragma pack(1)

namespace PrivateRaw{

//world -> func
//系统公告，通知全服
struct SendSysNotifyToGlobal
{
	ChannelType type;
	ArraySize textSize = 0;
	char text[0];
};

//func -> worlds 
//将端发来的GM指令，广播到各个world
struct BroadcastGmMsgToGlobal
{
	RoleId roleId;
	ArraySize textSize;
	char text[0];
};

}


#pragma pack()

#endif
