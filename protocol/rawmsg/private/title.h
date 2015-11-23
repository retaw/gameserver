/*
 * Author: zhupengfei
 *
 * Created: 2015-07-24 14:30 +0800
 *
 * Modified: 2015-07-24 14:30 +0800
 *
 * Description: 称号相关消息
 */

#ifndef PROTOCOL_RAWMSG_PRIVATE_TITLE_MSG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_TITLE_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

//world -> db
//更新称号
struct UpdateOrInsertTitle
{
	RoleId roleId;

	ArraySize size = 0;
	TitleInfo data[0];
};


}

#pragma pack()


#endif
