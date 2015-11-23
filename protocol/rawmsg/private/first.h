/*
 * Author: zhupengfei
 *
 * Created: 2015-10-09 11:40 +0800
 *
 * Modified: 2015-10-09 11:40 +0800
 *
 * Description: 天下第一相关消息
 */

#ifndef PROTOCOL_RAWMSG_PRIVATE_FIRST_MSG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_FIRST_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

//world -> func
//更新天下第一获得者信息
struct UpdateFirstPlayerInfo
{
	RoleId roleId;
	char name[NAME_BUFF_SZIE];
	Job job;
	Sex sex;
};


}

#pragma pack()


#endif
