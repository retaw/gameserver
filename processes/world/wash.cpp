#include "wash.h"
#include "wash_config.h"
#include "pk.h"
#include "role.h"
#include "role_manager.h"
#include "hero.h"

#include "water/common/commdef.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/wash.h"
#include "protocol/rawmsg/public/wash.codedef.public.h"

#include "protocol/rawmsg/private/wash.h"
#include "protocol/rawmsg/private/wash.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

Wash::Wash(SceneItemType sceneItem, RoleId roleId, PK& owner)
: m_sceneItem(sceneItem)
, m_roleId(roleId)
, m_owner(owner)
{
}

void Wash::loadFromDB(const std::vector<WashPropInfo>& propVec)
{
	m_curWashMap.clear();
	for(auto iter = propVec.begin(); iter != propVec.end(); ++iter)
	{
		auto& item = m_curWashMap[iter->washType];
		
		PropItem temp;
		temp.propType = iter->propType;
		temp.prop = iter->prop;
		temp.quality = iter->quality;
		temp.lock = false;

		item[iter->group].push_back(temp);
	}

	calcAttribute();
	return;
}

void Wash::sendCurPropList(uint8_t type)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	std::vector<uint8_t> buf;
	buf.reserve(512);
	buf.resize(sizeof(PublicRaw::RetPropList));

	auto* msg = reinterpret_cast<PublicRaw::RetPropList*>(buf.data());
	msg->sceneItem = m_sceneItem;
	msg->type = type;
	msg->size = 0;

	auto pos = m_curWashMap.find(type);
	if(pos == m_curWashMap.end())
	{
		role->sendToMe(RAWMSG_CODE_PUBLIC(RetPropList), buf.data(), buf.size());
		return;	
	}

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		for(auto item = iter->second.begin(); item != iter->second.end(); ++item)
		{
			buf.resize(buf.size() + sizeof(msg->data[0]));
			auto* msg  = reinterpret_cast<PublicRaw::RetPropList*>(buf.data());

			msg->data[msg->size].propType = item->propType;
			msg->data[msg->size].prop = item->prop;
			msg->data[msg->size].quality = item->quality;

			LOG_TRACE("洗练, 返回当前属性, roleId={}, sceneItem={}, group={}, propType={}, prop={}, quality={}",
					  role->id(), m_sceneItem, iter->first, msg->data[msg->size].propType, 
					  msg->data[msg->size].prop, msg->data[msg->size].quality);
			msg->size++;
		}
	}

	role->sendToMe(RAWMSG_CODE_PUBLIC(RetPropList), buf.data(), buf.size());		
	return;
}

void Wash::requestLockOrUnlockProp(uint8_t type, uint8_t group, bool lock)
{
	if(!checkLevel(type))
		return;

	auto pos = m_curWashMap.find(type);
	if(pos == m_curWashMap.end())
		return;

	auto iter = pos->second.find(group);
	if(iter == pos->second.end())
		return;

	for(auto item = iter->second.begin(); item != iter->second.end(); ++item)
	{
		item->lock = lock;
	}

	return;
}

void Wash::requestWash(uint8_t type, uint8_t washWay)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	if(!checkLevel(type))
		return;

	if(!checkAndReduceMoney(type, washWay))
		return;

	m_washResultMap.clear();

	for(uint8_t i = 1; i <= 4; )
	{
		const uint8_t quality = randomQuality(type, washWay);
		if(0 == quality)
		{
			LOG_ERROR("洗练, 随机品质错误, 洗练失败, roleId={}, sceneItem={}, type={}, washWay={}",
					  role->id(), m_sceneItem, type, washWay);
			return;
		}

		const PropertyType propType = randomPropType(type);
		if(propType == PropertyType::none)
		{
			LOG_ERROR("洗练, 随机属性类型错误, 洗练失败, roleId={}, sceneItem={}, type={}, washWay={}", 
					  role->id(), m_sceneItem, type, washWay);
			return;
		}

		if(isPropTypeExit(type, propType))
			continue;

		const uint32_t propValue = randmonPropValue(type, propType, quality);
		if(0 == propValue)
		{
			LOG_ERROR("洗练, 随机属性值错误, 洗练失败, roleId={}, sceneItem={}, type={}, washWay={}", 
					  role->id(), m_sceneItem, type, washWay);
			return;
		}
	
		auto& item = m_washResultMap[type]; 
		if(isSpecialPropType(propType))
		{
			componet::Random<uint32_t> minPropValue(0, propValue);
			PropItem temp;
			temp.propType = getMinPropTypeByPropType(propType);
			temp.prop = minPropValue.get();
			temp.quality = quality;
			temp.lock = false;

			item[i].push_back(temp);
		}

		PropItem temp;
		temp.propType = propType;
		temp.prop = propValue;
		temp.quality = quality;
		temp.lock = false;

		item[i].push_back(temp);
		
		i += 1;
	}

	sendWashPropResult(type);
	return;
}

void Wash::requestReplaceCurProp(uint8_t type)
{
	const auto posResult = m_washResultMap.find(type);
	if(posResult == m_washResultMap.end())
		return;

	const auto posCur = m_curWashMap.find(type);
	if(posCur == m_curWashMap.end())
	{
		m_curWashMap[type] = posResult->second;
	}
	else
	{
		std::map<uint8_t, std::vector<PropItem> > tempPropMap;
		tempPropMap.clear();
		for(uint8_t i = 1; i <= 4; i++)
		{
			const auto iterCur = posCur->second.find(i);
			const auto iterResult = posResult->second.find(i);
			if(iterCur == posCur->second.end() || iterResult == posResult->second.end())
				return;

			bool lockFlag = false;
			for(auto iter = iterCur->second.begin(); iter != iterCur->second.end(); ++iter)
			{
				if(iter->lock)
				{	
					lockFlag = true;
					break;
				}
			}

			if(lockFlag)
			{
				//锁定，则保留当前属性
				tempPropMap[i] = iterCur->second;
			}
			else
			{
				//非锁定，则用洗练出的属性替换当前属性
				tempPropMap[i] = iterResult->second;
			}
		}

		m_curWashMap[type] = tempPropMap;
	}

	//world->db
	updateWashPropToDB(type);		

	//s->c
	sendCurPropList(type);
	calcAttribute();
	m_owner.sendMainToMe();
	return;
}

void Wash::sendWashPropResult(uint8_t type)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	std::vector<uint8_t> buf;
	buf.reserve(512);
	buf.resize(sizeof(PublicRaw::RetWashPropResult));

	auto* msg = reinterpret_cast<PublicRaw::RetWashPropResult*>(buf.data());
	msg->sceneItem = m_sceneItem;
	msg->type = type;
	msg->size = 0;

	auto pos = m_washResultMap.find(type);
	if(pos == m_washResultMap.end())
	{
		role->sendToMe(RAWMSG_CODE_PUBLIC(RetWashPropResult), buf.data(), buf.size());		
		return;
	}
		
	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		for(auto item = iter->second.begin(); item != iter->second.end(); ++item)
		{
			buf.resize(buf.size() + sizeof(msg->data[0]));
			auto* msg  = reinterpret_cast<PublicRaw::RetWashPropResult*>(buf.data());

			msg->data[msg->size].propType = item->propType;
			msg->data[msg->size].prop = item->prop;
			msg->data[msg->size].quality = item->quality;

			LOG_TRACE("洗练, 返回洗练结果, roleId={}, sceneItem={}, group={}, propType={}, prop={}, quality={}",
					  role->id(), m_sceneItem, iter->first, msg->data[msg->size].propType, 
					  msg->data[msg->size].prop, msg->data[msg->size].quality);
			++msg->size;
		}
	}

	role->sendToMe(RAWMSG_CODE_PUBLIC(RetWashPropResult), buf.data(), buf.size());		
	return;
}

void Wash::updateWashPropToDB(uint8_t type)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	auto pos = m_curWashMap.find(type);
	if(pos == m_curWashMap.end())
		return;

	std::vector<uint8_t> buf;
	buf.reserve(512);
	buf.resize(sizeof(PrivateRaw::UpdateOrInsertWashProp));

	auto* msg = reinterpret_cast<PrivateRaw::UpdateOrInsertWashProp*>(buf.data());
	msg->roleId = role->id();	
	msg->sceneItem = m_sceneItem;
	msg->washType = type;
	msg->size = 0;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		for(auto item = iter->second.begin(); item != iter->second.end(); ++item)
		{
			buf.resize(buf.size() + sizeof(WashPropInfo));
			auto* msg  = reinterpret_cast<PrivateRaw::UpdateOrInsertWashProp*>(buf.data());
	
			msg->data[msg->size].washType = type;
			msg->data[msg->size].group = iter->first;
			msg->data[msg->size].quality = item->quality;
			msg->data[msg->size].propType = item->propType;
			msg->data[msg->size].prop = item->prop;

			msg->size++;
		}
	}

	ProcessIdentity dbcachedId("dbcached", 1);
	World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateOrInsertWashProp), buf.data(), buf.size());
	
	return;
}

bool Wash::checkLevel(uint8_t type)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return false;

	const auto& cfg = WashConfig::me().washCfg;
	auto pos = cfg.m_baseMap.find(type);
	if(pos == cfg.m_baseMap.end())
		return false;

	if(pos->second.level > m_owner.level())
	{
		role->sendSysChat("等级不足");
		return false;
	}

	return true;
}


Role::Ptr Wash::getRole() 
{
	Role::Ptr role = RoleManager::me().getById(m_roleId);
	if(role == nullptr)
		return nullptr;

	return role;
}

bool Wash::checkAndReduceMoney(uint8_t type, uint8_t washWay)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return false;

	const auto& cfg = WashConfig::me().washCfg;
	auto pos = cfg.m_consumeMap.find(type);
	if(pos == cfg.m_consumeMap.end())
		return false;

	auto iter = pos->second.find(washWay);
	if(iter == pos->second.end())
		return false;

	const uint8_t lockNum = getCurLockPropNum(type);
	MoneyType needMoneyType = iter->second.needMoneyType;
	uint32_t needMoneyNum = iter->second.needMoneyNum;
	uint32_t lockNeedYuanbao = iter->second.lockNeedYuanbao * lockNum;

	//检查金币
	if(!role->checkMoney(needMoneyType, needMoneyNum))
		return false;

	//检查锁定所需元宝
	if(0 != lockNeedYuanbao)
	{
		if(!role->checkMoney(MoneyType::money_4, lockNeedYuanbao))
			return false;
	}

	//扣除金币
	if(!role->reduceMoney(needMoneyType, needMoneyNum, "洗练"))
		return false;

	//扣除锁定所需元宝
	if(0 != lockNeedYuanbao)
	{
		if(!role->reduceMoney(MoneyType::money_4, lockNeedYuanbao, "洗练"))
			return false;
	}

	return true;
}

uint8_t Wash::getCurLockPropNum(uint8_t type)
{
	auto pos = m_curWashMap.find(type);
	if(pos == m_curWashMap.end())
		return 0;

	uint8_t count = 0;
	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		for(auto item = iter->second.begin(); item != iter->second.end(); ++item)
		{
			if(!item->lock)
				continue;

			count++;
			break;
		}
	}

	return count;
}

uint8_t Wash::randomQuality(uint8_t type, uint8_t washWay)
{
	const auto& cfg = WashConfig::me().washCfg;
	auto pos = cfg.m_consumeMap.find(type);
	if(pos == cfg.m_consumeMap.end())
		return 0;

	auto iter = pos->second.find(washWay);
	if(iter == pos->second.end())
		return 0;

	if(iter->second.qualityVec.empty())
		return 0;
	
	componet::Random<uint32_t> rand(0, iter->second.qualityVec.size() - 1);
	return iter->second.qualityVec[rand.get()];
}

PropertyType Wash::randomPropType(uint8_t type)
{
	const auto& cfg = WashConfig::me().washCfg;
	auto pos = cfg.m_baseMap.find(type);
	if(pos == cfg.m_baseMap.end())
		return PropertyType::none;

	if(pos->second.propTypeVec.empty())
		return PropertyType::none;

	componet::Random<uint32_t> rand(0, pos->second.propTypeVec.size() - 1);
	return pos->second.propTypeVec[rand.get()];
}

uint32_t Wash::randmonPropValue(uint8_t type, PropertyType propType, uint8_t quality)
{
	if(0 == quality)
		return 0;

	const auto& cfg = WashConfig::me().washCfg;
	auto pos = cfg.m_propMap.find(type);
	if(pos == cfg.m_propMap.end())
		return 0;

	auto iter = pos->second.find(propType);
	if(iter == pos->second.end())
		return 0;

	const uint8_t index = quality - 1 ;
	if(index >= iter->second.propValueVec.size())
		return 0;

	componet::Random<uint32_t> rand(1, 2);
	const uint8_t randNum = rand.get();
	if(1 == randNum)
	{
		return iter->second.propValueVec[index].first;
	}
	else if(2 == randNum)
	{
		return iter->second.propValueVec[index].second;
	}

	return 0;
}

bool Wash::isPropTypeExit(uint8_t type, PropertyType propType)
{
	auto pos = m_washResultMap.find(type);
	if(pos == m_washResultMap.end())
		return false;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		for(auto item = iter->second.begin(); item != iter->second.end(); ++item)
		{
			if(item->propType != propType)
				continue;

			return true;
		}
	}

	return false;
}

bool Wash::isSpecialPropType(PropertyType propType)
{
	switch(propType)
	{
	case PropertyType::p_attackMax:
		return true;
	case PropertyType::m_attackMax:
		return true;
	case PropertyType::witchMax:
		return true;
	case PropertyType::p_defenceMax:
		return true;
	case PropertyType::m_defenceMax:
		return true;
	default:
		break;
	}

	return false;
}

PropertyType Wash::getMinPropTypeByPropType(PropertyType propType)
{
	switch(propType)
	{
	case PropertyType::p_attackMax:
		return  PropertyType::p_attackMin;
	case PropertyType::m_attackMax:
		return PropertyType::m_attackMin;
	case PropertyType::witchMax:
		return PropertyType::witchMin;
	case PropertyType::p_defenceMax:
		return PropertyType::p_defenceMin;
	case PropertyType::m_defenceMax:
		return PropertyType::m_defenceMin;
	default:
		break;
	}

	LOG_ERROR("洗练, 获取最小属性类型失败, roleId={}, sceneItem={}, propType={}",
			  m_roleId, m_sceneItem, propType);

	return PropertyType::none;
}

void Wash::calcAttribute()
{
	Attribute::reset();
	
	std::vector<std::pair<PropertyType, uint32_t> > propVec;
	for(auto pos = m_curWashMap.begin(); pos != m_curWashMap.end(); ++pos)
	{
		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			for(auto item = iter->second.begin(); item != iter->second.end(); ++item)
			{
				propVec.push_back(std::make_pair(item->propType, item->prop));
			}
		}
	}

	Attribute::addAttribute(propVec);
	return;
}


}
