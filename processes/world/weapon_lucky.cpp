#include "weapon_lucky.h"
#include "role_manager.h"
#include "lucky_config.h"

#include "water/common/commdef.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/package.h"
#include "protocol/rawmsg/public/package.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

WeaponLucky WeaponLucky::m_me;

WeaponLucky& WeaponLucky::me()
{
	return m_me;
}


void WeaponLucky::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestLevelUpWeaponLucky, std::bind(&WeaponLucky::clientmsg_RequestWeaponLucky, this, _1, _2, _3));
	
}

//请求提升武器幸运
void WeaponLucky::clientmsg_RequestWeaponLucky(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestLevelUpWeaponLucky*>(msgData);
	if(!rev)
		return;

	PackageType packageType = rev->packageType;
	EquipPackage::Ptr packagePtr = getEquipPackagePtr(roleId, packageType);
	if(packagePtr == nullptr)
		return;

	const uint16_t cell = 0;	//武器在第0格
	ObjChildType childType = packagePtr->getObjChildTypeByCell(cell);
	if(childType == ObjChildType::none || childType != ObjChildType::weapon)
		return;

	const uint8_t curLevel = packagePtr->getWeaponLuckyLevel();
	if((uint8_t)-1 == curLevel)
		return;

	const auto& cfg = LuckyConfig::me().luckyCfg;
	if(curLevel >= cfg.m_luckyMap.size())
	{
		role->sendSysChat("已提升到最高星级");
		return;
	}
	
	const uint8_t nextLevel = curLevel + 1;
	if(!reduceMaterial(roleId, nextLevel, rev->bProtect, rev->autoBuy))
		return;

	auto pos = cfg.m_luckyMap.find(nextLevel);
	if(pos == cfg.m_luckyMap.end())
		return;

	componet::Random<uint32_t> sucess_prob(1, STRONG_EQUIP_BASE_PROP_NUM);
	if(pos->second.prob >= sucess_prob.get()) //武器幸运提升成功
	{
		if(!packagePtr->setWeaponLuckyLevel(nextLevel))
			return;

		sendWeaponLuckyResult(roleId, packageType, curLevel, nextLevel);
		role->sendSysChat("提升成功");
	}
	else //武器幸运提升失败
	{
		if(0 == pos->second.reduceLevel)
		{
			sendWeaponLuckyResult(roleId, packageType, curLevel, curLevel);
			role->sendSysChat("提升失败, 幸运星级保持不变");
			return;
		}
		
		//若使用保护符，则不降级
		if(rev->bProtect)
		{
			sendWeaponLuckyResult(roleId, packageType, curLevel, curLevel);
			role->sendSysChat("提升失败, 幸运星级被保护, 幸运星级保持不变");
			return;
		}

		uint8_t newLevel = SAFE_SUB(curLevel, pos->second.reduceLevel);	
		if(!packagePtr->setWeaponLuckyLevel(newLevel))
			return;
		
		sendWeaponLuckyResult(roleId, packageType, curLevel, newLevel);
		role->sendSysChat("提升失败, 幸运星级 -{}", curLevel - newLevel);
	}

	return;
}

bool WeaponLucky::reduceMaterial(RoleId roleId, uint8_t nextLevel, bool bProtect, bool autoBuy)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return false;

	const auto& cfg = LuckyConfig::me().luckyCfg;
	auto pos = cfg.m_luckyMap.find(nextLevel);
	if(pos == cfg.m_luckyMap.end())
		return false;

	//验证货币
	MoneyType needMoneyType = static_cast<MoneyType>(pos->second.needMoneyType);
	const uint32_t needMoneyNum = pos->second.needMoneyNum;
	if(!role->checkMoney(needMoneyType, needMoneyNum))
	{
		role->sendSysChat("{}不足", role->getMoneyName(needMoneyType));
		return false;
	}

	uint32_t needObjTplId = pos->second.needObjTplId;
	uint16_t needObjNum = pos->second.needObjNum;
	uint32_t needProTplId = pos->second.needProTplId;
	uint16_t needProNum = pos->second.needProNum;
	
	//验证幸运油
	const uint16_t objNum = role->m_packageSet.getObjNum(needObjTplId, PackageType::role);
	if(needObjNum > objNum)
	{
		if(autoBuy && !role->checkMoneyByObj(needObjTplId, needObjNum))
		{
			return false;
		}
		else if(!autoBuy)
		{
			role->sendSysChat("祝福油不足");
			return false;
		}
	}

	//验证保护符
	if(bProtect)
	{
		if(0 == needProTplId || 0 == needProNum)
		{
			role->sendSysChat("此星级强化不允许使用保护符");
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

	//扣幸运油
	if(!role->m_packageSet.eraseObj(needObjTplId, needObjNum, PackageType::role, "武器幸运"))
	{
		if(autoBuy && !role->autoReduceObjMoney(needObjTplId, needObjNum, "自动武器幸运"))
			return false;
	}

	//扣保护符
	if(bProtect && 0 != needProTplId && 0 != needProNum)
	{
		if(!role->m_packageSet.eraseObj(needProTplId, needProNum, PackageType::role, "武器幸运"))
		{
			if(autoBuy && !role->autoReduceObjMoney(needProTplId, needProNum, "自动武器幸运"))
				return false;
		}
	}
	
	//扣钱
	if(!role->reduceMoney(needMoneyType, needMoneyNum, "武器幸运"))
		return false; 

	return true;
}

EquipPackage::Ptr WeaponLucky::getEquipPackagePtr(RoleId roleId, PackageType packageType)
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

bool WeaponLucky::isHeroEquipPackage(PackageType packageType)
{
	if(packageType == PackageType::equipOfWarrior 
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
		return true;

	return false;
}

void WeaponLucky::sendWeaponLuckyResult(RoleId roleId, PackageType packageType, uint8_t oldLevel, uint8_t newLevel)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	PublicRaw::RetLevelUpWeaponLuckyResult send;
	send.oldLevel = oldLevel;
	send.newLevel = newLevel;
	send.packageType = packageType;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetLevelUpWeaponLuckyResult), &send, sizeof(send));
	return;
}


}
