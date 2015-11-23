#include "exp_area.h"
#include "exp_area_config.h"
#include "exp_area_manager.h"
#include "role.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/exp_area.h"
#include "protocol/rawmsg/public/exp_area.codedef.public.h"

#include "protocol/rawmsg/private/exp_area.h"
#include "protocol/rawmsg/private/exp_area.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

ExpArea::ExpArea(Role& owner)
: m_owner(owner)
, m_openExpType(ExpSecType::none)
, m_openExpTime(EPOCH)
, bRefreshSceneData(false)
{
}

void ExpArea::loadFromDB(const std::vector<std::pair<uint8_t, uint32_t> >& expSecVec)
{
	for(auto iter = expSecVec.begin(); iter != expSecVec.end(); ++iter)
	{
		m_autoExpMap.insert(std::make_pair(iter->first, iter->second));
	}

	return;
}

void ExpArea::sendAutoAddExpList()
{
	std::vector<uint8_t> buf;
	buf.reserve(32);
	buf.resize(sizeof(PublicRaw::RetAutoAddExpList));

	auto* msg = reinterpret_cast<PublicRaw::RetAutoAddExpList*>(buf.data());
	msg->size = 0;

	for(auto pos = m_autoExpMap.begin(); pos != m_autoExpMap.end(); ++pos)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetAutoAddExpList::AutoExpItem));
		auto* msg  = reinterpret_cast<PublicRaw::RetAutoAddExpList*>(buf.data());

		msg->data[msg->size].type = pos->first;
		msg->data[msg->size].sec = pos->second;

		++msg->size;
	}

	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetAutoAddExpList), buf.data(), buf.size());		
	return;
}

void ExpArea::requestOpenAutoAddExp(uint8_t type)
{
	if(!m_owner.inArea(AreaType::exp))
	{
		m_owner.sendSysChat("不足泡点区, 无法开启泡点");
		return;
	}

	if(!checkLevel())
	{
		m_owner.sendSysChat("等级不足, 无法开启泡点");
		return;
	}
	
	if(m_owner.isDead())
		return;

	auto pos = m_autoExpMap.find(type);
	if(pos == m_autoExpMap.end())
		return;

	if(0 == pos->second)
		return;

	m_openExpType = static_cast<ExpSecType>(type);	
	sendOpenAutoAddExpSucess(type);
	m_owner.sendSysChat("泡点开启");
	return;
}

void ExpArea::requestCloseAutoAddExp()
{
	breakAutoAddExp();
	m_owner.sendSysChat("泡点关闭");
	return;
}

void ExpArea::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
	switch(interval)
	{
	case StdInterval::sec_1:
		autoAddExp(now);
		break;
	default:
		break;
	}

	return;
}

void ExpArea::dealNotSameDay()
{
	m_owner.m_roleCounter.clear(CounterType::autoAddExpSec);
}

void ExpArea::autoAddExp(const componet::TimePoint& now)
{
	if(m_openExpType == ExpSecType::none)
		return;

	if(!m_owner.inArea(AreaType::exp))
	{
		breakAutoAddExp();
		sendBreakAutoAddExp();
		m_owner.sendSysChat("离开泡点区域");
		return;	
	}

	if(m_owner.isDead())
	{
		breakAutoAddExp();
		sendBreakAutoAddExp();
		return;	
	}

	auto pos = m_autoExpMap.find(static_cast<uint8_t>(m_openExpType));
	if(pos == m_autoExpMap.end() || 0 == pos->second)
	{
		breakAutoAddExp();
		sendBreakAutoAddExp();
		m_owner.sendSysChat("泡点时间已用完");
		return;
	}

	//判断时间间隔并发放经验奖励
	if(m_openExpTime == EPOCH)
	{
		m_openExpTime = now;
	}
	else if(0 == SAFE_MOD(SAFE_SUB(toUnixTime(now), toUnixTime(m_openExpTime)), 3))
	{
		uint32_t baseRewardExp = getBaseRewardExp();
		uint32_t basePercent = getBasePercent();
		uint32_t vipPercent = getVipPercent();	
		if((uint32_t)-1 == baseRewardExp 
		   || (uint32_t)-1 == basePercent 
		   || (uint32_t)-1 == vipPercent)
			return;

		uint32_t totalPercent =  basePercent + vipPercent;
		uint64_t totalRewardExp = baseRewardExp + SAFE_DIV(baseRewardExp * totalPercent, 100);
		if(ExpAreaManager::me().isActionBegin(now))
		{
			const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
			totalRewardExp = totalRewardExp * cfg.multiple;
		}

		m_owner.addExp(totalRewardExp);
		sendAutoAddExpResult(totalRewardExp);
	}

	//剩余时间扣1秒
	pos->second -= 1;
	sendSecToDB(pos->first, pos->second);	
	
	if(!bRefreshSceneData)
	{
		bRefreshSceneData = true;
		m_owner.syncScreenDataTo9();
	}
	return;
}

void ExpArea::breakAutoAddExp()
{
	m_openExpType = ExpSecType::none;
	m_openExpTime = EPOCH;
	bRefreshSceneData = false;
	m_owner.syncScreenDataTo9();
	return;
}

bool ExpArea::checkLevel()
{
	uint32_t level = m_owner.level();
	const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
	for(auto iter = cfg.rewardVec.begin(); iter != cfg.rewardVec.end(); ++iter)
	{
		if(level >= iter->minLevel && level <= iter->maxLevel)
			return true;
	}

	return false;
}

uint32_t ExpArea::getBaseRewardExp()
{
	uint32_t level = m_owner.level();
	const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
	for(auto iter = cfg.rewardVec.begin(); iter != cfg.rewardVec.end(); ++iter)
	{
		if(level >= iter->minLevel && level <= iter->maxLevel)
			return iter->addExp;
	}

	return (uint32_t)-1;
}

uint32_t ExpArea::getBasePercent()
{
	const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
	auto pos = cfg.bubbleMap.find(static_cast<uint8_t>(m_openExpType));
	if(pos == cfg.bubbleMap.end())
		return (uint32_t)-1;

	return pos->second;
}

uint32_t ExpArea::getVipPercent()
{
	const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
	auto pos = cfg.vipMap.find(m_owner.vipLevel());
	if(pos == cfg.vipMap.end())
		return (uint32_t)-1;

	return pos->second.percent;
}

void ExpArea::sendOpenAutoAddExpSucess(uint8_t type)
{
	PublicRaw::RetOpenAutoAddExpSucess send;
	send.type = type;
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetOpenAutoAddExpSucess), &send, sizeof(send));
}

void ExpArea::sendAutoAddExpResult(uint64_t exp)
{
	PublicRaw::RetAutoAddExp send;
	send.exp = exp;
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetAutoAddExp), &send, sizeof(send));
}

void ExpArea::sendBreakAutoAddExp()
{
	PublicRaw::RetServerBreakAutoAddExp send;
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetServerBreakAutoAddExp), &send, sizeof(send));
}

bool ExpArea::sendSecToDB(uint8_t type, uint32_t sec)
{
	PrivateRaw::UpdateOrInsertExpSec send;
	send.roleId = m_owner.id();
	send.type = type;
	send.sec = sec;

	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateOrInsertExpSec), &send, sizeof(send));
	return ret;
}

void ExpArea::addSec(ExpSecType type, uint32_t sec)
{
	if(type != ExpSecType::one && type != ExpSecType::two 
	   && type != ExpSecType::three && type != ExpSecType::four)
		return;

	m_autoExpMap[static_cast<uint8_t>(type)] += sec;
	sendSecToDB(static_cast<uint8_t>(type), m_autoExpMap[static_cast<uint8_t>(type)]);
	sendAutoAddExpList();
	m_owner.sendSysChat(ChannelType::screen_right_down, "获得 泡点时间: {}", sec);
	return;
}

uint32_t ExpArea::getSecByType(ExpSecType type) const
{
	auto pos = m_autoExpMap.find(static_cast<uint8_t>(type));
	if(pos == m_autoExpMap.end())
		return 0;

	return pos->second;
}

uint32_t ExpArea::getLimitCount() const
{
	const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
	auto pos = cfg.vipMap.find(m_owner.vipLevel());
	if(pos == cfg.vipMap.end())
		return (uint32_t)-1;

	return pos->second.limitCount;
}

bool ExpArea::isAutoAddExp() const
{
	if(m_openExpTime != EPOCH)
		return true;

	return false;
}


}
