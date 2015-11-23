/*
 * Author: zhupengfei
 *
 * Created: 2015-08-14 10:58 +0800
 *
 * Modified: 2015-08-14 10:58+0800
 *
 * Description: 装备升品相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_EQUIP_QUALITY_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_EQUIP_QUALITY_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求装备升品
struct RequestImproveEquipQuality
{
	uint8_t type = 0;		//1升品	2神装合成
	bool autoBuy = false;
	uint16_t cell = 0;	
	PackageType packageType;
};

//s -> c
//返回装备升品结果
struct RetEquipImproveQualityResult
{
	OperateRetCode code;		//1成功	2失败 3材料不足
};


}

#pragma pack()


#endif
