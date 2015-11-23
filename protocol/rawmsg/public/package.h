/*
 * Author: zhupengfei
 *
 * Created: 2015-05-25 11:49 +0800
 *
 * Modified: 2015-05-25 11:49 +0800
 *
 * Description: 包裹相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_PACKAGE_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_PACKAGE_MSG_HPP

#include "water/common/roledef.h"
#include "water/common/objdef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求背包物品列表
struct RequestPackageObjList
{
	PackageType packageType;    //背包类型  1角色 2英雄 3仓库
};

//s -> c
//返回背包物品列表
struct RetPackageObjList
{
	ArraySize objListSize;
	PackageType packageType;		//背包类型  

	struct ObjectData
	{
		TplId tplId;				//物品tplId
		uint16_t item;              //物品数量
		uint16_t cell;              //格子编号
		uint32_t skillId;			//极品装备触发的被动技能Id
		uint8_t strongLevel;		//强化等级
		uint8_t luckyLevel;			//武器幸运等级
		Bind bind;					//1非绑 2绑定
		uint8_t fixed;				//0非锁定	1锁定
		ObjParentType parentType;	//1普通	2装备 3宝石	4药品	
	} data[0];
};

//s -> c
//返回背包变动的物品列表
struct RetPackageModifyObjList
{
	TplId tplId;				//物品tplId
	uint16_t item;              //物品数量
	uint16_t cell;              //格子编号
	uint32_t skillId;			//极品装备触发的被动技能Id
	uint8_t strongLevel;		//强化等级
	uint8_t luckyLevel;			//武器幸运等级
	Bind bind;					//1非绑 2绑定
	uint8_t fixed;				//0非锁定	1锁定
	ObjParentType parentType;	//1普通	2装备 3宝石	4药品
	PackageType packageType;	//背包类型  
	bool bNewObj = false;		//是否新获得的物品
};

//s -> c
//返回装备包物品列表(九屏消息)
struct RetPackageObjListToNine
{
	PKId id;					//RoleId 或 HeroId
	ArraySize objListSize = 3;
	PackageType packageType;	//背包类型  
	TplId tplId[3];				//物品tplId
};

//c -> s
//请求背包已解锁的格子数
struct RequestPackageUnlockCellNum
{
	PackageType packageType;    //背包类型  
};

//s -> c
//返回背包已解锁格子数
struct RetPackageUnlockCellNum
{
	uint16_t unlockCellNUm = 0;
	PackageType packageType;    //背包类型  
};

//c -> s
//请求摧毁一定数量的某类型物品(且指定格子)
struct RequestDestoryObjByCell
{
	uint16_t cell = 0;
	uint16_t num = 0;
	PackageType packageType;
};

//c -> s
//请求丢弃一定数量的某类型物品(且指定格子)
struct RequestDiscardObjByCell
{
	uint16_t cell = 0;
	uint16_t num = 0;
	PackageType packageType;
};

//c -> s
//请求拆分物品数量
struct RequestSplitObjNum
{
	uint16_t cell = 0;
	uint16_t num = 0;	//要拆分的数量
	PackageType packageType;
};

//c -> s
//请求交换格子
struct RequestExchangeCell
{
	uint16_t fromCell = 0;
	uint16_t toCell = 0;
	PackageType fromPackage;
	PackageType toPackage;
};

//c -> s
//请求存取物品, 跨背包
struct RequestMoveObj 
{
	uint16_t fromCell = 0;
	PackageType fromPackage;
	PackageType toPackage;
};

//c -> s
//请求整理物品
struct RequestSortObj
{
	PackageType packageType;
};

//c -> s
//请求出售道具
struct RoleRequestSellObj
{
    uint16_t fromCell   = 0;   //格子编号
    uint16_t num        = 0;
    PackageType fromPackageType;
};


//c -> s
//请求回购某件商品
struct RoleRequestRepurchaseObj
{
    uint16_t fromCell   = 0;    //位置
};

//请求正在解锁的格子剩余解锁时间
//c -> s
struct RequestUnlockCellNeedSec
{
};

//s -> c
//返回正在解锁的格子剩余解锁时间(主背包)
struct RetUnlockCellNeedSec
{
	uint16_t cell = 0;
	uint32_t needSec = 0;
	uint16_t unlockCellNUm = 0;
};

//c -> s
//请求解锁格子需要的元宝数
struct RequestUnlockCellNeedMoney
{
	uint16_t num = 0;	//请求解锁N个格子
	PackageType packageType;
};

//s -> c
//返回解锁格子需要的元宝数
struct RetUnlockCellNeedMoney
{
	uint16_t num = 0;
	uint32_t needMoney = 0;
};

//c -> s
//请求解锁格子
struct RequestUnlockCell
{
	uint16_t num = 0;	//请求解锁N个格子
	PackageType packageType;
};


/******************强化装备*******************/
//c -> s
//请求强化装备
struct RequestStrongEquip
{
	uint16_t cell = 0;
	bool bProtect = false;	//是否使用保护符
	bool autoBuy = false;	//是否自动购买物品
	PackageType packageType;
};

//s -> c
//返回强化结果
struct RetStrongEquipResult
{
	uint16_t cell = 0;
	uint8_t oldLevel = 0;
	uint8_t newLevel = 0;
	PackageType packageType;
};


/******************武器幸运*******************/
//c -> s
//请求武器幸运提升
struct RequestLevelUpWeaponLucky
{
	bool bProtect = false;	//是否使用保护符
	bool autoBuy = false;	//是否自动购买物品
	PackageType packageType;
};

//s -> c
//返回武器幸运提升结果
struct RetLevelUpWeaponLuckyResult
{
	uint8_t oldLevel = 0;
	uint8_t newLevel = 0;
	PackageType packageType;
};


/******************装备融合*******************/
//c -> s
//请求融合装备
struct RequestRongheEquip
{
	bool autoBuy = false;			//是否自动购买材料
	uint16_t sourceCell = 0;		//消耗装备所在格子
	uint16_t destCell = 0;			//目标装备所在格子
	PackageType sourcePackageType;	//消耗装备所在背包	
	PackageType destPackageType;	//目标装备所在背包
};

//s -> c
//返回融合结果
struct RetEquipRongheResult
{
	OperateRetCode code;		//1成功	2失败 3材料不足
};


}

#pragma pack()


#endif
