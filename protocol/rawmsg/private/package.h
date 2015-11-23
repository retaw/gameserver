#ifndef PROTOCOL_RAWMSG_PRIVATE_PACKAGE_MSG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_PACKAGE_MSG_HPP

#include "water/common/roledef.h"

#include <stdint.h>

#pragma pack(1)

namespace PrivateRaw{


//world ->db
//返回修改物品数据
struct RetModifyObjData
{
	RoleId roleId = 0;
	ArraySize objListSize;

	struct ModifyObj
	{
		uint64_t objId;			//相当于role唯一的objId
		ModifyType modifyType;
		PackageType packageType;
		uint16_t cell;
		uint32_t tplId;
		uint16_t item;
		uint32_t skillId;		//极品装备触发的被动技能Id(几率触发)
		Bind bind;
        uint32_t sellTime;
		uint8_t strongLevel;	//强化等级
		uint8_t luckyLevel;		//武器幸运等级
	} data[0];
};

//world -> db
//更新背包解锁格子数
struct UpdatePackageUnlockCellNum
{
	RoleId roleId;
	uint16_t unlockCellNumOfRole = 0;
	uint16_t unlockCellNumOfHero = 0;
	uint16_t unlockCellNumOfStorage = 0;
};

//world -> db
//请求指定背包类型的物品列表 (英雄使用)
struct RequestObjListByPackageType
{
	RoleId roleId;
	PackageType packageType;
};

//db -> world
//返回指定背包类型的物品列表 (英雄使用)
struct RetObjListByPackageType
{
	RoleId roleId;
	PackageType packageType;
	ArraySize size = 0;
	RoleObjData::ObjData data[0];
};

}


#pragma pack()

#endif
