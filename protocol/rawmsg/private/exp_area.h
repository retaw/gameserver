/*
 * Author: zhupengfei
 *
 * Created: 2015-09-16 14:35 +0800
 *
 * Modified: 2015-09-16 14:35 +0800
 *
 * Description: 经验区相关消息
 */

#ifndef PROTOCOL_RAWMSG_PRIVATE_EXP_AREA_MSG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_EXP_AREA_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

//world -> db
//更新或插入经验类型时间数据
struct UpdateOrInsertExpSec
{
	RoleId roleId;
	uint8_t type;
	uint32_t sec;
};


}

#pragma pack()


#endif
