#include "role.h"
#include "world.h"
#include "scene.h"
#include "equip_package.h"
#include "suit_config.h"
#include "role_equip_package.h"
#include "hero_equip_package.h"
#include "strong_config.h"
#include "lucky_config.h"
#include "wing_config.h"

#include "water/common/commdef.h"
#include "water/componet/logger.h"
#include "water/process/process_id.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include "protocol/rawmsg/private/package.h"
#include "protocol/rawmsg/private/package.codedef.private.h"

#include "protocol/rawmsg/public/package.h"
#include "protocol/rawmsg/public/package.codedef.public.h"

namespace world{

EquipPackage::EquipPackage(SceneItemType sceneItem, PackageType packageType, uint16_t totalCellNum, uint16_t unlockNum, const std::vector<CellInfo>& cellVec)
: Package(sceneItem, packageType, totalCellNum, unlockNum, cellVec)
, m_hp(0)
, m_mp(0)
, m_hpLv(0)
, m_mpLv(0)
, m_pAttackMin(0)
, m_pAttackMax(0)
, m_mAttackMin(0)
, m_mAttackMax(0)
, m_witchMin(0)
, m_witchMax(0)
, m_pDefenceMin(0)
, m_pDefenceMax(0)
, m_mDefenceMin(0)
, m_mDefenceMax(0)
, m_lucky(0)
, m_evil(0)
, m_shot(0)
, m_shotRatio(0)
, m_pEscape(0)
, m_mEscape(0)
, m_escapeRatio(0)
, m_crit(0)
, m_critRatio(0)
, m_antiCrit(0)
, m_critDamage(0)
, m_damageAdd(0)
, m_damageAddLv(0)
, m_damageReduce(0)
, m_damageReduceLv(0)
, m_antiDropEquip(0)
{
	initEquipPackageMap();
}

void EquipPackage::initEquipPackageMap()
{
	m_equipPackageMap.clear();
	m_equipPackageMap.insert(std::make_pair(0, ObjChildType::weapon));
	m_equipPackageMap.insert(std::make_pair(1, ObjChildType::helmet));
	m_equipPackageMap.insert(std::make_pair(2, ObjChildType::clothes));
	m_equipPackageMap.insert(std::make_pair(3, ObjChildType::necklace));
	m_equipPackageMap.insert(std::make_pair(4, ObjChildType::braceletLeft));
	m_equipPackageMap.insert(std::make_pair(5, ObjChildType::braceletRight));
	m_equipPackageMap.insert(std::make_pair(6, ObjChildType::ringLeft));
	m_equipPackageMap.insert(std::make_pair(7, ObjChildType::shoes));
	m_equipPackageMap.insert(std::make_pair(8, ObjChildType::belt));
	m_equipPackageMap.insert(std::make_pair(9, ObjChildType::wing));
	m_equipPackageMap.insert(std::make_pair(10, ObjChildType::ringRight));
	
	return;
}

void EquipPackage::setOwner(std::shared_ptr<PK> owner)
{
	if(owner == nullptr)
		return;

	Package::setOwner(owner);

	calculateAllAttribute();
	updateAllEquipSkills();
	return;
}

bool EquipPackage::checkPutObj(Object::Ptr obj, uint16_t num, Bind bind)
{
	if(nullptr == obj || 1 != num || bind == Bind::none)
		return false;

	uint16_t cell = getCanPutCellByObj(obj);
	if((uint16_t)-1 == cell)
		return false;

	return checkPutObjByCell(obj, cell, num, bind);
}

bool EquipPackage::checkPutObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind)
{
	if(sceneItemType() == SceneItemType::role)
	{
		RoleEquipPackage::Ptr roleEquipPackagePtr = std::static_pointer_cast<RoleEquipPackage>(shared_from_this());
		if(roleEquipPackagePtr == nullptr)
			return false;

		return roleEquipPackagePtr->checkPutObjByCell(obj, cell, num, bind);
	}
	else if(sceneItemType() == SceneItemType::hero)
	{
		HeroEquipPackage::Ptr heroEquipPackagePtr = std::static_pointer_cast<HeroEquipPackage>(shared_from_this());
		if(heroEquipPackagePtr == nullptr)
			return false;

		return heroEquipPackagePtr->checkPutObjByCell(obj, cell, num, bind);
	}
	
	return false;
}

//装备包不可交换格子
bool EquipPackage::exchangeCell(uint16_t fromCell, uint16_t toCell)
{
	return false;
}

uint16_t EquipPackage::putObj(Object::Ptr obj, uint16_t num, Bind bind, bool newObj/*=true*/)
{
	if(nullptr == obj || 1 != num || bind == Bind::none)
		return 0;

	uint16_t cell = getCanPutCellByObj(obj);
	if((uint16_t)-1 == cell)
		return 0;

	return putObjByCell(obj, cell, num, bind, newObj);
}

uint16_t EquipPackage::putObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, bool newObj/*=true*/)
{
	if(obj == nullptr || 1 != num || bind == Bind::none)
		return 0;

	//判断此装备是否可以放入此格子
	if(!checkPutObjByCell(obj, cell, num, bind))
		return 0;
	
	if(Package::putObjInCell(obj, cell, num, bind, newObj))
		return num;
	
	return 0;
}


ObjChildType EquipPackage::getObjChildTypeByCell(uint16_t cell) const
{
	if(cell >= m_equipPackageMap.size())
		return ObjChildType::none;

	auto pos = m_equipPackageMap.find(cell);
	if(pos == m_equipPackageMap.end())
		return ObjChildType::none;

	return pos->second;
}

uint16_t EquipPackage::getCanPutCellByObj(Object::Ptr obj) const
{
	if(obj == nullptr)
		return (uint16_t)-1;

	ObjChildType childType = obj->childType();
	for(auto pos = m_equipPackageMap.begin(); pos != m_equipPackageMap.end(); ++pos)
	{
		if(pos->second != childType)
			continue;

		return pos->first;
	}

	return (uint16_t)-1;
}

void EquipPackage::resetAllAttribute()
{
	m_hp = 0;
	m_mp = 0;
	m_hpLv = 0;
	m_mpLv = 0;
	m_pAttackMin = 0;
	m_pAttackMax = 0;
	m_mAttackMin = 0;
	m_mAttackMax = 0;
	m_witchMin = 0;
	m_witchMax = 0;
	m_pDefenceMin = 0;
	m_pDefenceMax = 0;
	m_mDefenceMin = 0;
	m_mDefenceMax = 0;

	m_lucky = 0;
	m_evil = 0;
	m_shot = 0;
	m_shotRatio = 0;
	m_pEscape = 0;
	m_mEscape = 0;
	m_escapeRatio = 0;
	m_crit = 0;
	m_critRatio = 0;
	m_antiCrit = 0;

	m_critDamage = 0;
	m_damageAdd = 0;
	m_damageAddLv = 0;
	m_damageReduce = 0;
	m_damageReduceLv = 0;

    m_antiDropEquip = 0;

	return;
}

void EquipPackage::calculateAllAttribute()
{
	resetAllAttribute();

	//计算基础属性及强化属性
	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;
	
		const uint8_t strongLevel = iter->objPtr->strongLevel();
		const ObjBasicData& data = iter->objPtr->getObjBasicData();
		m_hp += data.hp;
		m_hp += getStrongProp(PropertyType::maxhp, strongLevel, data);
		m_mp += data.mp;
		m_mp += getStrongProp(PropertyType::maxmp, strongLevel, data);
		m_pAttackMin += data.p_attackMin;
		m_pAttackMin += getStrongProp(PropertyType::p_attackMin, strongLevel, data);
		m_pAttackMax += data.p_attackMax;
		m_pAttackMax += getStrongProp(PropertyType::p_attackMax, strongLevel, data);
		m_mAttackMin += data.m_attackMin;
		m_mAttackMin += getStrongProp(PropertyType::m_attackMin, strongLevel, data);
		m_mAttackMax += data.m_attackMax;
		m_mAttackMax += getStrongProp(PropertyType::m_attackMax, strongLevel, data);
		m_witchMin += data.witchMin;
		m_witchMin += getStrongProp(PropertyType::witchMin, strongLevel, data);
		m_witchMax += data.witchMax;
		m_witchMax += getStrongProp(PropertyType::witchMax, strongLevel, data);
		m_pDefenceMin += data.p_defenceMin;
		m_pDefenceMin += getStrongProp(PropertyType::p_defenceMin, strongLevel, data);
		m_pDefenceMax += data.p_defenceMax;
		m_pDefenceMax += getStrongProp(PropertyType::p_defenceMax, strongLevel, data);
		m_mDefenceMin += data.m_defenceMin;
		m_mDefenceMin += getStrongProp(PropertyType::m_defenceMin, strongLevel, data);
		m_mDefenceMax += data.m_defenceMax;
		m_mDefenceMax += getStrongProp(PropertyType::m_defenceMax, strongLevel, data);

		m_shot += data.shot;
		m_shot += getStrongProp(PropertyType::shot, strongLevel, data);
		m_pEscape += data.p_escape;
		m_pEscape += getStrongProp(PropertyType::p_escape, strongLevel, data);
		m_mEscape += data.m_escape;
		m_mEscape += getStrongProp(PropertyType::m_escape, strongLevel, data);
		m_crit += data.crit;
		m_crit += getStrongProp(PropertyType::crit, strongLevel, data);
		m_antiCrit += data.antiCrit;
		m_antiCrit += getStrongProp(PropertyType::antiCrit, strongLevel, data);

		m_lucky += data.lucky;
		m_lucky += getStrongProp(PropertyType::lucky, strongLevel, data);
		m_damageAddLv += data.damageAddLv;
		m_damageAddLv += getStrongProp(PropertyType::damageAddLv, strongLevel, data);
		m_damageReduceLv += data.damageReduceLv;
		m_damageReduceLv += getStrongProp(PropertyType::damageReduceLv, strongLevel, data);
		m_hpLv += data.hpLv;
		m_hpLv += getStrongProp(PropertyType::hpLv, strongLevel, data);
		m_mpLv += data.mpLv;
		m_mpLv += getStrongProp(PropertyType::mpLv, strongLevel, data);

        m_antiDropEquip += data.antiDropEquip;
		m_antiDropEquip += getStrongProp(PropertyType::antiDropEquip, strongLevel, data);
	}

	//计算全身强化属性奖励
	calculateRewardStrongProp();	

	//计算翅膀注灵属性加成
	calculateWingZhulingRewardProp();
	return;
}

uint8_t EquipPackage::getSuitNum(uint32_t suitId) const
{
	uint8_t count = 0;

	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;

		if(iter->objPtr->suitId() != suitId)
			continue;
	
		count += 1;
	}

	return count;
}

void EquipPackage::resetSuitPropList()
{
	m_skillOfSuitMap.clear();

	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;

		//一次性将某类型的套装属性全部记录，因此不再处理相同套装的物品
		uint32_t suitId = iter->objPtr->suitId();
		auto item = m_skillOfSuitMap.find(suitId);
		if(item != m_skillOfSuitMap.end())	
			continue;

		const auto& cfg = SuitConfig::me().suitCfg;
		auto pos = cfg.m_suitPropMap.find(suitId);
		if(pos == cfg.m_suitPropMap.end())
			continue;

		uint32_t suitNum = getSuitNum(suitId);
		std::vector<uint32_t> skillVec;
		for(auto iterProp = pos->second.begin(); iterProp != pos->second.end(); ++iterProp)
		{
			if(suitNum < iterProp->suitNum)
				continue;
			
			skillVec.push_back(iterProp->skillId);
		}

		if(skillVec.empty())
			continue;

		m_skillOfSuitMap.insert(std::make_pair(suitId, skillVec));
	}

	return;
}

const std::map<uint32_t, std::vector<uint32_t> >& EquipPackage::getSkillOfSuitMap() const
{
	return m_skillOfSuitMap;
}

//根据属性类型获取装备强化属性加成
uint32_t EquipPackage::getStrongProp(PropertyType type, uint8_t strongLevel, const ObjBasicData& data) const
{
	if(0 == strongLevel)
		return 0;

	for(auto iter = data.strongPropVec.begin(); iter != data.strongPropVec.end(); ++iter)
	{
		if(iter->level != strongLevel)
			continue;

		if(iter->propType != type)
			continue;

		return iter->prop;
	}

	return 0;
}

//计算全身强化属性奖励
void EquipPackage::calculateRewardStrongProp()
{
	if(!canGetRewardStrongProp())
		return;

	const uint8_t minStrongLevel = getMinStrongLevel();
	if(0 == minStrongLevel)
		return;

	const auto& cfg = StrongConfig::me().strongCfg;
	auto pos = cfg.m_strongMap.find(minStrongLevel);
	if(pos == cfg.m_strongMap.end())
		return;

	const auto& rewardPropMap = pos->second.rewardPropMap;
	if(rewardPropMap.empty())
		return;

	m_hp += getRewardStrongProp(PropertyType::maxhp, rewardPropMap);
	m_mp += getRewardStrongProp(PropertyType::maxmp, rewardPropMap);
	m_pAttackMin += getRewardStrongProp(PropertyType::p_attackMin, rewardPropMap);
	m_pAttackMax += getRewardStrongProp(PropertyType::p_attackMax, rewardPropMap);
	m_mAttackMin += getRewardStrongProp(PropertyType::m_attackMin, rewardPropMap);
	m_mAttackMax += getRewardStrongProp(PropertyType::m_attackMax, rewardPropMap);
	m_witchMin += getRewardStrongProp(PropertyType::witchMin, rewardPropMap);
	m_witchMax += getRewardStrongProp(PropertyType::witchMax, rewardPropMap);
	m_pDefenceMin += getRewardStrongProp(PropertyType::p_defenceMin, rewardPropMap);
	m_pDefenceMax += getRewardStrongProp(PropertyType::p_defenceMax, rewardPropMap);
	m_mDefenceMin += getRewardStrongProp(PropertyType::m_defenceMin, rewardPropMap);
	m_mDefenceMax += getRewardStrongProp(PropertyType::m_defenceMax, rewardPropMap);

	m_shot += getRewardStrongProp(PropertyType::shot, rewardPropMap);
	m_pEscape += getRewardStrongProp(PropertyType::p_escape, rewardPropMap);
	m_mEscape += getRewardStrongProp(PropertyType::m_escape, rewardPropMap);
	m_crit += getRewardStrongProp(PropertyType::crit, rewardPropMap);
	m_antiCrit += getRewardStrongProp(PropertyType::antiCrit, rewardPropMap);

	m_lucky += getRewardStrongProp(PropertyType::lucky, rewardPropMap);
	m_damageAddLv += getRewardStrongProp(PropertyType::damageAddLv, rewardPropMap);
	m_damageReduceLv += getRewardStrongProp(PropertyType::damageReduceLv, rewardPropMap);
	m_hpLv += getRewardStrongProp(PropertyType::hpLv, rewardPropMap);
	m_mpLv += getRewardStrongProp(PropertyType::mpLv, rewardPropMap);

	m_antiDropEquip += getRewardStrongProp(PropertyType::antiDropEquip, rewardPropMap);
	return;
}

//获取全身强化奖励属性
uint32_t EquipPackage::getRewardStrongProp(PropertyType type, const std::map<PropertyType, uint32_t>& rewardPropMap) const
{
	if(rewardPropMap.empty())
		return 0;

	auto pos = rewardPropMap.find(type);
	if(pos == rewardPropMap.end())
		return 0;

	return pos->second;
}

//获取全身装备最小强化等级
uint8_t EquipPackage::getMinStrongLevel() const
{
	const auto cfg = StrongConfig::me().strongCfg;
	if(cfg.m_strongMap.empty())
		return 0;

	uint8_t minLevel = cfg.m_strongMap.size();
	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	if(cellVec.empty())
		return 0;

	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			return 0;

		//翅膀不能强化
		if(iter->objPtr->childType() == ObjChildType::wing)
			continue;

		const uint8_t strongLevel = iter->objPtr->strongLevel();
		if(strongLevel <= minLevel)
			minLevel = strongLevel;
	}

	return minLevel;
}

//判断是否可以获得全身强化属性奖励
bool EquipPackage::canGetRewardStrongProp()
{
	uint8_t strongNum = 0;
	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;

		if(0 == iter->objPtr->strongLevel())
			continue;

		strongNum += 1;
	}

	//翅膀不可以强化，故-1
	if(strongNum == MAX_CELL_NUM_OF_EQUIP - 1)
		return true;

	return false;
}

uint32_t EquipPackage::getWeaponLuckyRewardProp() const
{
	//武器在第0个格子
	const uint8_t cell = 0;
	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	if(cell >= cellVec.size())
		return 0;

	if(cellVec[cell].objPtr == nullptr)
		return 0;
	
	const uint8_t level = cellVec[cell].objPtr->luckyLevel();
	const auto& cfg = LuckyConfig::me().luckyCfg;
	auto pos = cfg.m_luckyMap.find(level);
	if(pos == cfg.m_luckyMap.end())
		return 0;

	return pos->second.rewardLuckyNum;
}

//计算翅膀注灵属性加成
void EquipPackage::calculateWingZhulingRewardProp()
{
	const auto& cellVec = Package::getCellVec();
	if(9 >= cellVec.size())
		return;
	
	if(cellVec[9].objPtr == nullptr)
		return;

	uint8_t level = 0;
	if(sceneItemType() == SceneItemType::role)
	{
		Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
		if(role == nullptr)
			return;

		 level = role->m_wing.getLingliLevel();
	}
	else if(sceneItemType() == SceneItemType::hero)
	{
		Hero::Ptr hero = std::static_pointer_cast<Hero>(getOwner());
		if(hero == nullptr)
			return;

		level = hero->m_wing.getLingliLevel();
	}

	if(0 == level)
		return;

	const auto& cfg = WingConfig::me().wingCfg;
	auto pos = cfg.m_rewardMap.find(level);
	if(pos == cfg.m_rewardMap.end())
		return;

	const uint32_t precent = pos->second.addPropPercent;
	const ObjBasicData& data = cellVec[9].objPtr->getObjBasicData();
	m_hp += SAFE_DIV(data.hp * precent, 100);
	m_mp += SAFE_DIV(data.mp * precent, 100);
	m_pAttackMin += SAFE_DIV(data.p_attackMin * precent, 100);
	m_pAttackMax += SAFE_DIV(data.p_attackMax * precent, 100);
	m_mAttackMin += SAFE_DIV(data.m_attackMin * precent, 100);
	m_mAttackMax += SAFE_DIV(data.m_attackMax * precent, 100);
	m_witchMin += SAFE_DIV(data.witchMin * precent, 100);
	m_witchMax += SAFE_DIV(data.witchMax * precent, 100);
	m_pDefenceMin += SAFE_DIV(data.p_defenceMin * precent, 100);
	m_pDefenceMax += SAFE_DIV(data.p_defenceMax * precent, 100);
	m_mDefenceMin += SAFE_DIV(data.m_defenceMin * precent, 100);
	m_mDefenceMax += SAFE_DIV(data.m_defenceMax * precent, 100);

	m_shot += SAFE_DIV(data.shot * precent, 100);
	m_pEscape += SAFE_DIV(data.p_escape * precent, 100);
	m_mEscape += SAFE_DIV(data.m_escape * precent, 100);
	m_crit += SAFE_DIV(data.crit * precent, 100);
	m_antiCrit += SAFE_DIV(data.antiCrit * precent, 100);

	m_lucky += SAFE_DIV(data.lucky * precent, 100);
	m_damageAddLv += SAFE_DIV(data.damageAddLv * precent, 100);
	m_damageReduceLv += SAFE_DIV(data.damageReduceLv * precent, 100);
	m_hpLv += SAFE_DIV(data.hpLv * precent, 100);
	m_mpLv += SAFE_DIV(data.mpLv * precent, 100);

	m_antiDropEquip += SAFE_DIV(data.antiDropEquip * precent, 100);

	return;
}


//获取生命
uint32_t EquipPackage::getHp() const
{
	return m_hp;
}

//获取魔法
uint32_t EquipPackage::getMp() const
{
	return m_mp;
}

//获取生命等级
uint32_t EquipPackage::getHpLv() const
{
	return m_hpLv;
}

//获取魔法等级
uint32_t EquipPackage::getMpLv() const
{
	return m_mpLv;
}

//获取物攻Min
uint32_t EquipPackage::getPAtkMin() const
{
	return m_pAttackMin;
}

//获取物攻Max
uint32_t EquipPackage::getPAtkMax() const
{
	return m_pAttackMax;
}

//获取魔攻Min
uint32_t EquipPackage::getMAtkMin() const
{
	return m_mAttackMin;
}

//获取魔攻Max
uint32_t EquipPackage::getMAtkMax() const
{
	return m_mAttackMax;
}

//获取道术Min
uint32_t EquipPackage::getWitchMin() const
{
	return m_witchMin;
}

//获取道术Max
uint32_t EquipPackage::getWitchMax() const
{
	return m_witchMax;
}

//获取物防Min
uint32_t EquipPackage::getPDefMin() const
{
	return m_pDefenceMin;
}

//获取物防Max
uint32_t EquipPackage::getPDefMax() const
{
	return m_pDefenceMax;
}

//获取魔防Min
uint32_t EquipPackage::getMDefMin() const
{
	return m_mDefenceMin;
}

//获取魔防Max
uint32_t EquipPackage::getMDefMax() const
{
	return m_mDefenceMax;
}

//获取幸运
uint32_t EquipPackage::getLucky() const
{
	uint32_t ret = m_lucky;
	ret += getWeaponLuckyRewardProp();
	return ret;
}

//获取诅咒
uint32_t EquipPackage::getEvil() const
{
	return m_evil;
}

//获取命中
uint32_t EquipPackage::getShot() const
{
	return m_shot;
}

//获取命中率
uint32_t EquipPackage::getShotRatio() const
{
	return m_shotRatio;
}

//获取物闪
uint32_t EquipPackage::getPEscape() const
{
	return m_pEscape;
}

//获取魔闪
uint32_t EquipPackage::getMEscape() const
{
	return m_mEscape;
}

//获取闪避率
uint32_t EquipPackage::getEscapeRatio() const
{
	return m_escapeRatio;
}

//获取暴击
uint32_t EquipPackage::getCrit() const
{
	return m_crit;
}

//获取暴击率
uint32_t EquipPackage::getCritRatio() const
{
	return m_critRatio;
}

//获取防爆(暴击抗性)
uint32_t EquipPackage::getAntiCrit() const
{
	return m_antiCrit;
}

//获取暴伤
uint32_t EquipPackage::getCritDamage() const
{
	return m_critDamage;
}

//获取增伤
uint32_t EquipPackage::getDamageAdd() const
{
	return m_damageAdd;
}

//获取增伤等级
uint32_t EquipPackage::getDamageAddLv() const
{
	return m_damageAddLv;
}

//获取减伤
uint32_t EquipPackage::getDamageReduce() const
{
	return m_damageReduce;
}

//获取减伤等级
uint32_t EquipPackage::getDamageReduceLv() const
{
	return m_damageReduceLv;
}

//防爆(装备掉落)
uint32_t EquipPackage::getAntiDropEquip() const
{
    return m_antiDropEquip;
}


}
