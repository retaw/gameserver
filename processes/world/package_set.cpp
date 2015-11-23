#include "role.h"
#include "world.h"  
#include "package_set.h"
#include "role_manager.h"
#include "object_config.h"
#include "nonsuch_config.h"
#include "scene.h"
#include "scene_object.h"
#include "scene_object_manager.h"
#include "role_equip_package.h"
#include "hero_equip_package.h"
#include "stone_package.h"
#include "package_config.h"

#include "water/componet/logger.h"
#include "water/componet/random.h"
#include "water/componet/scope_guard.h"
#include "water/process/process_id.h"

#include "protocol/rawmsg/private/package.h"
#include "protocol/rawmsg/private/package.codedef.private.h"

#include "protocol/rawmsg/public/package.h"
#include "protocol/rawmsg/public/package.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"  

namespace world{

PackageSet::PackageSet(SceneItemType sceneItem, Job job, RoleId roleId, const uint8_t unlockCellNumOfRole, const uint8_t unlockCellNumOfHero, const uint8_t unlockCellNumOfStorage)
: m_sceneItem(sceneItem)
, m_job(job)
, m_roleId(roleId)
, m_unlockCellNumOfRole(unlockCellNumOfRole)
, m_unlockCellNumOfHero(unlockCellNumOfHero)
, m_unlockCellNumOfStorage(unlockCellNumOfStorage)
, m_sortObjTimePoint(EPOCH) 
{
	init(sceneItem);
}

void PackageSet::init(SceneItemType sceneItem)
{
	if(sceneItem == SceneItemType::role)
	{
		m_packageNum = MAX_PACKAGE_NUM_OF_ROLE;
	}
	else if(sceneItem == SceneItemType::hero)
	{
		m_packageNum = MAX_PACKAGE_NUM_OF_HERO;
	}

	return;
}

//加载背包数据
void PackageSet::loadFromDB(const std::vector<RoleObjData::ObjData> objVec)
{
	std::map<PackageType, std::vector<RoleObjData::ObjData> > packageMap;
	for(auto iter = objVec.begin(); iter != objVec.end(); ++iter)
	{
		if(sceneItemType() == SceneItemType::role && iter->packageType >= PackageType::equipOfWarrior)
			continue;

		RoleObjData::ObjData temp;
		temp.objId = iter->objId;
		temp.packageType = iter->packageType;
		temp.cell = iter->cell;
		temp.tplId = iter->tplId;
		temp.item = iter->item;
		temp.skillId = iter->skillId;
		temp.bind = iter->bind;
        temp.sellTime = iter->sellTime;
		temp.strongLevel = iter->strongLevel;
		temp.luckyLevel = iter->luckyLevel;

		auto pos = packageMap.find(temp.packageType);
		if(pos == packageMap.end())
		{
			std::vector<RoleObjData::ObjData> cellVec;
			cellVec.push_back(temp);

			packageMap.insert(std::make_pair(temp.packageType, cellVec));
		}
		else
		{
			pos->second.push_back(temp);
		}

		LOG_TRACE("背包, 加载数据, roleId={}, packageType={}, cell={}, tplId={}, item={}, skillId={}, bind={}, strongLevel={}, luckyLevel={}, objVecSize={}",
				  m_roleId, (uint8_t)temp.packageType,
				  temp.cell, temp.tplId, temp.item, temp.skillId, 
				  temp.bind, temp.strongLevel, temp.luckyLevel, objVec.size());
	}

	if(packageMap.size() < m_packageNum)
	{
		std::vector<RoleObjData::ObjData> cellVec;
		if(sceneItemType() == SceneItemType::role)
		{
			for(uint8_t i = 1; i <= m_packageNum; i ++)
			{
				auto iter = packageMap.find(static_cast<PackageType>(i));
				if(iter != packageMap.end())
					continue;

				packageMap.insert(std::make_pair(static_cast<PackageType>(i), cellVec));
				LOG_TRACE("背包, 加载数据, 角色, 此类型背包为空, roleId={}, packageType={}, packageMap={}, packageNum={} ",
						  m_roleId, i, packageMap.size(), m_packageNum);
			}
		}
		else if(sceneItemType() == SceneItemType::hero)
		{
			PackageType packageType = getHeroEquipPackageType();
			if(packageType == PackageType::none)
				return;

			for(uint8_t i = static_cast<uint8_t>(packageType); i < static_cast<uint8_t>(packageType) + m_packageNum; i++)
			{
				auto pos = packageMap.find(static_cast<PackageType>(i));
				if(pos != packageMap.end())
					continue;

				packageMap.insert(std::make_pair(static_cast<PackageType>(i), cellVec));
				LOG_TRACE("背包, 加载数据, 英雄, 此类型背包为空, roleId={}, job={}, packageType={}, packageMap={}, packageNum={} ",
						  m_roleId, m_job, i, packageMap.size(), m_packageNum);
			}
		}
	}

	if(packageMap.size() != m_packageNum)
	{
		EXCEPTION(CreatePackageSetFailed, "背包, load 失败, roleId={}, packageMap={}, packageNum={}",
				  m_roleId, packageMap.size(), m_packageNum);
		return;
	}

	//创建背包
	createPackage(packageMap);
	
	return;
}

void PackageSet::createPackage(const std::map<PackageType, std::vector<RoleObjData::ObjData> >& packageMap)
{
	for(auto pos = packageMap.begin(); pos != packageMap.end(); ++pos)
	{
		const PackageType packageType = pos->first;
		const uint16_t totalCellNum = getTotalCellNum(packageType);
		const uint16_t unlockCellNum = getUnlockCellNum(packageType);

		if(0 == unlockCellNum || 0 == totalCellNum)
		{
			EXCEPTION(CreatePackageSetFailed, "背包, 格子错误, make_shared<Package> 失败, roleId={}, packageType={}, unlocekCellNum={}, totalCellNum={}",
					  m_roleId, packageType, unlockCellNum, totalCellNum);
			return;
		}

		if(pos->second.size() > unlockCellNum)
		{
			EXCEPTION(CreatePackageSetFailed, "背包, 格子错误, make_shared<Package> 失败, roleId={}, packageType={}, unlocekCellNum={}, cellDataSize={}",
					  m_roleId, packageType, unlockCellNum, pos->second.size());
			return;
		}

		std::vector<CellInfo> cellInfoVec;
		cellInfoVec.resize(unlockCellNum);
		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			Object::Ptr obj = createObj(iter->tplId, iter->objId, iter->skillId, iter->strongLevel, iter->luckyLevel);
			if(obj == nullptr)
			{
				EXCEPTION(CreatePackageSetFailed, "背包, 获取 Object::Ptr失败, make_shared<Package> 失败, roleId={}, packageType={}, tplId={}, objId={}, skillId={}, strongLevel={}, luckyLevel={}",
						  m_roleId, packageType, iter->tplId,
						  iter->objId, iter->skillId, iter->strongLevel, iter->luckyLevel);
				return;
			}

			CellInfo temp;
            temp.modifyType = ModifyType::none;
			temp.objPtr = obj;
			temp.item = iter->item;
			temp.objId = iter->objId;
			temp.cell = iter->cell;
			temp.bind = iter->bind;
			temp.modifyType = ModifyType::none;
            temp.sellTime = iter->sellTime;
			cellInfoVec[temp.cell] = temp;
		}

		std::shared_ptr<Package> package = nullptr;
		if(isRoleEquipPackage(packageType)) 
		{
			package = std::make_shared<RoleEquipPackage>(sceneItemType(), packageType, totalCellNum, unlockCellNum, cellInfoVec);
		}
		else if(isHeroEquipPackage(packageType))
		{
			package = std::make_shared<HeroEquipPackage>(sceneItemType(), packageType, totalCellNum, unlockCellNum, cellInfoVec);
		}
		else if(isStonePackage(packageType))
		{
			package = std::make_shared<StonePackage>(sceneItemType(), packageType, totalCellNum, unlockCellNum, cellInfoVec);
		}
		else
		{
			package = std::make_shared<Package>(sceneItemType(), packageType, totalCellNum, unlockCellNum, cellInfoVec);
		}
		
		if(package == nullptr)
		{
			EXCEPTION(CreatePackageSetFailed, "背包, make_shared<Package> 失败, roleId={}, packageType={}, unlockCellNum={}, unEmptyCellNum={}",
					  m_roleId, packageType, unlockCellNum, pos->second.size());
			return;
		}

		m_packageMap.insert(std::make_pair(packageType, package));
		LOG_TRACE("背包, make_shred<Package> 成功, roleId={}, packageType={}, unlockCellNum={}, unEmptyCellNum={}",
				  m_roleId, packageType, unlockCellNum, pos->second.size());
	}

	return;
}

void PackageSet::setOwner(std::shared_ptr<PK> owner)
{
	if(owner == nullptr)
		return;

	m_owner = owner;

	for(auto pos = m_packageMap.begin(); pos !=  m_packageMap.end(); ++pos)
	{
		if(pos->second == nullptr)
		{
			LOG_ERROR("背包, 获取Package::Ptr失败, owner=({}, {}), packageType={}, m_packageMap={}",
					  owner->name(), owner->id(), pos->first, m_packageMap.size());
		}

		pos->second->setOwner(owner);
	}

	//返回正在解锁的格子剩余解锁时间
	sendUnlockCellNeedSec();
	return;
}

std::shared_ptr<PK> PackageSet::getOwner() const
{
	PK::Ptr owner = m_owner.lock();
	if(owner == nullptr)
		return nullptr;

	return owner;
}

SceneItemType PackageSet::sceneItemType() const
{
	return m_sceneItem;
}

Object::Ptr PackageSet::createObj(TplId tplId, const ObjectId objId, const uint32_t skillId, const uint8_t strongLevel, const uint8_t luckyLevel) const
{
	if(0 == tplId)
		return nullptr;

	// 获取其基本属性，并作为obj完整属性的一部分（完整 = 基本部分 + 变化部分）
	const auto& cfg = ObjectConfig::me().objectCfg;
	auto pos = cfg.m_objBasicDataMap.find(tplId);
	if(pos == cfg.m_objBasicDataMap.end())
		return nullptr;

	ObjectId curObjId = 0;
	if(0 == objId)
	{
		Role::Ptr role = getRole();
		if(role == nullptr)
			return nullptr;

		role->autoAddCurObjId();
		curObjId = role->getCurObjId();
	}
	
	ObjBasicData data;
	data = pos->second;
	data.objId = 0 == objId ? curObjId : objId;
	data.skillId = (uint32_t)-1 == skillId ? getSkillIdByNonsuchId(pos->second.nonsuchId) : skillId;
	data.strongLevel = strongLevel;	
	data.luckyLevel = luckyLevel;

	Object::Ptr obj = std::make_shared<Object>(data);
	return obj;
}

uint32_t PackageSet::getSkillIdByNonsuchId(uint32_t nonsuchId) const
{
	const auto& cfg = NonsuchConfig::me().nonsuchCfg;     
	auto iter = cfg.m_skillTypeMap.find(nonsuchId);
	if(iter == cfg.m_skillTypeMap.end())
		return 0;

	if(iter->second.empty())
		return 0;

	componet::Random<uint32_t> rand_skill_type(0, iter->second.size() - 1);
	uint32_t skillTypeId = iter->second[rand_skill_type.get()];
	if(0 == skillTypeId)
		return 0;

	auto pos = cfg.m_skillMap.find(skillTypeId);
	if(pos == cfg.m_skillMap.end())
		return 0;

	if(pos->second.empty())
		return 0;

	componet::Random<uint32_t> rand_skill(0, pos->second.size() - 1);
	return pos->second[rand_skill.get()];
}

PackageType PackageSet::getHeroEquipPackageType() const
{
	if(sceneItemType() != SceneItemType::hero)
		return PackageType::none;

	if(m_job == Job::warrior)
		return PackageType::equipOfWarrior;
	else if(m_job == Job::magician)
		return PackageType::equipOfMagician;
	else if(m_job == Job::taoist)
		return PackageType::equipOfTaoist;

	return PackageType::none;
}

void PackageSet::updateUnlockCellNumTODB()
{
	RoleId roleId = getRoleId();
	if(0 == roleId)
		return;

	PrivateRaw::UpdatePackageUnlockCellNum send;
	send.roleId = roleId;
	send.unlockCellNumOfRole = getUnlockCellNum(PackageType::role);
	send.unlockCellNumOfHero = getUnlockCellNum(PackageType::hero);
	send.unlockCellNumOfStorage = getUnlockCellNum(PackageType::storage);

	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdatePackageUnlockCellNum), &send, sizeof(send));
	LOG_TRACE("背包, send UpdatePackageUnlockCellNum to {}, {}, roleId={}, unlockCellNumOfRole={}, unlockCellNumOfHero={}, unlockCellNumOfStorage={})",
			  dbcachedId, ret ? "ok" : "falied",
			  send.roleId, send.unlockCellNumOfRole,
			  send.unlockCellNumOfHero, send.unlockCellNumOfStorage);
	
	return;
}

std::shared_ptr<Role> PackageSet::getRole() const
{
	if(sceneItemType() == SceneItemType::role)
	{
		Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
		if(role == nullptr)
			return nullptr;

		return role;
	}
	else if(sceneItemType() == SceneItemType::hero)
	{
		Hero::Ptr hero = std::static_pointer_cast<Hero>(getOwner());
		if(hero == nullptr)
			return nullptr;

		Role::Ptr role = hero->getOwner();
		if(role == nullptr)
			return nullptr;

		return role;
	}

	return nullptr;
}

RoleId PackageSet::getRoleId() const
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return 0;

	return role->id();
}

Package::Ptr PackageSet::getPackageByPackageType(PackageType packageType) const
{
	auto pos = m_packageMap.find(packageType);
	if(pos == m_packageMap.end())
	{
		LOG_ERROR("背包, 未找到packageType, 获取Package::Ptr失败, sceneItem={}, job={}, roleId={}, packageType={}, packageMap={}",
				  m_sceneItem, m_job, m_roleId, packageType, m_packageMap.size());
		return nullptr;
	}

	return pos->second;
}

uint16_t PackageSet::getEmptyCellNum(PackageType packageType) const
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return 0;
    return packagePtr->getEmptyCellNum();
}

uint16_t PackageSet::getUnlockCellNum(PackageType packageType) const
{
	uint16_t unlockCellNum = 0;

	switch(packageType)
	{
	case PackageType::role:
		unlockCellNum = m_unlockCellNumOfRole;
		break;
	case PackageType::hero:
		unlockCellNum = m_unlockCellNumOfHero;
		break;
	case PackageType::storage:
		unlockCellNum = m_unlockCellNumOfStorage;
		break;
	case PackageType::equipOfRole:
	case PackageType::equipOfWarrior:
	case PackageType::equipOfMagician:
	case PackageType::equipOfTaoist:
		unlockCellNum = MAX_CELL_NUM_OF_EQUIP;
		break;
    case PackageType::repurchase:
        unlockCellNum = MAX_CELL_NUM_OF_REPURCHASE;
        break;
	case PackageType::stoneOfRole:
	case PackageType::stoneOfWarrior:
	case PackageType::stoneOfMagician:
	case PackageType::stoneOfTaoist:
		unlockCellNum = MAX_CELL_NUM_OF_STONE;
		break;
	default:
		break;
	}

	return unlockCellNum;
}

uint16_t PackageSet::getTotalCellNum(PackageType packageType) const
{
	uint16_t totalCellNum = 0;

	switch(packageType)
	{
	case PackageType::role:
		totalCellNum = MAX_CELL_NUM_OF_ROLE;
		break;
	case PackageType::hero:
		totalCellNum = MAX_CELL_NUM_OF_HERO;
		break;
	case PackageType::storage:
		totalCellNum = MAX_CELL_NUM_OF_STORAGE;
		break;
	case PackageType::equipOfRole:
	case PackageType::equipOfWarrior:
	case PackageType::equipOfMagician:
	case PackageType::equipOfTaoist:
		totalCellNum = MAX_CELL_NUM_OF_EQUIP;
		break;
    case PackageType::repurchase:
        totalCellNum = MAX_CELL_NUM_OF_REPURCHASE;
        break;
	case PackageType::stoneOfRole:
	case PackageType::stoneOfWarrior:
	case PackageType::stoneOfMagician:
	case PackageType::stoneOfTaoist:
		totalCellNum = MAX_CELL_NUM_OF_STONE;
		break;
	default:
		break;
	}

	return totalCellNum;
}

void PackageSet::setUnlockCellNum(uint16_t num, PackageType packageType)
{
	if(packageType != PackageType::role
	   && packageType != PackageType::hero 
	   && packageType != PackageType::storage)
		return;

	if(packageType == PackageType::role)
	{
		m_unlockCellNumOfRole = num;
	}
	else if(packageType == PackageType::hero)
	{
		m_unlockCellNumOfHero = num;
	}
	else if(packageType == PackageType::storage)
	{
		m_unlockCellNumOfStorage = num;
	}

	updateUnlockCellNumTODB();
	sendUnlockCellNum(packageType);
	return;
}

bool PackageSet::checkPutObj(TplId tplId, uint16_t num, Bind bind, PackageType packageType)	
{
	if(0 == tplId || 0 == num)
		return false;

    if(packageType == PackageType::repurchase)
        return true;

	Object::Ptr obj = createObj(tplId, (uint32_t)-1);
	if(obj == nullptr)
		return false;

	return checkPutObj(obj, num, bind, packageType);
}

bool PackageSet::checkPutObjByCell(TplId tplId, uint16_t cell, uint16_t num, Bind bind, PackageType packageType)
{
	if(0 == tplId || 0 == num)
		return false;

    if(packageType == PackageType::repurchase)
        return true;

	Object::Ptr obj = createObj(tplId, (uint32_t)-1);
	if(obj == nullptr)
		return false;

	return checkPutObjByCell(obj, cell, num, bind, packageType);
}

bool PackageSet::checkPutObj(Object::Ptr obj, uint16_t num, Bind bind, PackageType packageType)	
{
	if(nullptr == obj || 0 == num)
		return false;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return false;

	return packagePtr->checkPutObj(obj, num, bind);
}

bool PackageSet::checkPutObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, PackageType packageType)
{
	if(nullptr == obj || 0 == num)
		return false;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return false;

	return packagePtr->checkPutObjByCell(obj, cell, num, bind);
}



bool PackageSet::exchangeCell(uint16_t fromCell, uint16_t toCell, PackageType packageType)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return false;

	return packagePtr->exchangeCell(fromCell, toCell);
}


uint16_t PackageSet::putObj(TplId tplId, uint16_t num, Bind bind, PackageType packageType, uint32_t skillId/*=(uint32_t)-1*/, uint8_t strongLevel/*=0*/, uint8_t luckyLevel/*=0*/, bool newObj/*=true*/)
{
	if(0 == tplId || 0 == num)
		return 0;

	if(!checkPutObj(tplId, num, bind, packageType))
		return 0;

	//putObj() 一次仅放入一个格子，放入N个物品，需要几个格子，每个格子放几个,
	//则由PackageSet上的PutObj()接口计算
	
	//从config中获取此tplId的物品最大可叠加数量，判断是否拆分num
	//先写入，随后从config中获取此数据
	const uint16_t maxStackNum = ObjectConfig::me().getMaxStackNum(tplId);
	if(0 == maxStackNum)
		return 0;

	const uint16_t needPutObjNum = num;		//需要放入背包的物品总数
	uint16_t totalPutObjNum = 0;			//已经放入背包的物品数量
	uint16_t needCellNum = SAFE_DIV(num, maxStackNum) + 1;	//需要的空格子数

	for(uint16_t i = 0; i < needCellNum; i++)
	{
		if(totalPutObjNum >= needPutObjNum)
			break;

		uint16_t putNum = 0;
		if(SAFE_SUB(needPutObjNum, totalPutObjNum) >= maxStackNum)
		{
			putNum = maxStackNum;
		}
		else
		{
			putNum = SAFE_SUB(needPutObjNum, totalPutObjNum);
		}

		if(0 == putNum)
			break;

		Object::Ptr obj = createObj(tplId, 0, skillId, strongLevel, luckyLevel);
		if(obj == nullptr)
			return totalPutObjNum;

		totalPutObjNum += putObj(obj, putNum, bind, packageType, newObj);
	}

	return totalPutObjNum;
}

uint16_t PackageSet::putObjByCell(TplId tplId, uint16_t cell, uint16_t num, Bind bind, PackageType packageType, uint32_t skillId/*=(uint32_t)-1*/, uint8_t strongLevel/*=0*/, uint8_t luckyLevel/*=0*/, bool newObj/*=true*/)
{
	if(0 == tplId || 0 == num)
		return 0;

	if(!checkPutObjByCell(tplId, cell, num, bind, packageType))
		return 0;
	
	const uint16_t maxStackNum = ObjectConfig::me().getMaxStackNum(tplId);
	if(0 == maxStackNum || num > maxStackNum)
		return 0;
	
	Object::Ptr obj = createObj(tplId, 0, skillId, strongLevel, luckyLevel);
	if(obj == nullptr)
		return 0;
	
	return putObjByCell(obj, cell, num, bind, packageType, newObj);
}

uint16_t PackageSet::putObj(Object::Ptr obj, uint16_t num, Bind bind, PackageType packageType, bool newObj/*=true*/)
{
	if(obj == nullptr || 0 == num)
		return 0;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->putObj(obj, num, bind, newObj);
}

uint16_t PackageSet::putObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, PackageType packageType, bool newObj/*=true*/)
{
	if(obj == nullptr || 0 == num)
		return 0;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->putObjByCell(obj, cell, num, bind, newObj);
}


uint16_t PackageSet::tryEraseObj(TplId tplId, uint16_t num, PackageType packageType, const std::string& text)
{
	if(0 == tplId || 0 == num)
		return 0;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return 0;
	
	return packagePtr->tryEraseObj(tplId, num, text);
}

uint16_t PackageSet::tryEraseObjByCell(uint16_t cell, PackageType packageType, const std::string& text)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->tryEraseObjByCell(cell, text);
}

uint16_t PackageSet::tryEraseObjByCell(uint16_t cell, uint16_t num, PackageType packageType, const std::string& text)
{
	if(0 == num)
		return 0;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->tryEraseObjByCell(cell, num, text);
}

bool PackageSet::eraseObj(TplId tplId, uint16_t num, PackageType packageType, const std::string& text)
{
	if(0 == tplId)
		return false;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return false;

	return packagePtr->eraseObj(tplId, num, text);
}

bool PackageSet::eraseObj(TplId tplId, uint16_t num, Bind bind, PackageType packageType, const std::string& text)
{
    if(0 == tplId)
        return false;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return false;

	return packagePtr->eraseObj(tplId, num, bind, text);
}

Object::Ptr PackageSet::eraseObjByCell(uint16_t cell, PackageType packageType, const std::string& text, bool notify/*=true*/)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return nullptr;

 	return packagePtr->eraseObjByCell(cell, text, false, notify);
}

Object::Ptr PackageSet::eraseObjByCell(uint16_t cell, uint16_t num, PackageType packageType, const std::string& text, bool notify/*=true*/)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return nullptr;

 	return packagePtr->eraseObjByCell(cell, num, text, false, notify);
}

Object::Ptr PackageSet::eraseFixedCellObj(uint16_t cell, PackageType packageType, const std::string& text)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return nullptr;

 	return packagePtr->eraseObjByCell(cell, text, true);
}

uint16_t PackageSet::getObjNum(TplId tplId, PackageType packageType) const
{
	if(0 == tplId)
		return 0;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->getObjNum(tplId);
}

uint16_t PackageSet::getObjNumByCell(uint16_t cell, PackageType packageType) const
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->getObjNumByCell(cell);
}

TplId PackageSet::getTplIdByCell(uint16_t cell, PackageType packageType) const
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return 0;
	
	return packagePtr->getTplIdByCell(cell);
}

Object::Ptr PackageSet::getObjByCell(uint16_t cell, PackageType packageType) const
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return nullptr;

	return packagePtr->getObjByCell(cell);
}

Bind PackageSet::getBindByCell(uint16_t cell, PackageType packageType) const
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return Bind::none;

	return packagePtr->getBindByCell(cell);
}

void PackageSet::fixCell(uint16_t cell, PackageType packageType)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return;

	return packagePtr->fixCell(cell);
}

void PackageSet::cancelFixCell(uint16_t cell, PackageType packageType)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return;

	return packagePtr->cancelFixCell(cell);
}

bool PackageSet::isCellFixed(uint16_t cell, PackageType packageType) const
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return true;

	return packagePtr->isCellFixed(cell);
}

bool PackageSet::isCanDiscard(uint16_t cell, PackageType packageType) const
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return false;

	const Object::Ptr obj = packagePtr->getObjByCell(cell);
	if(obj == nullptr)
		return false;

    if(packagePtr->isCellFixed(cell))
        return false;

	return obj->bDiscard();
}

TplId PackageSet::getTplIdByObjChildType(ObjChildType childType, PackageType packageType) const
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->getTplIdByObjChildType(childType);
}

bool PackageSet::isRoleEquipPackage(PackageType packageType) const
{
	if(packageType == PackageType::equipOfRole) 
	{
		return true;
	}

	return false;
}

bool PackageSet::isHeroEquipPackage(PackageType packageType) const
{
	if(packageType == PackageType::equipOfWarrior
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
	{
		return true;
	}

	return false;
}

bool PackageSet::isStonePackage(PackageType packageType) const
{
	if(packageType == PackageType::stoneOfRole
	   || packageType == PackageType::stoneOfWarrior
	   || packageType == PackageType::stoneOfMagician
	   || packageType == PackageType::stoneOfTaoist)
	{
		return true;
	}

	return false;
}


/******************************** 获取装备包属性 begin ****************************/ 
//获取生命
uint32_t PackageSet::getHp(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getHp();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getHp();
	}

	return 0;
}

//获取魔法
uint32_t PackageSet::getMp(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMp();
	}
	else if(isStonePackage(packageType))
	{ 
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;
	
		return packagePtr->getMp();
	}

	return 0;
}

//获取生命等级
uint32_t PackageSet::getHpLv(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getHpLv();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getHpLv();
	}

	return 0;
}

//获取魔法等级
uint32_t PackageSet::getMpLv(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMpLv();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMpLv();
	}

	return 0;
}

//获取物攻Min
uint32_t PackageSet::getPAtkMin(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPAtkMin();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPAtkMin();
	}

	return 0;
}

//获取物攻Max
uint32_t PackageSet::getPAtkMax(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPAtkMax();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPAtkMax();
	}

	return 0;
}

//获取魔攻Min
uint32_t PackageSet::getMAtkMin(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMAtkMin();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMAtkMin();	
	}

	return 0;
}

//获取魔攻Max
uint32_t PackageSet::getMAtkMax(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMAtkMax();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMAtkMax();
	}

	return 0;
}

//获取道术Min
uint32_t PackageSet::getWitchMin(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getWitchMin();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getWitchMin();
	}

	return 0;
}

//获取道术Max
uint32_t PackageSet::getWitchMax(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getWitchMax();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getWitchMax();
	}

	return 0;
}

//获取物防Min
uint32_t PackageSet::getPDefMin(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{	
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPDefMin();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPDefMin();
	}

	return 0;
}

//获取物防Max
uint32_t PackageSet::getPDefMax(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPDefMax();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPDefMax();
	}

	return 0;
}

//获取魔防Min
uint32_t PackageSet::getMDefMin(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMDefMin();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		packagePtr->getMDefMin();
	}

	return 0;
}

//获取魔防Max
uint32_t PackageSet::getMDefMax(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMDefMax();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMDefMax();	
	}

	return 0;
}

//获取幸运
uint32_t PackageSet::getLucky(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{	
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getLucky();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getLucky();
	}

	return 0;
}

//获取诅咒
uint32_t PackageSet::getEvil(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getEvil();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getEvil();
	}

	return 0;
}

//获取命中
uint32_t PackageSet::getShot(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getShot();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getShot();
	}

	return 0;
}

//获取命中率
uint32_t PackageSet::getShotRatio(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getShotRatio();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getShotRatio();
	}

	return 0;
}

//获取物闪
uint32_t PackageSet::getPEscape(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPEscape();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getPEscape();
	}

	return 0;
}

//获取魔闪
uint32_t PackageSet::getMEscape(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMEscape();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getMEscape();
	}

	return 0;
}

//获取闪避率
uint32_t PackageSet::getEscapeRatio(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getEscapeRatio();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getEscapeRatio();
	}

	return 0;
}

//获取暴击
uint32_t PackageSet::getCrit(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getCrit();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getCrit();
	}

	return 0;
}

//获取暴击率
uint32_t PackageSet::getCritRatio(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getCritRatio();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getCritRatio();
	}

	return 0;
}

//获取防爆(暴击抗性, 坚韧)
uint32_t PackageSet::getAntiCrit(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{	
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getAntiCrit();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getAntiCrit();
	}

	return 0;
}

//获取暴伤
uint32_t PackageSet::getCritDamage(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getCritDamage();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getCritDamage();
	}

	return 0;
}

//获取增伤
uint32_t PackageSet::getDamageAdd(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getDamageAdd();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getDamageAdd();
	}

	return 0;
}

//获取增伤等级
uint32_t PackageSet::getDamageAddLv(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getDamageAddLv();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		packagePtr->getDamageAddLv();
	}

	return 0;
}

//获取减伤
uint32_t PackageSet::getDamageReduce(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getDamageReduce();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getDamageReduce();
	}

	return 0;
}

//获取减伤等级
uint32_t PackageSet::getDamageReduceLv(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getDamageReduceLv();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getDamageReduceLv();
	}

	return 0;
}

uint32_t PackageSet::getAntiDropEquip(PackageType packageType) const
{
	if(isRoleEquipPackage(packageType) || isHeroEquipPackage(packageType))
	{
		auto packagePtr =  std::static_pointer_cast<EquipPackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getAntiDropEquip();
	}
	else if(isStonePackage(packageType))
	{
		auto packagePtr = std::static_pointer_cast<StonePackage>(getPackageByPackageType(packageType));
		if(packagePtr == nullptr)
			return 0;

		return packagePtr->getAntiDropEquip();
	}

	return 0;
}
/******************************** 获取装备包属性 end ****************************/  




/********************************* 处理背包操作消息 begin *****************************/
//返回背包物品列表
void PackageSet::sendObjList(PackageType packageType)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return;

	LOG_TRACE("背包, 物品列表, 主动请求, roleId={}, packageType={}", getRoleId(), packageType);
	packagePtr->sendObjListToMe();
}

//返回背包已解锁的格子数
void PackageSet::sendUnlockCellNum(PackageType packageType)
{
	PK::Ptr owner = getOwner();
	if(owner == nullptr)
	{
		LOG_ERROR("背包, PK::WPtr失效, s->c, 发送背包已解锁格子数消息失败");
		return;
	}
	
	PublicRaw::RetPackageUnlockCellNum send;
	send.unlockCellNUm = getUnlockCellNum(packageType);
	send.packageType = packageType;
	owner->sendToMe(RAWMSG_CODE_PUBLIC(RetPackageUnlockCellNum), &send, sizeof(send));
}

//请求摧毁一定数量的某类型物品, 且指定格子
void PackageSet::requestDestoryObjByCell(uint16_t cell, uint16_t num, PackageType packageType)
{
	if(0 == num || packageType != PackageType::role)
		return;

	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());        
	if(role == nullptr)
		return;

	uint16_t objNum = getObjNumByCell(cell, packageType);
	if(num > objNum)
	{	
		LOG_ERROR("物品不足, 摧毁失败, name={}, roleId={}, cell={}, needEraseNum={}, objNum={}, packageType={}", 
				   role->name(), role->id(), cell, num, objNum, packageType);
		return;
	}

	Object::Ptr obj = getObjByCell(cell, packageType);
	if(obj == nullptr)
		return;

	std::string name = obj->name();
	if(nullptr == eraseObjByCell(cell, num, packageType, "摧毁"))
	{
		LOG_ERROR("物品不足, 摧毁失败, name={}, roleId={}, cell={}, needEraseNum={}, packageType={}", 
				   role->name(), role->id(), cell, num, packageType);
		return;
	}
	
	//role->sendSysChat(ChannelType::screen_right_down, "摧毁 {}*{}", name, num);	
	return;
}

//请求丢弃一定数量的某类型物品, 且指定格子
void PackageSet::requestDiscardObjByCell(uint16_t cell, uint16_t num, PackageType packageType)
{
	if(0 == num || packageType != PackageType::role)
		return;

	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());        
	if(role == nullptr)
		return;

	Scene::Ptr s = role->scene();
	if(s == nullptr)
		return;

	if(!isCanDiscard(cell, packageType))
		return;

	{//是绑定的，则摧毁
		Package::Ptr packagePtr = getPackageByPackageType(packageType);
		if(packagePtr == nullptr)
			return;

		Bind bind = packagePtr->getBindByCell(cell);
		if(bind == Bind::none)
			return;

		if(bind == Bind::yes)
		{
			requestDestoryObjByCell(cell, num, packageType);
			return;
		}
	}

	uint16_t objNum = getObjNumByCell(cell, packageType);
	Object::Ptr obj = getObjByCell(cell, packageType);
	if(obj == nullptr)
		return;

	if(num > objNum)
	{	
		LOG_ERROR("物品不足, 丢弃失败, name={}, roleId={}, cell={}, needEraseNum={}, objNum={}, packageType={}", 
				   role->name(), role->id(), cell, num, objNum, packageType);
		return;
	}

	if(nullptr == eraseObjByCell(cell, num, packageType, "丢弃"))
	{
		LOG_ERROR("物品不足, 丢弃失败, name={}, roleId={}, cell={}, needEraseNum={}, packageType={}", 
				   role->name(), role->id(), cell, num, packageType);
		return;
	}

	std::vector<RoleId> ownerVec;
	SceneObject::Ptr sceneObj = SceneObjectManager::me().createObj(obj->tplId(), num, Bind::no, ownerVec, EPOCH, obj->skillId(), obj->strongLevel(), obj->luckyLevel());
	if(sceneObj == nullptr)
		return;

	if(!s->addObj(sceneObj, role->pos(), 8))
		return;

	sceneObj->afterEnterScene();
	//role->sendSysChat(ChannelType::screen_right_down, "丢弃 {}*{}", sceneObj->name(), num);
	return;
}

//请求拆分物品数量
void PackageSet::requestSplitObjNum(uint16_t cell, uint16_t num, PackageType packageType)
{
	if(0 == num || packageType != PackageType::role)
		return;

	if(packageType == PackageType::equipOfRole)
		return;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return;

    if(packagePtr->isCellFixed(cell))
        return;


	Object::Ptr obj = getObjByCell(cell, packageType);
	uint16_t item = getObjNumByCell(cell, packageType);
	if(nullptr == obj || 0 == item)
		return;

	if(num >= item)
		return;

	uint16_t toCell = packagePtr->getFirstEmptyCellIndex();
	Bind bind = packagePtr->getBindByCell(cell);
	if(toCell == (uint16_t)-1 || bind == Bind::none)
		return;

	if(nullptr == eraseObjByCell(cell, num, packageType, "拆分", false))
		return;

	//此处不可直接使用obj指针存放物品, 否则丢失物品
	putObjByCell(obj->tplId(), toCell, num, bind, packageType, obj->skillId(), obj->strongLevel(), obj->luckyLevel(), false);

	return;
}

//请求交换格子, 支持同一背包及跨背包
void PackageSet::requestExchangeCell(uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage)
{
	if(fromPackage == toPackage)
	{
		//若可合并，则执行合并操作，否则执行交换操作
		if(mergerObj(fromCell, toCell, fromPackage))
			return;

		exchangeCell(fromCell, toCell, fromPackage);
	}
	else if(isStonePackage(toPackage)) //目标宝石背包, 特殊处理
	{
		moveObjToStonePackage(fromCell, toCell, fromPackage, toPackage);
	}
	else
	{
		moveObj(fromCell, toCell, fromPackage, toPackage);
	}

	return;
}

bool PackageSet::mergerObj(uint16_t fromCell, uint16_t toCell, PackageType packageType)
{
	if(packageType != PackageType::role && packageType != PackageType::storage)
		return false;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);    
	if(packagePtr == nullptr)
		return false;

    if(packagePtr->isCellFixed(fromCell) || packagePtr->isCellFixed(toCell))
        return false;

	const Object::Ptr sourceObj = packagePtr->getObjByCell(fromCell);
	const Object::Ptr destObj = packagePtr->getObjByCell(toCell);
	if(sourceObj == nullptr ||  destObj == nullptr)
		return false;

	if(destObj->tplId() != sourceObj->tplId())
		return false;

	Bind sourceBind = packagePtr->getBindByCell(fromCell);
	Bind destBind = packagePtr->getBindByCell(toCell);
	if(sourceBind == Bind::none || destBind == Bind::none)
		return false;

	if(sourceBind != destBind)
		return false;

	const uint16_t sourceItem = packagePtr->getObjNumByCell(fromCell);
	const uint16_t destItem = packagePtr->getObjNumByCell(toCell);
	uint16_t maxStackNum = destObj->maxStackNum();

	if(destItem >= maxStackNum)
		return false;

	uint16_t mergerNum = 0;
	if(destItem + sourceItem <= maxStackNum)
		mergerNum = sourceItem;
	else if(destItem + sourceItem > maxStackNum)
		mergerNum = SAFE_SUB(maxStackNum, destItem);

	if(0 == mergerNum)
		return false;

	if(nullptr == packagePtr->eraseObjByCell(fromCell, mergerNum, "合并", false, false))
		return false;

	const uint16_t putObjNum = packagePtr->putObjByCell(sourceObj, toCell, mergerNum, sourceBind, false);

	//合并失败，则回退
	if(mergerNum != putObjNum)
	{
		packagePtr->putObjByCell(sourceObj, fromCell, mergerNum, sourceBind, false);
		packagePtr->eraseObjByCell(toCell, putObjNum, "合并失败回退", false, false);
		return false;
	}
	
	return true;
}

//移动物品，跨背包
void PackageSet::moveObj(uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage)
{
	//不允许直接将仓库中的物品直接移动到英雄背包中
	if(fromPackage == PackageType::storage && toPackage == PackageType::hero)
		return;
	
	//不允许将非角色背包中的物品移动到角色装备中
	if(toPackage == PackageType::equipOfRole && fromPackage != PackageType::role)
		return;

	//不允许将非角色背包中的物品移动到英雄装备中
	if(isHeroEquipPackage(toPackage) && fromPackage != PackageType::role)
		return;

	//不允许将非角色背包中的物品移到宝石包中
	if(isStonePackage(toPackage) && fromPackage != PackageType::role)
		return;

	Package::Ptr fromPackagePtr = getPackageByPackageType(fromPackage);
	Package::Ptr toPackagePtr = getPackageByPackageType(toPackage);
	if(fromPackagePtr == nullptr || toPackagePtr == nullptr)
		return;

    if(fromPackagePtr->isCellFixed(fromCell) || toPackagePtr->isCellFixed(toCell))
        return;

	//必须放在eraseObjByCell之前
	const uint16_t sourceItem = fromPackagePtr->getObjNumByCell(fromCell);
	const uint16_t destItem = toPackagePtr->getObjNumByCell(toCell);
	const Bind sourceBind = fromPackagePtr->getBindByCell(fromCell);
	const Bind destBind = toPackagePtr->getBindByCell(toCell);

	const Object::Ptr sourceObj = fromPackagePtr->eraseObjByCell(fromCell, "移动", false, false);
	const Object::Ptr destObj = toPackagePtr->eraseObjByCell(toCell, "移动", false, false);
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

//移动物品到宝石背包, 特殊处理
void PackageSet::moveObjToStonePackage(uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage)
{
	if(!isStonePackage(toPackage) || fromPackage != PackageType::role)
		return;

	Package::Ptr fromPackagePtr = getPackageByPackageType(fromPackage);
	Package::Ptr toPackagePtr = getPackageByPackageType(toPackage);
	if(fromPackagePtr == nullptr || toPackagePtr == nullptr)
		return;

	//必须放在eraseObjByCell之前
	const uint16_t eraseSourceItem = 1;	//镶嵌宝石，一个目标格子尽可镶嵌一个，特殊处理
	const uint16_t destItem = toPackagePtr->getObjNumByCell(toCell);
	const Bind sourceBind = fromPackagePtr->getBindByCell(fromCell);
	const Bind destBind = toPackagePtr->getBindByCell(toCell);

	const Object::Ptr sourceObj = fromPackagePtr->eraseObjByCell(fromCell, eraseSourceItem,"移动", false, false);
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
	if(0 == putObjByCell(sourceObj->tplId(), toCell, eraseSourceItem, sourceBind, toPackage, sourceObj->skillId(), sourceObj->strongLevel(), sourceObj->luckyLevel(), false))
	{
		//若目标背包放入失败，则执行回退操作
		fromPackagePtr->putObjByCell(sourceObj, fromCell, eraseSourceItem, sourceBind, false);
		toPackagePtr->putObjByCell(destObj, toCell, destItem, destBind, false);
		return;
	}
	fromPackagePtr->putObjByCell(destObj, fromCell, destItem, destBind, false);
	return;
}

//请求存取物品，跨背包 
void PackageSet::requestMoveObj(uint16_t fromCell, PackageType fromPackage, PackageType toPackage)
{
    Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
    if(role == nullptr)
        return;
	
	//不允许直接将仓库中物品直接移动到英雄背包中
	if(fromPackage == PackageType::storage && toPackage == PackageType::hero)
		return;

	//不允许对装备包及宝石包进行不指定格子操作
	if(isRoleEquipPackage(toPackage) || isHeroEquipPackage(toPackage) || isStonePackage(toPackage))
	{
		return;
	}

	Package::Ptr fromPackagePtr = getPackageByPackageType(fromPackage);
	Package::Ptr toPackagePtr = getPackageByPackageType(toPackage);
	if(fromPackagePtr == nullptr || toPackagePtr == nullptr)
		return;

    if(fromPackagePtr->isCellFixed(fromCell))
        return;

	const Object::Ptr obj = fromPackagePtr->getObjByCell(fromCell);
	const uint16_t item = fromPackagePtr->getObjNumByCell(fromCell);
	Bind bind = fromPackagePtr->getBindByCell(fromCell);
	if(obj == nullptr|| item == 0|| bind == Bind::none)
		return;

	if(!toPackagePtr->checkPutObj(obj, item, bind))
	{
		role->sendSysChat("背包空间不足");
		return;
	}

	if(nullptr == fromPackagePtr->eraseObjByCell(fromCell, "存取", false, false))
		return;

	const uint16_t putNum = toPackagePtr->putObj(obj, item, bind, false);
	if(putNum < item)
	{
		//失败部分，做回退操作
		const uint16_t notPutNum = SAFE_SUB(item, putNum);
		if(notPutNum == fromPackagePtr->putObjByCell(obj, fromCell, notPutNum, bind, false))
		{
			LOG_TRACE("背包, 跨背包, 不指定格子存取物品, 部分失败, 已将失败部分放回原背包, name={}, roleId={}, objId={}, tplId={}, item={}, putNum={}, notPutCell={}, fromCell={}, package:{}->{}",
					  role->name(), role->id(), obj->objId(), obj->tplId(), 
					  item, putNum, notPutNum, fromCell, fromPackage, toPackage);
			return;
		}
		LOG_ERROR("背包, 跨背包, 不指定格子存取物品, 部分失败, name={}, roleId={}, objId={}, tplId={}, item={}, putNum={}, notPutCell={}, fromCell={}, package:{}->{}",
				  role->name(), role->id(), obj->objId(), obj->tplId(), 
				  item, putNum, notPutNum, fromCell, fromPackage, toPackage);
		return;
	}

	LOG_TRACE("背包, 跨背包, 不指定格子存取物品, 成功, name={}, roleId={}, tplId={}, item={}, putNum={}, fromCell={}, package:{}->{}",
			   role->name(), role->id(), obj->tplId(), item, putNum, fromCell, fromPackage, toPackage);
}

//请求整理物品
void PackageSet::requestSortObj(PackageType packageType)
{
	if(packageType != PackageType::role && packageType != PackageType::storage)
		return;

	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return;

	if(sceneItemType() != SceneItemType::role)
		return;

    Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
    if(role == nullptr)
        return;
   
	TimePoint now = componet::Clock::now();
	if(m_sortObjTimePoint != EPOCH)
	{
		if(now <= m_sortObjTimePoint + std::chrono::seconds {10})
		{
			role->sendSysChat("整理背包冷却中");
			return;
		}
	}

	m_sortObjTimePoint = now;
	packagePtr->sortObj();
}

void PackageSet::requestSellObj(uint16_t fromCell, uint16_t num, PackageType fromPackage)
{
	if(sceneItemType() != SceneItemType::role)
		return;

    Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
    if(role == nullptr)
        return;
    
	if(fromPackage != PackageType::role)
        return;
	
	Package::Ptr fromPackagePtr = getPackageByPackageType(fromPackage);
	Package::Ptr toPackagePtr = getPackageByPackageType(PackageType::repurchase);
	if(fromPackagePtr == nullptr || toPackagePtr == nullptr)
		return;

    if(fromPackagePtr->isCellFixed(fromCell))
        return;

	const Object::Ptr obj = fromPackagePtr->getObjByCell(fromCell);
	Bind bind = fromPackagePtr->getBindByCell(fromCell);
	if(obj == nullptr || bind == Bind::none)
		return;

    MoneyType moneyType = obj->moneyType();
    const uint64_t price = obj->price();
	if(nullptr == fromPackagePtr->eraseObjByCell(fromCell, num, "出售"))
		return;

	const uint16_t putNum = putObj(obj->tplId(), num, bind, PackageType::repurchase, false);
	if(putNum < num)
	{
		fromPackagePtr->putObjByCell(obj, fromCell, SAFE_SUB(num, putNum), bind, false);
		LOG_ERROR("出售道具, 失败, role({}),({}), objname={}, tplId={}, num={}, putNum={}, fromCell={}, package:{}",
                  role->name(), role->id(), obj->name(), obj->tplId(), num, putNum, fromCell, fromPackage);
		return;
	}

    role->addMoney(moneyType, num*price, "出售道具, tplId={}, num={}", obj->tplId(), num);
	LOG_TRACE("出售道具, 成功, role({}),({}), objname={}, tplId={}, num={}, putNum={}, moneyType={}, money={}, fromCell={}, package:{}",
              role->name(), role->id(), obj->name(), obj->tplId(), num, putNum, moneyType, num*price, fromCell, fromPackage);
	return;
}


void PackageSet::requestRepurchaseObj(uint16_t fromCell)
{
	if(sceneItemType() != SceneItemType::role)
		return;

    Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
    if(role == nullptr)
        return;
	
	Package::Ptr fromPackagePtr = getPackageByPackageType(PackageType::repurchase);
	Package::Ptr toPackagePtr = getPackageByPackageType(PackageType::role);
	if(fromPackagePtr == nullptr || toPackagePtr == nullptr)
		return;

    const Object::Ptr obj = fromPackagePtr->getObjByCell(fromCell);
    Bind bind = fromPackagePtr->getBindByCell(fromCell);
    const uint16_t objnum = fromPackagePtr->getObjNumByCell(fromCell);
    if(obj == nullptr || bind == Bind::none || 0 == objnum)
        return;

    if(!checkPutObj(obj->tplId(), objnum, bind, PackageType::role))
    {
        role->sendSysChat("背包空间不足");
        return;
    }
    MoneyType moneyType = obj->moneyType();
    const uint64_t totalMoney = role->getMoney(moneyType);
    const uint64_t needMoney = (uint64_t)objnum * obj->price();
    if(totalMoney < needMoney)
    {
        role->sendSysChat("{}不足", role->getMoneyName(moneyType));
        return;
    }

	if(nullptr == fromPackagePtr->eraseObjByCell(fromCell, objnum, "回购", false, false))
        return;
	const uint16_t putNum = putObj(obj->tplId(), objnum, bind, PackageType::role, obj->skillId(), obj->strongLevel(), obj->luckyLevel());
	if(putNum < objnum)
	{
		fromPackagePtr->putObjByCell(obj, fromCell, SAFE_SUB(objnum, putNum), bind);
		LOG_ERROR("回购道具, 失败, role({}),({}), objname={}, tplId={}, num={}, putNum={}, fromCell={}",
                  role->name(), role->id(), obj->name(), obj->tplId(), objnum, putNum, fromCell);
		return;
	}

    role->reduceMoney(moneyType, needMoney, "回购道具");
	LOG_TRACE("回购道具, 成功, role({}),({}), objname={}, tplId={}, num={}, putNum={}, moneyType={}, money={}, fromCell={}",
              role->name(), role->id(), obj->name(), obj->tplId(), objnum, putNum, moneyType, needMoney, fromCell);
	return;
}

//s->c 返回正在解锁的格子剩余解锁时间(主背包)
void PackageSet::sendUnlockCellNeedSec()
{
	if(sceneItemType() != SceneItemType::role)
		return;

	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
	if(role == nullptr)
		return;

	uint16_t unlockCellNum = getUnlockCellNum(PackageType::role);
	if(0 == unlockCellNum || unlockCellNum == MAX_CELL_NUM_OF_ROLE)
		return;

	uint16_t canUnlockNum = 0;	//玩家在线时间可开启的格子数
	const uint32_t totalOnlineSec = role->getTotalOnlineSec();
	
	const auto& cfg = PackageConfig::me().packageCfg;
	for(auto pos = cfg.m_roleMap.begin(); pos != cfg.m_roleMap.end(); ++pos)
	{
		if(unlockCellNum > pos->second.cell)
			continue;

		if(pos->second.needOnlineSec > totalOnlineSec)
			continue;

		canUnlockNum++;
	}

	//正在倒计时的格子
	uint16_t cell = unlockCellNum + canUnlockNum;
	if(cell >= MAX_CELL_NUM_OF_ROLE)
		cell = MAX_CELL_NUM_OF_HERO - 1;

	auto pos = cfg.m_roleMap.find(cell);
	if(pos == cfg.m_roleMap.end())
		return;

	PublicRaw::RetUnlockCellNeedSec send;
	send.cell = cell;
	send.needSec = SAFE_SUB(pos->second.needOnlineSec, totalOnlineSec);
	send.unlockCellNUm = getUnlockCellNum(PackageType::role);

	role->sendToMe(RAWMSG_CODE_PUBLIC(RetUnlockCellNeedSec), &send, sizeof(send));
	return;
}

//获取解锁格子需要的元宝数
void PackageSet::requestUnlockCellNeedMoney(uint16_t num, PackageType packageType)
{
	if(sceneItemType() != SceneItemType::role || packageType != PackageType::role)
		return;

	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
	if(role == nullptr)
		return;

	uint32_t addExp = 0;
	uint32_t needMoney = getUnlockCellNeedMoney(num, addExp);
	if((uint32_t)-1 == needMoney)
		return;

	PublicRaw::RetUnlockCellNeedMoney send;
	send.num = num;
	send.needMoney = needMoney;
	
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetUnlockCellNeedMoney), &send, sizeof(send));
	return;
}

//请求解锁格子
void PackageSet::requestUnlockCell(uint16_t num, PackageType packageType)
{
	if(sceneItemType() != SceneItemType::role)
		return;

	if(packageType == PackageType::role)
	{
		unlockCellOfRole(num);
	}
	else if(packageType == PackageType::storage)
	{
		unlockCellOfStorage(num);
	}

	return;
}

//解锁主背包格子
void PackageSet::unlockCellOfRole(uint16_t num)
{
	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
	if(role == nullptr)
		return;
	
	Package::Ptr packagePtr = getPackageByPackageType(PackageType::role);
	if(packagePtr == nullptr)
		return;

	uint32_t addExp = 0;
	uint32_t needMoney = getUnlockCellNeedMoney(num, addExp);
	if((uint32_t)-1 == needMoney || 0 == addExp)
		return;

	if(0 != needMoney)
	{
		if(!role->reduceMoney(MoneyType::money_4, needMoney, "解锁背包格子"))
			return;
	}

	if(!packagePtr->unlockCell(num))
	{
		//解锁格子失败，则返还扣除的元宝
		role->addMoney(MoneyType::money_4, needMoney, "解锁拜拜格子");
		return;
	}
	
	role->addExp(addExp);
	setUnlockCellNum(packagePtr->getUnlockCellNum(), PackageType::role);
	sendUnlockCellNeedSec();
	role->sendSysChat("解锁背包格子成功");
	return;
}

//解锁仓库格子
void PackageSet::unlockCellOfStorage(uint16_t num)
{
	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
	if(role == nullptr)
		return;
	
	Package::Ptr packagePtr = getPackageByPackageType(PackageType::storage);
	if(packagePtr == nullptr)
		return;

	uint16_t unlockCellNum = getUnlockCellNum(PackageType::storage);
	if(0 == unlockCellNum || unlockCellNum == MAX_CELL_NUM_OF_STORAGE)
		return;

	const uint16_t fromLockCell = unlockCellNum;
	const uint16_t toLockCell = fromLockCell + num - 1;
	if(toLockCell >= MAX_CELL_NUM_OF_STORAGE)
		return;

	const auto& cfg = PackageConfig::me().packageCfg;
	uint32_t needTplId = cfg.needTplId;
	uint16_t needNum = 0;
	uint32_t addExp = 0;

	for(uint16_t i = fromLockCell; i <= toLockCell; i++)
	{
		auto pos = cfg.m_storageMap.find(i);
		if(pos == cfg.m_storageMap.end())
			return;

		needNum += pos->second.needNum;
		addExp += pos->second.addExp;
	}

	if(0 == needNum || 0 == addExp)
		return;

	uint32_t objNum = getObjNum(needTplId, PackageType::role);
	if(needNum > objNum)
	{
		role->sendSysChat("仓库扩充券不足");
		return;
	}

	uint16_t eraseNum = tryEraseObj(needTplId, needNum, PackageType::role, "解锁仓库");
	if(eraseNum != needNum)
	{
		//消耗物品失败，则返还已扣除的物品
		putObj(needTplId, eraseNum, Bind::yes, PackageType::role, false);
		return;
	}

	if(!packagePtr->unlockCell(num))
	{
		//解锁格子失败，则返还扣除的物品
		putObj(needTplId, needNum, Bind::yes, PackageType::role, false);
		return;
	}

	role->addExp(addExp);
	setUnlockCellNum(packagePtr->getUnlockCellNum(), PackageType::storage);
	role->sendSysChat("解锁仓库格子成功");
	return;
}

//获取解锁格子需要的元宝数
uint32_t PackageSet::getUnlockCellNeedMoney(uint16_t num, uint32_t& addExp)
{
	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
	if(role == nullptr)
		return (uint32_t)-1;
	
	uint16_t unlockCellNum = getUnlockCellNum(PackageType::role);
	if(0 == unlockCellNum || unlockCellNum == MAX_CELL_NUM_OF_ROLE)
		return (uint32_t)-1;

	const uint16_t fromLockCell = unlockCellNum;
	const uint16_t toLockCell = fromLockCell + num - 1;
	if(toLockCell >= MAX_CELL_NUM_OF_ROLE)
		return (uint32_t)-1;

	const auto& cfg = PackageConfig::me().packageCfg;
	uint32_t sec = cfg.sec;
	uint32_t yuanbao = cfg.yuanbao;
	if(0 == sec || 0 == yuanbao)
		return (uint32_t)-1;

	uint32_t needOnlineSec =0;
	for(uint16_t i = fromLockCell; i <= toLockCell; i++)
	{
		auto pos = cfg.m_roleMap.find(i);
		if(pos == cfg.m_roleMap.end())
			return (uint32_t)-1;

		needOnlineSec = pos->second.needOnlineSec;
		addExp += pos->second.addExp;
	}

	if(0 == needOnlineSec || 0 == addExp)
		return (uint32_t)-1;

	//需要折算成元宝的秒数
	uint32_t needMoney = 0;
	uint32_t tempSec = SAFE_SUB(needOnlineSec, role->getTotalOnlineSec());
	if(0 != tempSec)
	{
		needMoney = SAFE_DIV(tempSec, sec) * yuanbao;
		if(0 != SAFE_MOD(tempSec, sec))
		{
			needMoney += yuanbao;
		}
	}

	return needMoney;
}

/********************************* 处理背包操作消息 end *****************************/



/********************************* 处理场景上的物品 begin *****************************/ 
//请求拾取物品
void PackageSet::requestPickupObject(ObjectId objIdOfScene)
{
	if(sceneItemType() != SceneItemType::role)
		return;

	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
	if(role == nullptr)
		return;

	SceneObject::Ptr sceneObj = SceneObjectManager::me().getById(objIdOfScene);	
	if(sceneObj == nullptr || sceneObj->needErase())
		return;

	if(sceneObj->sceneId() != role->sceneId())
		return;

	Coord2D sceneObjPos = sceneObj->pos();
	Coord2D rolePos = role->pos();
	if(std::abs(sceneObjPos.x - rolePos.x) > 2 || std::abs(sceneObjPos.y - rolePos.y) > 2)
		return;

	if(!sceneObj->canPickUp(role->id()))
	{
		role->sendSysChat("该物品暂不属于你, 稍后再拾取");
		return;
	}

	uint16_t item = sceneObj->item();
	if(sceneObj->parentType() == ObjParentType::money)
	{
		uint16_t type = SAFE_SUB((uint16_t)sceneObj->childType(), (uint16_t)sceneObj->parentType());
		if(!role->addMoney(static_cast<MoneyType>(type), item, "拾取货币"))
			return;
	}
	else
	{
		uint32_t tplId = sceneObj->tplId();
		Bind bind = sceneObj->bind();
		uint32_t skillId = sceneObj->skillId();
		uint8_t strongLevel = sceneObj->strongLevel();
		uint8_t luckyLevel = sceneObj->strongLevel();

		if(!role->checkPutObj(tplId, item, bind))
		{
			role->sendSysChat("背包空间不足");
			return;
		}

		if(0 == role->putObj(tplId, item, bind, PackageType::role, skillId, strongLevel, luckyLevel))
			return;
	}
	
	SceneObjectManager::me().eraseObj(sceneObj->objId());
	return;
}
/********************************* 处理场景上的物品 end *******************************/



void PackageSet::execPackageCell(PackageType packageType, std::function<bool (CellInfo& cellInfo)> exec)
{
	Package::Ptr packagePtr = getPackageByPackageType(packageType);
	if(packagePtr == nullptr)
		return;

    packagePtr->execCell(exec);
}

}

