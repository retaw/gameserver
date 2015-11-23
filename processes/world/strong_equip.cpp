#include "strong_equip.h"
#include "role_manager.h"
#include "strong_config.h"

#include "water/common/commdef.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/package.h"
#include "protocol/rawmsg/public/package.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

StrongEquip StrongEquip::m_me;

StrongEquip& StrongEquip::me()
{
	return m_me;
}


void StrongEquip::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestStrongEquip, std::bind(&StrongEquip::clientmsg_RequestStrongEquip, this, _1, _2, _3));
	
}

//请求强化装备
void StrongEquip::clientmsg_RequestStrongEquip(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestStrongEquip*>(msgData);
	if(!rev)
		return;

	PackageType packageType = rev->packageType;
	EquipPackage::Ptr packagePtr = getEquipPackagePtr(roleId, packageType);
	if(packagePtr == nullptr)
		return;

	const uint16_t cell = rev->cell;
	const uint8_t curLevel = packagePtr->getStrongLevelByCell(cell);
	if((uint8_t)-1 == curLevel)
		return;

	const auto& cfg = StrongConfig::me().strongCfg;
	if(curLevel >= cfg.m_strongMap.size())
	{
		role->sendSysChat("已强化到最高星级");
		return;
	}

	ObjChildType childType = packagePtr->getObjChildTypeByCell(cell);
	if(childType == ObjChildType::none || childType == ObjChildType::wing)
		return;

	const uint8_t nextLevel = curLevel + 1;
	if(!reduceMaterial(roleId, nextLevel, rev->bProtect, rev->autoBuy))
		return;

	auto pos = cfg.m_strongMap.find(nextLevel);
	if(pos == cfg.m_strongMap.end())
		return;

	const uint32_t maxStrongValue = pos->second.maxStrongValue;
	componet::Random<uint32_t> sucess_prob(1, STRONG_EQUIP_BASE_PROP_NUM);
	if(pos->second.prob >= sucess_prob.get()) //强化成功
	{
		if(!packagePtr->setStrongLevel(cell, nextLevel))
			return;

		if(role->getMoney(MoneyType::money_6) >= maxStrongValue)
		{
			role->reduceMoney(MoneyType::money_6, role->getMoney(MoneyType::money_6), "强化");
		}

		sendStrongEquipResult(roleId, cell, packageType, curLevel, nextLevel);
		role->sendSysChat("强化成功");
		return;
	}

	//强化失败，则奖励强化值
	uint32_t rewardMoney = pos->second.rewardMoney;
	const uint64_t strongValue = role->getMoney(MoneyType::money_6);
	if(strongValue + rewardMoney >= maxStrongValue)
	{
		rewardMoney = SAFE_SUB(maxStrongValue, strongValue);
	}
	
	if(role->getMoney(MoneyType::money_6) < maxStrongValue)
	{
		role->addMoney(MoneyType::money_6, rewardMoney, "强化");
	}

	if(0 == pos->second.reduceLevel)
	{
		sendStrongEquipResult(roleId, cell, packageType, curLevel, curLevel);
		role->sendSysChat("强化失败, 强化星级保持不变");
		return;
	}

	//若使用保护符或达到强化值上限，则不降级
	if(role->getMoney(MoneyType::money_6) >= maxStrongValue)
	{
		role->reduceMoney(MoneyType::money_6, role->getMoney(MoneyType::money_6), "强化");
		sendStrongEquipResult(roleId, cell, packageType, curLevel, curLevel);
		role->sendSysChat("强化失败, 装备星级被保护, 强化星级保持不变");
		return;
	}
	else if(rev->bProtect)
	{
		sendStrongEquipResult(roleId, cell, packageType, curLevel, curLevel);
		role->sendSysChat("强化失败, 装备星级被保护, 强化星级保持不变");
		return;
	}

	uint8_t newLevel = SAFE_SUB(curLevel, pos->second.reduceLevel);	
	if(!packagePtr->setStrongLevel(cell, newLevel))
		return;

	sendStrongEquipResult(roleId, cell, packageType, curLevel, newLevel);
	role->sendSysChat("强化失败, 装备星级 -{}", curLevel - newLevel);

	return;
}

bool StrongEquip::isHeroEquipPackage(PackageType packageType)
{
	if(packageType == PackageType::equipOfWarrior 
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
		return true;

	return false;
}

bool StrongEquip::reduceMaterial(RoleId roleId, uint8_t nextLevel, bool bProtect, bool autoBuy)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return false;

	const auto& cfg = StrongConfig::me().strongCfg;
	auto pos = cfg.m_strongMap.find(nextLevel);
	if(pos == cfg.m_strongMap.end())
		return false;

	//验证货币
	MoneyType needMoneyType = static_cast<MoneyType>(pos->second.needMoneyType);
	const uint32_t needMoneyNum = pos->second.needMoneyNum;
	if(!role->checkMoney(needMoneyType, needMoneyNum))
	{
		role->sendSysChat("{}不足", role->getMoneyName(needMoneyType));
		return false;
	}

	uint32_t needStoneTplId = pos->second.needStoneTplId;
	uint16_t needStoneNum = pos->second.needStoneNum;
	uint32_t needProTplId = pos->second.needProTplId;
	uint16_t needProNum = pos->second.needProNum;
	
	//验证强化石
	const uint16_t stoneNum = role->m_packageSet.getObjNum(needStoneTplId, PackageType::role);
	if(needStoneNum > stoneNum)
	{
		if(autoBuy && !role->checkMoneyByObj(needStoneTplId, needStoneNum))
		{
			return false;
		}
		else if(!autoBuy)
		{
			role->sendSysChat("强化石不足");
			return false;
		}
	}

	//验证保护符
	if(role->getMoney(MoneyType::money_6) < pos->second.maxStrongValue && bProtect)
	{
		if(0 == needProTplId || 0 == needProNum)
		{
			role->sendSysChat("此等级强化不允许使用保护符");
			return false;
		}

		const uint16_t protectObjNum = role->m_packageSet.getObjNum(needProTplId, PackageType::role);
		if(needProNum > protectObjNum)
		{
			if(autoBuy && !role->checkMoneyByObj(needProTplId, needProNum))
			{
				return false;
			}
			else if(!autoBuy)
			{
				role->sendSysChat("保护符不足");
				return false;
			}
		}
	}

	//扣强化石
	if(!role->m_packageSet.eraseObj(needStoneTplId, needStoneNum, PackageType::role, "强化"))
	{
		if(autoBuy && !role->autoReduceObjMoney(needStoneTplId, needStoneNum, "自动强化"))
			return false;
	}

	//扣保护符
	if(role->getMoney(MoneyType::money_6) < pos->second.maxStrongValue 
	   && bProtect && 0 != needProTplId && 0 != needProNum)
	{
		if(!role->m_packageSet.eraseObj(needProTplId, needProNum, PackageType::role, "强化"))
		{
			if(autoBuy && !role->autoReduceObjMoney(needProTplId, needProNum, "自动强化"))
				return false;
		}
	}
	
	//扣钱
	if(!role->reduceMoney(needMoneyType, needMoneyNum, "强化"))
		return false; 

	return true;
}

EquipPackage::Ptr StrongEquip::getEquipPackagePtr(RoleId roleId, PackageType packageType)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return nullptr;

	EquipPackage::Ptr packagePtr = nullptr;
	if(packageType == PackageType::equipOfRole)
	{
		packagePtr = std::static_pointer_cast<EquipPackage>(role->m_packageSet.getPackageByPackageType(packageType));
	}
	else if(isHeroEquipPackage(packageType))
	{
		Job job = role->getHeroJobByPackageType(packageType);
		if(job == Job::none)
			return nullptr;

		Hero::Ptr hero = role->getHeroByJob(job);
		if(hero == nullptr)
			return nullptr;

		packagePtr = std::static_pointer_cast<EquipPackage>(hero->m_packageSet.getPackageByPackageType(packageType)); 
	}

	return packagePtr;
}

void StrongEquip::sendStrongEquipResult(RoleId roleId, uint16_t cell, PackageType packageType, uint8_t oldLevel, uint8_t newLevel)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	PublicRaw::RetStrongEquipResult send;
	send.cell = cell;
	send.oldLevel = oldLevel;
	send.newLevel = newLevel;
	send.packageType = packageType;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetStrongEquipResult), &send, sizeof(send));
}


}
