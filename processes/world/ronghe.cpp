#include "ronghe.h"
#include "ronghe_config.h"
#include "role_manager.h"

#include "water/common/commdef.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/package.h"
#include "protocol/rawmsg/public/package.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

Ronghe Ronghe::m_me;

Ronghe& Ronghe::me()
{
	return m_me;
}


void Ronghe::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestRongheEquip, std::bind(&Ronghe::clientmsg_RequestRongheEquip, this, _1, _2, _3));
	
}

//请求融合装备
void Ronghe::clientmsg_RequestRongheEquip(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestRongheEquip*>(msgData);
	if(!rev)
		return;

	if(rev->sourcePackageType != PackageType::role)
		return;

	Package::Ptr sourcePackagePtr = role->m_packageSet.getPackageByPackageType(rev->sourcePackageType);
	EquipPackage::Ptr destPackagePtr = getEquipPackagePtr(roleId, rev->destPackageType);
	if(nullptr == sourcePackagePtr || nullptr == destPackagePtr)
		return;

	Object::Ptr sourceObj = sourcePackagePtr->getObjByCell(rev->sourceCell);
	Object::Ptr destObj = destPackagePtr->getObjByCell(rev->destCell);
	if(nullptr == sourceObj || nullptr  == destObj)
		return;

	if(destObj->childType() == ObjChildType::none || destObj->childType() == ObjChildType::wing)
		return;

	if(destObj->childType() != sourceObj->childType())
		return;

	const uint8_t sourceStrongLevel = sourceObj->strongLevel();
	const uint8_t destStrongLevel = destObj->strongLevel();
	const uint8_t strongLevel = MAX(sourceStrongLevel, destStrongLevel);
	if(0 == strongLevel)
	{
		role->sendSysChat("装备没有强化等级, 不能融合");
		return;
	}

	if(!reduceMaterial(roleId, strongLevel, rev->autoBuy))
		return;

	const auto& cfg = RongheConfig::me().rongheCfg;
	auto pos = cfg.m_rongheMap.find(strongLevel);
	if(pos == cfg.m_rongheMap.end())
		return;

	componet::Random<uint32_t> sucess_prob(1, 1000);
	if(pos->second.prob >= sucess_prob.get()) //融合成功
	{
		if(!sourcePackagePtr->eraseObjByCell(rev->sourceCell, "装备融合"))
			return;

		destPackagePtr->setStrongLevel(rev->destCell, strongLevel);
		sendRongheResult(roleId, OperateRetCode::sucessful);
		role->sendSysChat("融合成功");
	}
	else
	{
		sendRongheResult(roleId, OperateRetCode::failed);
		role->sendSysChat("融合失败");
	}
	
	return;
}

bool Ronghe::reduceMaterial(RoleId roleId, uint8_t strongLevel, bool autoBuy)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return false;

	const auto& cfg = RongheConfig::me().rongheCfg;
	auto pos = cfg.m_rongheMap.find(strongLevel);
	if(pos == cfg.m_rongheMap.end())
		return false;

	//验证货币
	MoneyType needMoneyType = pos->second.needMoneyType;
	const uint32_t needMoneyNum = pos->second.needMoneyNum;
	if(!role->checkMoney(needMoneyType, needMoneyNum))
	{
		role->sendSysChat("{}不足", role->getMoneyName(needMoneyType));
		return false;
	}

	uint32_t needTplId = pos->second.needTplId;
	uint16_t needTplNum = pos->second.needTplNum;

	//验证融合材料
	const uint16_t objNum = role->m_packageSet.getObjNum(needTplId, PackageType::role);
	if(needTplNum > objNum)
	{
		if(autoBuy && !role->checkMoneyByObj(needTplId, needTplNum))
		{
			return false;
		}
		else if(!autoBuy)
		{
			role->sendSysChat("材料不足");
			sendRongheResult(roleId, OperateRetCode::materialNotEnough);
			return false;
		}
	}

	//扣融合材料
	if(!role->m_packageSet.eraseObj(needTplId, needTplNum, PackageType::role, "融合"))
	{
		if(autoBuy && !role->autoReduceObjMoney(needTplId, needTplNum, "自动融合"))
			return false;
	}

	//扣钱
	if(!role->reduceMoney(needMoneyType, needMoneyNum, "融合"))
		return false; 

	return true;
}

EquipPackage::Ptr Ronghe::getEquipPackagePtr(RoleId roleId, PackageType packageType)
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

bool Ronghe::isHeroEquipPackage(PackageType packageType)
{
	if(packageType == PackageType::equipOfWarrior 
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
		return true;

	return false;
}

void Ronghe::sendRongheResult(RoleId roleId, OperateRetCode code)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	PublicRaw::RetEquipRongheResult send;
	send.code = code;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetEquipRongheResult), &send, sizeof(send));
}


}
