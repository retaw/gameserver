#include "dragon_ball.h"
#include "dragon_ball_config.h"
#include "pk.h"
#include "role.h"
#include "role_manager.h"

#include "water/common/commdef.h"

#include "protocol/rawmsg/public/dragon_ball.h"
#include "protocol/rawmsg/public/dragon_ball.codedef.public.h"

#include "protocol/rawmsg/private/dragon_ball.h"
#include "protocol/rawmsg/private/dragon_ball.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

DragonBall::DragonBall(Role& owner)
: m_owner(owner)
{
}

void DragonBall::loadFromDB(const std::vector<DragonBallInfo>& dragonVec)
{
	for(auto iter = dragonVec.begin(); iter != dragonVec.end(); ++iter)
	{
		m_dragonMap.insert(std::make_pair(iter->type, iter->exp));
	}

	calcAttribute();
}

void DragonBall::sendDragonBallInfo()
{
	std::vector<uint8_t> buf;
	buf.reserve(128);
	buf.resize(sizeof(PublicRaw::RetDragonBallInfo));

	auto* msg = reinterpret_cast<PublicRaw::RetDragonBallInfo*>(buf.data());
	msg->size = 0;

	for(auto pos = m_dragonMap.begin(); pos != m_dragonMap.end(); ++pos)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetDragonBallInfo::DragonInfo));
		auto* msg = reinterpret_cast<PublicRaw::RetDragonBallInfo*>(buf.data());

		const uint8_t type = pos->first;
		msg->data[msg->size].type = type;
		msg->data[msg->size].level = getDragonBallLevel(type);
		msg->data[msg->size].exp = pos->second;

		++msg->size;
	}

	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetDragonBallInfo), buf.data(), buf.size());
	return;
}

void DragonBall::requestLevelUpDragonBall(uint8_t type)
{
	if(!checkLevel(type))
		return;

	if(!reduceMaterial(type))
		return;

	const auto& cfg = DragonBallConfig::me().dragonCfg;
	auto iter = cfg.m_dragonBallMap.find(type);
	if(iter == cfg.m_dragonBallMap.end())
		return;

	const uint8_t oldLevel = getDragonBallLevel(type); 
	if((uint8_t)-1 == oldLevel)
		return;

	auto pos = iter->second.find(oldLevel);
	if(pos == iter->second.end())
		return;

	//更新经验
	uint32_t oldExp = 0;
	if(m_dragonMap.find(type) != m_dragonMap.end())
	{
		oldExp = m_dragonMap[type];
	}
	const uint32_t newExp = oldExp + pos->second.rewardExp;
	m_dragonMap[type] = newExp;

	const uint8_t newLevel = getDragonBallLevel(type);
	if(newLevel > oldLevel)
	{
		m_owner.sendMainToMe();
	}

	if(!sendDragonBallExpToDB(type, newExp))
		return;
	
	calcAttribute();
	sendDragonBallLevelUpResult(type, oldLevel, newLevel, newExp);

	//发送公告
	if(!pos->second.bNotify)
		return;

	m_owner.sendSysChat(ChannelType::global, "恭喜玩家{}将{}提升到{}级, 获得大量奖励",
						m_owner.name(), pos->second.name, newLevel);
	return;
}

bool DragonBall::checkLevel(uint8_t type)
{
	const auto& cfg = DragonBallConfig::me().dragonCfg;
	auto iter = cfg.m_dragonBallMap.find(type);
	if(iter == cfg.m_dragonBallMap.end())
		return false;

	const uint8_t level = getDragonBallLevel(type);
	if((uint8_t)-1 == level)
		return false;

	auto pos = iter->second.find(level);
	if(pos == iter->second.end())
		return false;

	if(pos->second.needLevel > m_owner.level())
	{
		m_owner.sendSysChat("等级不足");
		return false;
	}

	if(pos->second.needTurnLifeLevel > m_owner.turnLifeLevel())
	{
		m_owner.sendSysChat("转生等级不足");
		return false;
	}

	return true;
}

bool DragonBall::reduceMaterial(uint8_t type)
{
	const auto& cfg = DragonBallConfig::me().dragonCfg;
	auto iter = cfg.m_dragonBallMap.find(type);
	if(iter == cfg.m_dragonBallMap.end())
		return false;

	const uint8_t level = getDragonBallLevel(type);
	if((uint8_t)-1 == level)
		return false;

	auto pos = iter->second.find(level);
	if(pos == iter->second.end())
		return false;

	//验证金币
	const MoneyType needMoneyType = pos->second.needMoneyType;
	const uint32_t needMoneyNum = pos->second.needMoneyNum;
	if(!m_owner.checkMoney(needMoneyType, needMoneyNum))
		return false;

	//验证材料
	const uint32_t needTplId = pos->second.needTplId;
	const uint32_t needTplNum = pos->second.needTplNum;
	const uint16_t objNum = m_owner.m_packageSet.getObjNum(needTplId, PackageType::role);
	if(needTplNum > objNum)
	{
		m_owner.sendSysChat("材料不足");
		return false;
	}

	//扣材料
	if(!m_owner.m_packageSet.eraseObj(needTplId, needTplNum, PackageType::role, "龙珠升级"))
		return false;

	//扣金币
	if(!m_owner.reduceMoney(needMoneyType, needMoneyNum, "龙珠升级"))
		return false;

	return true;
}

uint8_t DragonBall::getDragonBallLevel(uint8_t type) const
{
	auto pos = m_dragonMap.find(type);
	if(pos == m_dragonMap.end())
		return 0;
	
	const uint32_t exp = pos->second;
	uint8_t level = 0;

	const auto& cfg = DragonBallConfig::me().dragonCfg;
	auto itemType = cfg.m_dragonBallMap.find(type);
	if(itemType == cfg.m_dragonBallMap.end())
		return (uint8_t)-1;

	for(auto pos = itemType->second.begin(); pos != itemType->second.end(); ++pos)
	{
		if(level >= itemType->second.size() - 1)
			return level;

		if(pos->second.nextLevelNeedExp > exp)
			break;

		level = pos->first + 1;
	}
	
	return level;
}

bool DragonBall::sendDragonBallExpToDB(uint8_t type, uint32_t exp)
{
	PrivateRaw::UpdateOrInsertDragonBallExp send;
	send.roleId = m_owner.id();
	send.dragonType = type;
	send.exp = exp;

	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateOrInsertDragonBallExp), &send, sizeof(send));
	return ret;
}

void DragonBall::sendDragonBallLevelUpResult(uint8_t type, uint8_t oldLevel, uint8_t newLevel, uint32_t exp)
{
	PublicRaw::RetDragonBallLevelUpResult send;
	send.type = type;
	send.oldLevel = oldLevel;
	send.newLevel = newLevel;
	send.exp = exp;
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetDragonBallLevelUpResult), &send, sizeof(send));
	return;
}

void DragonBall::calcAttribute()
{
	Attribute::reset();
	for(auto pos = m_dragonMap.begin(); pos != m_dragonMap.end(); ++pos)
	{
		const uint8_t type = pos->first;
		const uint8_t level = getDragonBallLevel(type);
		if((uint8_t)-1 == level)
			continue;

		const auto& cfg = DragonBallConfig::me().dragonCfg;
		auto iter = cfg.m_dragonBallMap.find(type);
		if(iter == cfg.m_dragonBallMap.end())
			continue;

		auto item = iter->second.find(level);
		if(item == iter->second.end())
			continue;

		Attribute::addAttribute(item->second.rewardPropVec);
	}

	return;
}

}
