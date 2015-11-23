/*
 * Author: zhupengfei
 *
 * Created: 2015-09-06 13:40 +0800
 *
 * Modified: 2015-09-06 13:40 +0800
 *
 * Description: 角色、英雄相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_ZHUAN_SHENG_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_ZHUAN_SHENG_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求转生
struct RequestZhuansheng
{
	SceneItemType sceneItem;
};

//s -> c
//返回转生结果
struct RetZhuanshengResult
{
	SceneItemType sceneItem;
	OperateRetCode code;	//1成功	2失败 3材料不足
};

}

#pragma pack()


#endif
