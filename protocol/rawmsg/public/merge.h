/*
 * Author: zhupengfei
 *
 * Created: 2015-08-13 19:40 +0800
 *
 * Modified: 2015-08-13 19:40 +0800
 *
 * Description: 合成相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_MERGE_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_MERGE_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求合成物品
struct RequestMergeObj
{
	uint32_t mergeTplId = 0;
	uint8_t num = 0;			//合成数量
};

//s -> c
//返回物品合成结果
struct RetObjMergeResult
{
	OperateRetCode code;		//1成功	2失败 3材料不足
};


}

#pragma pack()


#endif
