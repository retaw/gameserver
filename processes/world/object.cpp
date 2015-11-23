#include "object.h"
#include "water/common/commdef.h"

namespace world{

Object::Object(ObjBasicData info)
: m_data(info)
{

}



std::string Object::name()
{
	return m_data.name;
}

ObjectId Object::objId() const
{
	return m_data.objId;
}

BroadCast Object::broadCast() const
{
    return m_data.broadCast;
}

ObjParentType Object::parentType() const
{
	uint16_t type = SAFE_DIV(m_data.childType, 100) * 100;
	return static_cast<ObjParentType>(type);
}

ObjChildType Object::childType() const
{
	return static_cast<ObjChildType>(m_data.childType);
}

uint16_t Object::maxStackNum() const
{
	return m_data.maxStackNum;
}

TplId Object::tplId() const
{
	return m_data.tplId;
}

uint8_t Object::quality() const
{
	return m_data.quality;
}

uint8_t Object::level() const
{
	return m_data.level;
}

Job Object::job() const
{
	return static_cast<Job>(m_data.job);
}

Sex Object::sex() const
{
	return static_cast<Sex>(m_data.sex);
}

TurnLife Object::turnLife() const
{
	return static_cast<TurnLife>(m_data.turnLife);
}

MoneyType Object::needMoneyType() const
{
	return static_cast<MoneyType>(m_data.needMoneyType);
}

uint32_t Object::needMoneyNum() const
{
	return m_data.needMoneyNum;
}

uint32_t Object::nonsuchId() const
{
	return m_data.nonsuchId;
}

uint32_t Object::suitId() const
{
	return m_data.suitId;
}

uint32_t Object::skillId() const
{
	return m_data.skillId;
}

bool Object::bBatUse() const
{
	return m_data.bBatUse;
}

bool Object::bDiscard() const
{
	return m_data.bDiscard;
}

MoneyType Object::moneyType() const
{
    return static_cast<MoneyType>(m_data.moneyType);
}

uint32_t Object::price() const
{
    return m_data.price;
}

uint32_t Object::prob() const
{
    return m_data.prob;
}

uint32_t Object::objLevel() const
{
	return m_data.objLevel;
}

uint32_t Object::spe1() const
{
	return m_data.sep1;
}

uint32_t Object::spe2() const
{
	return m_data.sep2;
}

uint32_t Object::spe3() const
{
	return m_data.sep3;
}

const std::vector<FenjieRewardItem>& Object::fenjieRewardVec() const
{
	return m_data.fenjieRewardVec;
}


void Object::setStrongLevel(uint8_t level)
{
	m_data.strongLevel = level;
}

uint8_t Object::strongLevel() const
{
	return m_data.strongLevel;
}

void Object::setLuckyLevel(uint8_t level)
{
	m_data.luckyLevel = level;
}

uint8_t Object::luckyLevel() const
{
	return m_data.luckyLevel;
}

bool Object::isEquipment() const
{
	if(parentType() == ObjParentType::equip)
		return true;

	return false;
}

const ObjBasicData& Object::getObjBasicData() const
{
	return m_data;
}

}
