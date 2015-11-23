/*
 * Author: zhupengfei
 *
 * Created: 2015-08-15 14:40 +0800
 *
 * Modified: 2015-08-15 14:40 +0800
 *
 * Description: 翅膀晋阶、注灵相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_WING_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_WING_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求翅膀晋阶
struct RequestWingLevelUp
{
	bool useYuanbao = false;
	uint16_t cell = 0;
	PackageType packageType;
};

//s -> c
//返回翅膀晋阶结果
struct RetWingLevelUpResult
{
	OperateRetCode code;		//1成功	2失败 3材料不足
};

//c -> s
//请求翅膀注灵
struct RequestWingZhuling
{
	uint8_t type = 0;			//1使用金币	2使用元宝
	uint16_t cell = 0;
	PackageType packageType;
};

//s -> c
//返回翅膀注灵结果
struct RetWingZhulingResult
{
	OperateRetCode code;		//1成功	2失败 3材料不足
};


}

#pragma pack()


#endif
