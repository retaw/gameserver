#include "zhuansheng.h"
#include "zhuansheng_config.h"
#include "pk.h"
#include "role.h"
#include "role_manager.h"
#include "hero.h"
#include "equip_package.h"

#include "water/common/commdef.h"

#include "protocol/rawmsg/public/zhuansheng.h"
#include "protocol/rawmsg/public/zhuansheng.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

Zhuansheng::Zhuansheng(SceneItemType sceneItem, RoleId roleId, PK& owner)
: m_sceneItem(sceneItem)
, m_roleId(roleId)
, m_owner(owner)
{
}

void Zhuansheng::requestZhuansheng()
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	const TurnLife curLevel = m_owner.turnLifeLevel();
	if(curLevel >= TurnLife::five)
		return;

	uint8_t nextLevel = static_cast<uint8_t>(curLevel) + 1;

	const auto& cfg = ZhuanshengConfig::me().zhuanshengCfg;
	auto pos = cfg.m_baseMap.find(nextLevel) ;
	if(pos == cfg.m_baseMap.end())
		return;

	if(!judgeZhuansheng())
		return;

	//扣货币
	if(!role->reduceMoney(pos->second.needMoneyType, pos->second.needMoneyNum, "转生"))
		return;
	
	m_owner.setTurnLifeLevel(static_cast<TurnLife>(nextLevel));

	m_owner.sendMainToMe();
	m_owner.syncScreenDataTo9();
	sendZhuanshengResult(OperateRetCode::sucessful);

	role->sendSysChat("转生成功");
	LOG_TRACE("转生, 转生成功! name={}, id={}, sceneItem={}, level={}->{}",
			  role->name(), role->id(), m_sceneItem, curLevel, nextLevel);
	return;
}

Role::Ptr Zhuansheng::getRole() const 
{
	Role::Ptr role = RoleManager::me().getById(m_roleId);
	if(role == nullptr)
		return nullptr;

	return role;
}

bool Zhuansheng::judgeZhuansheng()
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return false;

	TurnLife curLevel = m_owner.turnLifeLevel();
	uint8_t nextLevel = static_cast<uint8_t>(curLevel) + 1;
	
	const auto& cfg = ZhuanshengConfig::me().zhuanshengCfg;
	auto pos = cfg.m_baseMap.find(nextLevel);
	if(pos == cfg.m_baseMap.end())
		return false;

	if(m_sceneItem != SceneItemType::role && m_sceneItem != SceneItemType::hero)
		return false;

	if(m_sceneItem == SceneItemType::role)
	{
		for(auto iter = pos->second.limitVec.begin(); iter != pos->second.limitVec.end(); ++iter)
		{
			if(!judgeLimitRole(iter->first, iter->second))
				return false;
		}
	}
	else if(m_sceneItem == SceneItemType::hero)
	{
		for(auto iter = pos->second.limitVec.begin(); iter != pos->second.limitVec.end(); ++iter)
		{
			if(!judgeLimitHero(iter->first, iter->second))
				return false;
		}
	}

	return true;
}

bool Zhuansheng::judgeLimitRole(LimitType type, uint32_t needLevel)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return false;

	switch(type)
	{
	case LimitType::needLevel:
		return role->level() >= needLevel;
	case LimitType::strongLevel:
		return role->getMinStrongLevel() >= needLevel;
	case LimitType::stoneLevel:
		return role->getStoneTotalLevel() >= needLevel;
	case LimitType::horseLevel:
		return role->m_horse.star() >= needLevel;
	case LimitType::guanzhiLevel:
		return role->m_guanzhi.level() >= needLevel;
	case LimitType::lingliLevel:
		return role->m_wing.getLingliLevel() >= needLevel;
	default:
		break;
	}

	return false;
}

bool Zhuansheng::judgeLimitHero(LimitType type, uint32_t needLevel)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return false;

	Hero::Ptr hero = role->m_heroManager.getDefaultHero();
	if(hero == nullptr)
		return false;

	if(hero->id() != m_owner.id())
		return false;

	switch(type)
	{
	case LimitType::needLevel:
		return hero->level() >= needLevel;
	case LimitType::strongLevel:
		return hero->getMinStrongLevel() >= needLevel;
	case LimitType::stoneLevel:
		return hero->getStoneTotalLevel() >= needLevel;
	case LimitType::horseLevel:
		return role->m_horse.star() >= needLevel;
	case LimitType::guanzhiLevel:
		return role->m_guanzhi.level() >= needLevel;
	case LimitType::lingliLevel:
		return hero->m_wing.getLingliLevel() >= needLevel;
	default:
		break;
	}

	return false;
}

void Zhuansheng::calcAttribute()
{
	if(m_sceneItem == SceneItemType::role)
		calcAttributeOfRole();
	else if(m_sceneItem == SceneItemType::hero)
		calcAttributeOfHero();

	return;
}

void Zhuansheng::calcAttributeOfRole()
{
	Attribute::reset();
	
	const auto& cfg = ZhuanshengConfig::me().zhuanshengCfg;
	auto pos = cfg.m_roleRewardMap.find(m_owner.job());
	if(pos == cfg.m_roleRewardMap.end())
		return;

	uint8_t curLevel = static_cast<uint8_t>(m_owner.turnLifeLevel());
	auto iter = pos->second.find(curLevel);
	if(iter == pos->second.end())
		return;

	Attribute::addAttribute(iter->second.rewardPropVec);
	return;
}

void Zhuansheng::calcAttributeOfHero()
{
	Attribute::reset();
	
	const auto& cfg = ZhuanshengConfig::me().zhuanshengCfg;
	auto pos = cfg.m_heroRewardMap.find(m_owner.job());
	if(pos == cfg.m_heroRewardMap.end())
		return;

	uint8_t curLevel = static_cast<uint8_t>(m_owner.turnLifeLevel());
	auto iter = pos->second.find(curLevel);
	if(iter == pos->second.end())
		return;

	Attribute::addAttribute(iter->second.rewardPropVec);
	return;
}

void Zhuansheng::sendZhuanshengResult(OperateRetCode code)
{
	Role::Ptr role = getRole();
	if(role == nullptr)
		return;

	PublicRaw::RetZhuanshengResult send;
	send.sceneItem = m_sceneItem;
	send.code = code;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetZhuanshengResult), &send, sizeof(send));
	return;
}


}
