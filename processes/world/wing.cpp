#include "wing.h"
#include "wing_config.h"
#include "pk.h"
#include "role.h"
#include "role_manager.h"
#include "object.h"
#include "hero.h"
#include "object_config.h"

#include "water/common/commdef.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/wing.h"
#include "protocol/rawmsg/public/wing.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

Wing::Wing(SceneItemType sceneItem, RoleId roleId, PK& owner)
: m_sceneItem(sceneItem)
, m_roleId(roleId)
, m_owner(owner)
{
}

//请求翅膀晋阶
void Wing::requestWingLevelUp(uint16_t cell, PackageType packageType, bool useYuanbao)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	EquipPackage::Ptr packagePtr = getEquipPackagePtr(packageType);  
	if(nullptr == packagePtr)
		return;

	Object::Ptr sourceObj = packagePtr->getObjByCell(cell);       
	const Bind bind = packagePtr->getBindByCell(cell);
	if(sourceObj == nullptr || Bind::none == bind)
		return;

	if(sourceObj->childType() != ObjChildType::wing)
		return;

	const uint32_t sourceTplId = packagePtr->getTplIdByCell(cell);
	const auto& cfg = WingConfig::me().wingCfg;
	auto pos = cfg.m_levelUpMap.find(sourceTplId);
	if(pos == cfg.m_levelUpMap.end())
		return;

	const uint32_t destTplId = pos->second.destTplId;
	if(0 == destTplId)
		return;

	if(!checkLevel(sourceTplId, destTplId))
		return;

	if(!reduceMaterialOfLevelUp(sourceTplId, useYuanbao))
		return;

	componet::Random<uint32_t> sucess_prob(1, 1000);
	if(sucess_prob.get() > pos->second.prob) //翅膀晋阶失败
	{
		sendWingLevelUpResult(OperateRetCode::failed);
		role->sendSysChat("晋阶失败");
		return;
	}

	if(!packagePtr->eraseObjByCell(cell, "翅膀晋阶"))
		return;

	if(m_sceneItem == SceneItemType::role)
	{
		if(0 == role->putObj(destTplId, 1, bind, packageType))
		{
			LOG_ERROR("翅膀, 晋阶, 放入背包失败, roleId={}, sceneItem={}, sourceTplId{}, destTplId={}, bind={}, cell={}, useYuanbao={}, packageType={}",
					  role->id(), m_sceneItem, sourceTplId, destTplId, 
					  bind, cell, useYuanbao, packageType);
			return;
		}
	}
	else if(m_sceneItem == SceneItemType::hero)
	{
		Hero::Ptr hero = role->getHeroByJob(m_owner.job());
		if(hero == nullptr)
		{
			LOG_ERROR("翅膀, 晋阶, 英雄不存在, 放入背包失败, roleId={}, sceneItem={}, sourceTplId{}, destTplId={}, bind={}, cell={}, useYuanbao={}, packageType={}",
					  role->id(), m_sceneItem, sourceTplId, destTplId, 
					  bind, cell, useYuanbao, packageType);

			return;
		}

		if(0 == hero->m_packageSet.putObj(destTplId, 1, bind, packageType))
		{
			LOG_ERROR("翅膀, 晋阶, 放入背包失败, roleId={}, sceneItem={}, sourceTplId{}, destTplId={}, bind={}, cell={}, useYuanbao={}, packageType={}",
					  role->id(), m_sceneItem, sourceTplId, destTplId, 
					  bind, cell, useYuanbao, packageType);
			return;
		}

	}

	sendWingLevelUpResult(OperateRetCode::sucessful);
	role->sendSysChat("晋阶成功");
	return;
}


void Wing::requestWingZhuling(uint8_t type, uint16_t cell, PackageType packageType)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	EquipPackage::Ptr packagePtr = getEquipPackagePtr(packageType);  
	if(nullptr == packagePtr)
		return;

	Object::Ptr obj = packagePtr->getObjByCell(cell);       
	if(obj == nullptr)
		return;

	if(obj->childType() != ObjChildType::wing)
		return;

	if(!reduceMaterialOfZhuling(type))
		return;

	const auto& cfg = WingConfig::me().wingCfg;
	auto pos = cfg.m_consumeMap.find(type);
	if(pos == cfg.m_consumeMap.end())
		return;

	const uint8_t oldLevel = getLingliLevel(); 
	if(m_sceneItem == SceneItemType::role)
	{
		role->addMoney(MoneyType::money_8, pos->second.rewardLingli, "翅膀注灵");
	}
	else if(m_sceneItem == SceneItemType::hero)
	{
		role->addMoney(MoneyType::money_9, pos->second.rewardLingli, "翅膀注灵");
	}

	const uint8_t newLevel = getLingliLevel();
	if(newLevel > oldLevel)
	{
		packagePtr->sendAllAttributeToMe();
	}

	sendWingZhulingResult(OperateRetCode::sucessful);
	role->sendSysChat("注灵成功");
	return;
}


Role::Ptr Wing::getRole() const 
{
	Role::Ptr role = RoleManager::me().getById(m_roleId);
	if(role == nullptr)
		return nullptr;

	return role;
}

EquipPackage::Ptr Wing::getEquipPackagePtr(PackageType packageType)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return nullptr;

	if(packageType == PackageType::equipOfRole)
	{
		return std::static_pointer_cast<EquipPackage>(role->m_packageSet.getPackageByPackageType(packageType));
	}
	else if(isHeroEquipPackage(packageType))
	{
		Hero::Ptr hero = role->getHeroByJob(m_owner.job());
		if(hero == nullptr)
			return nullptr;

		return std::static_pointer_cast<EquipPackage>(hero->m_packageSet.getPackageByPackageType(packageType)); 
	}

	return nullptr;
}

bool Wing::isHeroEquipPackage(PackageType packageType)
{
	if(packageType == PackageType::equipOfWarrior 
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
		return true;

	return false;
}

bool Wing::checkLevel(TplId sourceTplId, TplId destTplId)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return false;

	const auto& cfg = WingConfig::me().wingCfg;
	auto pos = cfg.m_levelUpMap.find(sourceTplId);
	if(pos == cfg.m_levelUpMap.end())
		return false;

	if(pos->second.needLevel > m_owner.level())
	{
		role->sendSysChat("等级不足");
		return false;
	}

	if(m_sceneItem == SceneItemType::role)
	{
		if(pos->second.needTurnLifeLevel > role->turnLifeLevel())
		{
			role->sendSysChat("转生等级不足");
			return false;
		}
	}
	else if(m_sceneItem == SceneItemType::hero)
	{
		Hero::Ptr hero = role->getHeroByJob(m_owner.job());
		if(hero == nullptr)
			return false;

		if(pos->second.needTurnLifeLevel > hero->turnLifeLevel())
		{
			role->sendSysChat("转生等级不足");
			return false;
		}
	}

	ObjBasicData data;
	if(!ObjectConfig::me().getObjBasicData(destTplId, &data))
		return false;

	if(data.level > m_owner.level())
	{
		role->sendSysChat("达到{}级才能晋阶下一阶", data.level);
		return false;
	}

	return true;
}

bool Wing::reduceMaterialOfLevelUp(TplId sourceTplId, bool useYuanbao)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return false;

	const auto& cfg = WingConfig::me().wingCfg;
	auto pos = cfg.m_levelUpMap.find(sourceTplId);
	if(pos == cfg.m_levelUpMap.end())
		return false;

	//验证元宝
	if(useYuanbao)
	{
		if(!role->checkMoney(MoneyType::money_4, pos->second.needYuanbao))
			return false;
	}

	//验证金币
	const MoneyType needMoneyType = pos->second.needMoneyType;
	const uint32_t needMoneyNum = pos->second.needMoneyNum;
	if(!role->checkMoney(needMoneyType, needMoneyNum))
		return false;

	//验证材料
	const auto& needObjVec = pos->second.needObjVec;
	for(auto iter = needObjVec.begin(); iter != needObjVec.end(); ++iter)
	{
		const uint16_t objNum = role->m_packageSet.getObjNum(iter->first, PackageType::role);
		if(objNum >= iter->second)
			continue;

		role->sendSysChat("材料不足");
		return false;
	}

	//扣材料
	for(auto iter = needObjVec.begin(); iter != needObjVec.end(); ++iter)
	{
		if(!role->m_packageSet.eraseObj(iter->first, iter->second, PackageType::role, "翅膀晋阶"))
			return false;
	}

	//扣金币
	if(!role->reduceMoney(needMoneyType, needMoneyNum, "翅膀晋阶"))
		return false;

	//扣元宝
	if(useYuanbao)
	{
		if(!role->reduceMoney(MoneyType::money_4, pos->second.needYuanbao, "翅膀晋阶"))
			return false;
	}

	return true;
}

bool Wing::reduceMaterialOfZhuling(uint8_t type)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return false;

	const auto& cfg = WingConfig::me().wingCfg;
	auto pos = cfg.m_consumeMap.find(type);
	if(pos == cfg.m_consumeMap.end())
		return false;

	//验证货币
	const MoneyType needMoneyType = pos->second.needMoneyType;
	const uint32_t needMoneyNum = pos->second.needMoneyNum;
	if(!role->checkMoney(needMoneyType, needMoneyNum))
		return false;

	//验证材料
	const uint32_t needTplId = pos->second.needTplId;
	const uint16_t needTplNum = pos->second.needTplNum;
	if(0 != needTplId)
	{
		const uint16_t objNum = role->m_packageSet.getObjNum(needTplId, PackageType::role);
		if(needTplNum > objNum)
		{
			role->sendSysChat("材料不足");
			return false;
		}
	}

	//扣材料
	if(0 != needTplId)
	{
		if(!role->m_packageSet.eraseObj(needTplId, needTplNum, PackageType::role, "翅膀注灵"))
			return false;
	}

	//扣货币
	if(!role->reduceMoney(needMoneyType, needMoneyNum, "翅膀注灵"))
		return false;

	return true;
}

uint8_t Wing::getLingliLevel() const
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return 0;

	uint64_t lingli = 0;
	if(m_sceneItem == SceneItemType::role)
		lingli = role->getMoney(MoneyType::money_8);
	else if(m_sceneItem == SceneItemType::hero)
		lingli = role->getMoney(MoneyType::money_9);

	uint8_t level = 0;
	const auto& cfg = WingConfig::me().wingCfg;
	for(auto pos = cfg.m_rewardMap.begin(); pos != cfg.m_rewardMap.end(); ++pos)
	{
		if(lingli >= pos->second.needLingli)
			level = pos->first;
	}
	
	return level;
}

void Wing::sendWingLevelUpResult(OperateRetCode code)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	PublicRaw::RetWingLevelUpResult send;
	send.code = code;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetWingLevelUpResult), &send, sizeof(send));
	return;
}

void Wing::sendWingZhulingResult(OperateRetCode code)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	PublicRaw::RetWingZhulingResult send;
	send.code = code;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetWingZhulingResult), &send, sizeof(send));
	return;
}

}
