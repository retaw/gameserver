#include "role_package.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/package.h"
#include "protocol/rawmsg/public/package.codedef.public.h"

#include "protocol/rawmsg/public/object_scene.h"
#include "protocol/rawmsg/public/object_scene.codedef.public.h"

#include "protocol/rawmsg/private/package.h"
#include "protocol/rawmsg/private/package.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

RolePackage RolePackage::m_me;

RolePackage& RolePackage::me()
{
	return m_me;
}


void RolePackage::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestPackageObjList, std::bind(&RolePackage::clientmsg_RequestObjList, this, _1, _2, _3));
	
	REG_RAWMSG_PUBLIC(RequestPackageUnlockCellNum, std::bind(&RolePackage::clientmsg_RequestUnlockCellNum, this, _1, _2, _3));
	
	REG_RAWMSG_PUBLIC(RequestDestoryObjByCell, std::bind(&RolePackage::clientmsg_RequestDestoryObjByCell, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestDiscardObjByCell, std::bind(&RolePackage::clientmsg_RequestDiscardObjByCell, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestSplitObjNum, std::bind(&RolePackage::clientmsg_RequestSplitObjNum, this, _1, _2, _3));
	
	REG_RAWMSG_PUBLIC(RequestExchangeCell, std::bind(&RolePackage::clientmsg_RequestExchangeCell, this, _1, _2, _3));
	
	REG_RAWMSG_PUBLIC(RequestMoveObj, std::bind(&RolePackage::clientmsg_RequestMoveObj, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestSortObj, std::bind(&RolePackage::clientmsg_RequestSortObj, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RoleRequestSellObj, std::bind(&RolePackage::clientmsg_RoleRequestSellObj, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RoleRequestRepurchaseObj, std::bind(&RolePackage::clientmsg_RoleRequestRepurchaseObj, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestUnlockCellNeedSec, std::bind(&RolePackage::clientmsg_RequestUnlockCellNeedSec, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestUnlockCellNeedMoney, std::bind(&RolePackage::clientmsg_RequestUnlockCellNeedMoney, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestUnlockCell, std::bind(&RolePackage::clientmsg_RequestUnlockCell, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestPickupObject, std::bind(&RolePackage::clientmsg_RequestPickupObject, this, _1, _2, _3));

}


//请求背包物品列表
void RolePackage::clientmsg_RequestObjList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestPackageObjList*>(msgData);
	if(!rev)
		return;

	//hero包裹集
	if(isBelongHeroPackage(rev->packageType))
	{
		Job job = role->getHeroJobByPackageType(rev->packageType);
		if(job == Job::none)
			return;

		Hero::Ptr hero = role->getHeroByJob(job);
		if(hero == nullptr)
			return;

		hero->m_packageSet.sendObjList(rev->packageType);
		return;
	}

	role->m_packageSet.sendObjList(rev->packageType);
}

//请求背包已解锁的格子数
void RolePackage::clientmsg_RequestUnlockCellNum(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestPackageUnlockCellNum*>(msgData);
	if(!rev)
		return;

	role->m_packageSet.sendUnlockCellNum(rev->packageType);
}

//请求摧毁一定数量的某类型物品, 且指定格子 
void RolePackage::clientmsg_RequestDestoryObjByCell(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestDestoryObjByCell*>(msgData);
	if(!rev)
		return;
	
	role->m_packageSet.requestDestoryObjByCell(rev->cell, rev->num, rev->packageType);
}

//请求丢弃一定数量的某类型物品(且指定格子)
void RolePackage::clientmsg_RequestDiscardObjByCell(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestDiscardObjByCell*>(msgData);
	if(!rev)
		return;
	
	role->m_packageSet.requestDiscardObjByCell(rev->cell, rev->num, rev->packageType);
}

//请求拆分物品数量
void RolePackage::clientmsg_RequestSplitObjNum(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestSplitObjNum*>(msgData);
	if(!rev)
		return;

	role->m_packageSet.requestSplitObjNum(rev->cell, rev->num, rev->packageType);
}

//请求交换格子, 支持同一背包内及跨背包
void RolePackage::clientmsg_RequestExchangeCell(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestExchangeCell*>(msgData);
	if(!rev)
		return;
	
	//role包裹集
	if(!isBelongHeroPackage(rev->fromPackage) && !isBelongHeroPackage(rev->toPackage))
	{
		role->m_packageSet.requestExchangeCell(rev->fromCell, rev->toCell, rev->fromPackage, rev->toPackage);
		return;
	}
	else if(isHeroStonePackage(rev->toPackage))	//目标英雄的宝石背包, 特殊处理
	{
		moveObjToHeroStonePackage(roleId, rev->fromCell, rev->toCell, rev->fromPackage, rev->toPackage);
	}
	else 
	{	//跨背包集移动物品, hero及role包裹集
		moveObjCrossPackageSet(roleId, rev->fromCell, rev->toCell, rev->fromPackage, rev->toPackage);
	}

	return;
}


//请求存取物品，跨背包
void RolePackage::clientmsg_RequestMoveObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestMoveObj*>(msgData);
	if(!rev)
		return;

	//role包裹集
	if(!isBelongHeroPackage(rev->fromPackage))
	{
		role->m_packageSet.requestMoveObj(rev->fromCell, rev->fromPackage, rev->toPackage);
		return;
	}
	else
	{//hero包裹集
		if(rev->toPackage != PackageType::role)
			return;

		Job job = role->getHeroJobByPackageType(rev->fromPackage);
		if(job == Job::none)
			return;

		Hero::Ptr hero = role->getHeroByJob(job);
		if(hero == nullptr)
			return;

		Package::Ptr fromPackagePtr = hero->m_packageSet.getPackageByPackageType(rev->fromPackage);
		Package::Ptr toPackagePtr = role->m_packageSet.getPackageByPackageType(rev->toPackage);
		if(fromPackagePtr == nullptr || toPackagePtr == nullptr)
			return;

		Object::Ptr obj = fromPackagePtr->getObjByCell(rev->fromCell);
		uint16_t item = fromPackagePtr->getObjNumByCell(rev->fromCell);
		Bind bind = fromPackagePtr->getBindByCell(rev->fromCell);
		if(obj == nullptr|| 1 != item || bind == Bind::none)
			return;

		if(!toPackagePtr->checkPutObj(obj, item, bind))
			return;

		if(nullptr == fromPackagePtr->eraseObjByCell(rev->fromCell, "跨背包存取", false, false))
			return;

		if(0 == toPackagePtr->putObj(obj, item, bind, false))
		{
			fromPackagePtr->putObjByCell(obj, rev->fromCell, item, bind, false);
			return;
		}

		LOG_TRACE("背包, 存取物品, 跨背包, hero->role, 成功, tplId={}, item={}, fromCell={}, package:{}->{}",
				  obj->tplId(), item, rev->fromCell, rev->fromPackage, rev->toPackage);
		return;
	}
}

//请求整理物品
void RolePackage::clientmsg_RequestSortObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestSortObj*>(msgData);
	if(!rev)
		return;

	role->m_packageSet.requestSortObj(rev->packageType);
}

//请求出售道具
void RolePackage::clientmsg_RoleRequestSellObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RoleRequestSellObj*>(msgData);
	role->m_packageSet.requestSellObj(rev->fromCell, rev->num, rev->fromPackageType);
}

//请求回购道具
void RolePackage::clientmsg_RoleRequestRepurchaseObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RoleRequestRepurchaseObj*>(msgData);
	role->m_packageSet.requestRepurchaseObj(rev->fromCell);
}

//请求正在解锁的格子剩余解锁时间
void RolePackage::clientmsg_RequestUnlockCellNeedSec(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestUnlockCellNeedSec*>(msgData);
	if(!rev)
		return;

	role->m_packageSet.sendUnlockCellNeedSec();
}

//请求解锁格子需要的元宝数
void RolePackage::clientmsg_RequestUnlockCellNeedMoney(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestUnlockCellNeedMoney*>(msgData);
	if(!rev)
		return;

	role->m_packageSet.requestUnlockCellNeedMoney(rev->num, rev->packageType);

}

//请求解锁格子
void RolePackage::clientmsg_RequestUnlockCell(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestUnlockCell*>(msgData);
	if(!rev)
		return;

	role->m_packageSet.requestUnlockCell(rev->num, rev->packageType);
}


/************************************* 场景物品begin *************************************/
//请求拾取物品
void RolePackage::clientmsg_RequestPickupObject(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestPickupObject*>(msgData);
	if(!rev)
		return;

	role->m_packageSet.requestPickupObject(rev->objId);
}
/************************************* 场景物品end *************************************/

bool RolePackage::isBelongHeroPackage(PackageType packageType)
{
	if(isHeroEquipPackage(packageType) || isHeroStonePackage(packageType))
		return true;

	return false;
}

bool RolePackage::isHeroEquipPackage(PackageType packageType)
{
	if(packageType == PackageType::equipOfWarrior 
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
		return true;

	return false;
}

bool RolePackage::isHeroStonePackage(PackageType packageType)
{
	if(packageType == PackageType::stoneOfWarrior
	   || packageType == PackageType::stoneOfMagician
	   || packageType == PackageType::stoneOfTaoist)
		return true;

	return false;
}

//跨背包集移动物品
void RolePackage::moveObjCrossPackageSet(RoleId roleId, uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	if(!isBelongHeroPackage(fromPackage) && !isBelongHeroPackage(toPackage))
		return;

	Hero::Ptr hero = role->m_heroManager.getDefaultHero();
	if(hero == nullptr)
		return;

	Package::Ptr fromPackagePtr = nullptr;
	Package::Ptr toPackagePtr = nullptr;
	if(isBelongHeroPackage(fromPackage))
	{
		fromPackagePtr = hero->m_packageSet.getPackageByPackageType(fromPackage);
		toPackagePtr = role->m_packageSet.getPackageByPackageType(toPackage);
	}
	else if(isBelongHeroPackage(toPackage))
	{
		toPackagePtr = hero->m_packageSet.getPackageByPackageType(toPackage);
		fromPackagePtr = role->m_packageSet.getPackageByPackageType(fromPackage);
	}

	if(fromPackagePtr == nullptr || toPackagePtr == nullptr)
		return;

	//必须放在eraseObjByCell之前
	const uint16_t sourceItem = fromPackagePtr->getObjNumByCell(fromCell);
	const uint16_t destItem = toPackagePtr->getObjNumByCell(toCell);
	const Bind sourceBind = fromPackagePtr->getBindByCell(fromCell);
	const Bind destBind = toPackagePtr->getBindByCell(toCell);

	const Object::Ptr sourceObj = fromPackagePtr->eraseObjByCell(fromCell, "跨背包集移动", false, false);
	const Object::Ptr destObj = toPackagePtr->eraseObjByCell(toCell, "跨背包集移动", false, false);
	if(sourceObj == nullptr)
		return;

	if(!toPackagePtr->checkPutObjByCell(sourceObj, toCell, sourceItem, sourceBind))
	{
		//策划要求，若指定格子不能存放成功，则不指定格子存放
		if(!toPackagePtr->checkPutObj(sourceObj, sourceItem, sourceBind))
		{
			//不指定格子依然不可以存放, 则执行回退操作
			fromPackagePtr->putObjByCell(sourceObj, fromCell, sourceItem, sourceBind, false);
			toPackagePtr->putObjByCell(destObj, toCell, destItem, destBind, false);
			return;
		}

		//目标背包放入失败，则执行回退操作
		if(0 == toPackagePtr->putObj(sourceObj, sourceItem, sourceBind, false))
		{
			fromPackagePtr->putObjByCell(sourceObj, fromCell, sourceItem, sourceBind, false);
			toPackagePtr->putObjByCell(destObj, toCell, destItem, destBind, false);
			return;
		}
		fromPackagePtr->putObjByCell(destObj, fromCell, destItem, destBind, false);
		return;
	}

	//目标背包放入失败，则执行回退操作
	if(0 == toPackagePtr->putObjByCell(sourceObj, toCell, sourceItem, sourceBind, false))
	{
		fromPackagePtr->putObjByCell(sourceObj, fromCell, sourceItem, sourceBind, false);
		toPackagePtr->putObjByCell(destObj, toCell, destItem, destBind, false);
		return;
	}

	fromPackagePtr->putObjByCell(destObj, fromCell, destItem, destBind, false);
	return;
}

//将物品移到英雄装备包, 特殊处理
void RolePackage::moveObjToHeroStonePackage(RoleId roleId, uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	if(!isHeroStonePackage(toPackage) || fromPackage != PackageType::role)
		return;

	Job job = role->getHeroJobByPackageType(toPackage);
	if(job == Job::none)
		return;

	Hero::Ptr hero = role->getHeroByJob(job);
	if(hero == nullptr)
		return;

	Package::Ptr fromPackagePtr = role->m_packageSet.getPackageByPackageType(fromPackage);
	Package::Ptr toPackagePtr = hero->m_packageSet.getPackageByPackageType(toPackage);
	if(fromPackagePtr == nullptr || toPackagePtr == nullptr)
		return;

	//必须放在eraseObjByCell之前
	const uint16_t eraseSourceItem = 1; //镶嵌宝石，一个目标格子尽可镶嵌一个, 特殊处理 
	const uint16_t destItem = toPackagePtr->getObjNumByCell(toCell);
	const Bind sourceBind = fromPackagePtr->getBindByCell(fromCell);
	const Bind destBind = toPackagePtr->getBindByCell(toCell);

	const Object::Ptr sourceObj = fromPackagePtr->eraseObjByCell(fromCell, eraseSourceItem, "移动", false, false);
	const Object::Ptr destObj = toPackagePtr->eraseObjByCell(toCell, "移动", false, false);
	if(sourceObj == nullptr)
		return;

	if(!toPackagePtr->checkPutObjByCell(sourceObj, toCell, eraseSourceItem, sourceBind))
	{
		fromPackagePtr->putObjByCell(sourceObj, fromCell, eraseSourceItem, sourceBind, false);
		toPackagePtr->putObjByCell(destObj, toCell, destItem, destBind, false);
		return;
	}

	if(destObj != nullptr && !fromPackagePtr->checkPutObjByCell(destObj, fromCell, destItem, destBind))
	{
		fromPackagePtr->putObjByCell(sourceObj, fromCell, eraseSourceItem, sourceBind, false);
		toPackagePtr->putObjByCell(destObj, toCell, destItem, destBind, false);
		return;
	}

	//此处不可直接使用obj指针存放物品, 否则丢失物品
	if(0 == hero->m_packageSet.putObjByCell(sourceObj->tplId(), toCell, eraseSourceItem, sourceBind, toPackage, sourceObj->skillId(), sourceObj->strongLevel(), sourceObj->luckyLevel(), false))
	{
		//若目标背包放入失败，则执行回退操作
		fromPackagePtr->putObjByCell(sourceObj, fromCell, eraseSourceItem, sourceBind, false);
		toPackagePtr->putObjByCell(destObj, toCell, destItem, destBind, false);
		return;
	}

	fromPackagePtr->putObjByCell(destObj, fromCell, destItem, destBind, false);
	return;
}



}
