#include "role.h"
#include "world.h"
#include "scene.h"
#include "stone_package.h"
#include "suit_config.h"
#include "stone_config.h"

#include "water/common/commdef.h"
#include "water/componet/logger.h"
#include "water/process/process_id.h"

namespace world{

StonePackage::StonePackage(SceneItemType sceneItem, PackageType packageType, uint16_t totalCellNum, uint16_t unlockNum, const std::vector<CellInfo>& cellVec)
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
}

void StonePackage::setOwner(std::shared_ptr<PK> owner)
{
	if(owner == nullptr)
		return;

	Package::setOwner(owner);
	
	calculateAllAttribute();
	return;
}

//镶嵌宝石不支持不指定格子
bool StonePackage::checkPutObj(Object::Ptr obj, uint16_t num, Bind bind)
{
	return false;
}

bool StonePackage::checkPutObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind)
{
	Role::Ptr role = getRole(getOwner());
	if(role == nullptr)
		return false;

	if(obj == nullptr || 1 != num || bind == Bind::none)
		return false;

	//不允许将非宝石物品放入宝石包
	if(obj->parentType() != ObjParentType::stone)
		return false;

	//0~3、4~7、8~11、12~15、16~19...为一组、分别是一个装备位
	const uint16_t beginCell = SAFE_SUB(cell, SAFE_MOD(cell, 4));
	const uint16_t endCell = beginCell + 3;

	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	if(cell >= cellVec.size())
		return false;

	if(beginCell >= cellVec.size() || endCell >= cellVec.size())
		return false;

	ObjChildType stoneType = obj->childType();
	for(uint16_t i = beginCell; i <= endCell; i++)
	{
		if(cellVec[i].objPtr == nullptr)
			continue;

		if(cellVec[i].objPtr->childType() == stoneType)
		{
			role->sendSysChat("无法镶嵌更多的同类型宝石");
			return false;
		}
	}

	return true;
}

//宝石包不支持交换格子
bool StonePackage::exchangeCell(uint16_t fromCell, uint16_t toCell)
{
	return false;
}

//镶嵌宝石不支持不指定格子
uint16_t StonePackage::putObj(Object::Ptr obj, uint16_t num, Bind bind, bool newObj/*=false*/)
{
	return 0;
}

uint16_t StonePackage::putObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, bool newObj/*=false*/)
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

void StonePackage::sendAllAttributeToMe()
{
	calculateAllAttribute();
	
	if(sceneItemType() == SceneItemType::role)
	{
		Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
		if(role == nullptr)
			return;

		role->sendMainToMe();
	}

	if(sceneItemType() == SceneItemType::hero)
	{
		Hero::Ptr hero = std::static_pointer_cast<Hero>(getOwner());   
		if(hero == nullptr)
			return;

		hero->sendMainToMe();
	}

	return;
}


void StonePackage::resetAllAttribute()
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

uint32_t StonePackage::getStoneTotalLevel() const
{
	uint32_t totalLevel = 0;
	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;
		
		const ObjBasicData& data = iter->objPtr->getObjBasicData();
		totalLevel += data.objLevel;
	}

	return totalLevel;
}

void StonePackage::calculateAllAttribute()
{
	resetAllAttribute();

	PK::Ptr owner = getOwner();
	if(owner == nullptr)
		return;

	uint32_t objTotalLevel = 0;
	const std::vector<CellInfo>& cellVec = Package::getCellVec();
	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;
		
		const ObjBasicData& data = iter->objPtr->getObjBasicData();
		m_hp += data.hp;
		m_mp += data.mp;
		m_pAttackMin += data.p_attackMin;
		m_pAttackMax += data.p_attackMax;
		m_mAttackMin += data.m_attackMin;
		m_mAttackMax += data.m_attackMax;
		m_witchMin += data.witchMin;
		m_witchMax += data.witchMax;
		m_pDefenceMin += data.p_defenceMin;
		m_pDefenceMax += data.p_defenceMax;
		m_mDefenceMin += data.m_defenceMin;
		m_mDefenceMax += data.m_defenceMax;

		m_shot += data.shot;
		m_pEscape += data.p_escape;
		m_mEscape += data.m_escape;
		m_crit += data.crit;
		m_antiCrit += data.antiCrit;

		m_lucky += data.lucky;
		m_damageAddLv += data.damageAddLv;
		m_damageReduceLv += data.damageReduceLv;
		m_hpLv += data.hpLv;
		m_mpLv += data.mpLv;

        m_antiDropEquip += data.antiDropEquip;

		objTotalLevel += data.objLevel;
	}

	//宝石等级加成
	Job job = owner->job();
	const auto& cfg = StoneConfig::me().stoneCfg;
	for(auto iter = cfg.m_stoneVec.begin(); iter != cfg.m_stoneVec.end(); ++iter)
	{
		if(static_cast<Job>(iter->job) != job)
			continue;

		if(objTotalLevel >= iter->levelMin && objTotalLevel <= iter->levelMax)
		{
			m_hp += iter->hp;
			m_mp += iter->mp;
			m_pAttackMin += iter->p_attackMin;
			m_pAttackMax += iter->p_attackMax;
			m_mAttackMin += iter->m_attackMin;
			m_mAttackMax += iter->m_attackMax;
			m_witchMin += iter->witchMin;
			m_witchMax += iter->witchMax;
			m_pDefenceMin += iter->p_defenceMin;
			m_pDefenceMax += iter->p_defenceMax;
			m_mDefenceMin += iter->m_defenceMin;
			m_mDefenceMax += iter->m_defenceMax;

			m_shot += iter->shot;
			m_pEscape += iter->p_escape;
			m_mEscape += iter->m_escape;
			m_crit += iter->crit;
			m_antiCrit += iter->antiCrit;

			m_lucky += iter->lucky;
			m_damageAddLv += iter->damageAddLv;
			m_damageReduceLv += iter->damageReduceLv;
			m_hpLv += iter->hpLv;
			m_mpLv += iter->mpLv;

			m_antiDropEquip += iter->antiDropEquip;
		}
	}
	
	return;
}



//获取生命
uint32_t StonePackage::getHp() const
{
	return m_hp;
}

//获取魔法
uint32_t StonePackage::getMp() const
{
	return m_mp;
}

//获取生命等级
uint32_t StonePackage::getHpLv() const
{
	return m_hpLv;
}

//获取魔法等级
uint32_t StonePackage::getMpLv() const
{
	return m_mpLv;
}

//获取物攻Min
uint32_t StonePackage::getPAtkMin() const
{
	return m_pAttackMin;
}

//获取物攻Max
uint32_t StonePackage::getPAtkMax() const
{
	return m_pAttackMax;
}

//获取魔攻Min
uint32_t StonePackage::getMAtkMin() const
{
	return m_mAttackMin;
}

//获取魔攻Max
uint32_t StonePackage::getMAtkMax() const
{
	return m_mAttackMax;
}

//获取道术Min
uint32_t StonePackage::getWitchMin() const
{
	return m_witchMin;
}

//获取道术Max
uint32_t StonePackage::getWitchMax() const
{
	return m_witchMax;
}

//获取物防Min
uint32_t StonePackage::getPDefMin() const
{
	return m_pDefenceMin;
}

//获取物防Max
uint32_t StonePackage::getPDefMax() const
{
	return m_pDefenceMax;
}

//获取魔防Min
uint32_t StonePackage::getMDefMin() const
{
	return m_mDefenceMin;
}

//获取魔防Max
uint32_t StonePackage::getMDefMax() const
{
	return m_mDefenceMax;
}

//获取幸运
uint32_t StonePackage::getLucky() const
{
	return m_lucky;
}

//获取诅咒
uint32_t StonePackage::getEvil() const
{
	return m_evil;
}

//获取命中
uint32_t StonePackage::getShot() const
{
	return m_shot;
}

//获取命中率
uint32_t StonePackage::getShotRatio() const
{
	return m_shotRatio;
}

//获取物闪
uint32_t StonePackage::getPEscape() const
{
	return m_pEscape;
}

//获取魔闪
uint32_t StonePackage::getMEscape() const
{
	return m_mEscape;
}

//获取闪避率
uint32_t StonePackage::getEscapeRatio() const
{
	return m_escapeRatio;
}

//获取暴击
uint32_t StonePackage::getCrit() const
{
	return m_crit;
}

//获取暴击率
uint32_t StonePackage::getCritRatio() const
{
	return m_critRatio;
}

//获取防爆(暴击抗性)
uint32_t StonePackage::getAntiCrit() const
{
	return m_antiCrit;
}

//获取暴伤
uint32_t StonePackage::getCritDamage() const
{
	return m_critDamage;
}

//获取增伤
uint32_t StonePackage::getDamageAdd() const
{
	return m_damageAdd;
}

//获取增伤等级
uint32_t StonePackage::getDamageAddLv() const
{
	return m_damageAddLv;
}

//获取减伤
uint32_t StonePackage::getDamageReduce() const
{
	return m_damageReduce;
}

//获取减伤等级
uint32_t StonePackage::getDamageReduceLv() const
{
	return m_damageReduceLv;
}

//防爆(装备掉落)
uint32_t StonePackage::getAntiDropEquip() const
{
    return m_antiDropEquip;
}


}
