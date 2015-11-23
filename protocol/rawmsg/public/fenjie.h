/*
 * Author: zhupengfei
 *
 * Created: 2015-09-06 13:40 +0800
 *
 * Modified: 2015-09-06 13:40 +0800
 *
 * Description: 装备分解相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_FEN_JIE_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_FEN_JIE_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求分解装备
struct RequestFenjie
{
	ArraySize size = 0;
	uint16_t cell[0];
};

//s -> c
//返回装备分解产出
struct RetFenJieReward
{
	ArraySize size = 0;
	
	struct RewardList
	{
		TplId tplId;
		uint32_t num;
		Bind bind;
	} data[0];
};

}

#pragma pack()


#endif
