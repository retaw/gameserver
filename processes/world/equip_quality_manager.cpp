#include "equip_quality_manager.h"
#include "equip_quality_config.h"
#include "role_manager.h"
#include "object_config.h"

#include "water/common/commdef.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/equip_equality.h"
#include "protocol/rawmsg/public/equip_equality.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

EquipQualityManager EquipQualityManager::m_me;

EquipQualityManager& EquipQualityManager::me()
{
	return m_me;
}


void EquipQualityManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestImproveEquipQuality, std::bind(&EquipQualityManager::clientmsg_RequestImproveEquipQuality, this, _1, _2, _3));
	
}

//请求装备升品
void EquipQualityManager::clientmsg_RequestImproveEquipQuality(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestImproveEquipQuality*>(msgData);
	if(!rev)
		return;

	Package::Ptr packagePtr = getPackagePtr(roleId, rev->packageType);
	if(nullptr == packagePtr)
		return;

	Object::Ptr sourceObj = packagePtr->getObjByCell(rev->cell);
	if(nullptr == sourceObj)
		return;

	if(sourceObj->parentType() != ObjParentType::equip)
		return;

	const uint32_t sourceTplId = sourceObj->tplId();
	const auto& cfg = EquipQualityConfig::me().qualityCfg;
	auto pos = cfg.m_equipMap.find(sourceTplId);
	if(pos == cfg.m_equipMap.end())
		return;

	if(!reduceMaterial(roleId, sourceTplId, rev->autoBuy))
		return;

	const uint32_t destTplId = pos->second.destTplId;
	const Bind bind = packagePtr->getBindByCell(rev->cell);
	const uint32_t skillId = 0;
	const uint8_t strongLevel = sourceObj->strongLevel();
	const uint8_t luckyLevel = sourceObj->luckyLevel();

	componet::Random<uint32_t> sucess_prob(1, 1000);
	if(pos->second.prob >= sucess_prob.get()) //装备升品成功
	{
		if(!packagePtr->eraseObjByCell(rev->cell, "装备升品"))
			return;

		Object::Ptr destObj = role->m_packageSet.createObj(destTplId, 0, skillId, strongLevel, luckyLevel);
		if(destObj == nullptr)
			return;

		if(1 != packagePtr->putObjByCell(destObj, rev->cell, 1, bind))
		{
			LOG_ERROR("升品, 升品成功, 放入升品后的装备失败, sourceTplId={}, destTplId={}, bind={}, strongLevel={}, luckyLevel={}, packageType={}",
					  sourceTplId, destTplId, bind, strongLevel, luckyLevel, rev->packageType);
			return;
		}

		if(2 == rev->type)	//神装合成功能
		{
			role->sendSysChat(ChannelType::global, "恭喜玩家{}成功合成{}, 战力大增",
							  role->name(), ObjectConfig::me().getName(destTplId));
		}

		sendImproveResult(roleId, OperateRetCode::sucessful);
	}
	else
	{
		sendImproveResult(roleId, OperateRetCode::failed);
	}
	
	return;
}

bool EquipQualityManager::reduceMaterial(RoleId roleId, uint32_t sourceTplId, bool autoBuy)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return false;

	const auto& cfg = EquipQualityConfig::me().qualityCfg;
	auto pos = cfg.m_equipMap.find(sourceTplId);
	if(pos == cfg.m_equipMap.end())
		return false;

	//验证货币
	MoneyType needMoneyType = pos->second.needMoneyType;
	const uint32_t needMoneyNum = pos->second.needMoneyNum;
	if(!role->checkMoney(needMoneyType, needMoneyNum))
	{
		role->sendSysChat("{}不足", role->getMoneyName(needMoneyType));
		return false;
	}

	//验证升品材料
	std::vector<std::pair<uint32_t, uint16_t> > lackObjVec;	//材料不足的物品
	const auto& needObjVec = pos->second.needObjVec;
	for(auto iter = needObjVec.begin(); iter != needObjVec.end(); ++iter)
	{
		const uint16_t objNum = role->m_packageSet.getObjNum(iter->first, PackageType::role);
		if(objNum >= iter->second)
			continue;

		lackObjVec.push_back(std::make_pair(iter->first, SAFE_SUB(iter->second, objNum)));
	}
	
	if(autoBuy && !lackObjVec.empty())
	{
		if(!role->checkMoneyByObjs(lackObjVec))
			return false;
	}
	else if(!lackObjVec.empty())
	{
		role->sendSysChat("材料不足");
		sendImproveResult(roleId, OperateRetCode::materialNotEnough);
		return false;
	}

	//扣升品材料
	for(auto iter = needObjVec.begin(); iter != needObjVec.end(); ++iter)
	{
		const uint16_t objNum = role->m_packageSet.getObjNum(iter->first, PackageType::role);
		if(objNum >= iter->second)
		{
			if(!role->m_packageSet.eraseObj(iter->first, iter->second, PackageType::role, "装备升品"))
			   return false;
		}
		else if(autoBuy)
		{	
			//先扣除材料，再扣除不足的材料对应的货币
			if(0 != objNum && !role->m_packageSet.eraseObj(iter->first, objNum, PackageType::role, "装备升品"))	
				return false;
		
			const uint16_t needObjNum = SAFE_SUB(iter->second, objNum);
			if(!role->autoReduceObjMoney(iter->first, needObjNum, "自动升品"))
				return false;
		}
		else 
		{
			return false;
		}
	}

	//扣钱
	if(!role->reduceMoney(needMoneyType, needMoneyNum, "自动升品"))
		return false; 

	return true;
}

Package::Ptr EquipQualityManager::getPackagePtr(RoleId roleId, PackageType packageType)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return nullptr;

	if(isBelongRolePackage(packageType))
	{
		return role->m_packageSet.getPackageByPackageType(packageType);
	}
	else if(isHeroEquipPackage(packageType))
	{
		Job job = role->getHeroJobByPackageType(packageType);
		if(job == Job::none)
			return nullptr;

		Hero::Ptr hero = role->getHeroByJob(job);
		if(hero == nullptr)
			return nullptr;

		return hero->m_packageSet.getPackageByPackageType(packageType); 
	}

	return nullptr;
}

bool EquipQualityManager::isBelongRolePackage(PackageType packageType)
{
	if(packageType == PackageType::role || packageType == PackageType::equipOfRole)
		return true;

	return false;
}

bool EquipQualityManager::isHeroEquipPackage(PackageType packageType)
{
	if(packageType == PackageType::equipOfWarrior 
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
		return true;

	return false;
}

void EquipQualityManager::sendImproveResult(RoleId roleId, OperateRetCode code)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	PublicRaw::RetEquipImproveQualityResult send;
	send.code = code;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetEquipImproveQualityResult), &send, sizeof(send));
}


}
