#include "guanzhi.h"
#include "guanzhi_config.h"
#include "role.h"
#include "mail_manager.h"
#include "reward_manager.h"

#include "water/common/commdef.h"

#include "protocol/rawmsg/public/guanzhi.h"
#include "protocol/rawmsg/public/guanzhi.codedef.public.h"

#include "protocol/rawmsg/private/guanzhi.h"
#include "protocol/rawmsg/private/guanzhi.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

Guanzhi::Guanzhi(Role& owner, uint8_t level)
: m_owner(owner)
, m_level(level)
{
	calcAttribute();
}

void Guanzhi::sendRewardState()
{
	PublicRaw::RetGuanzhiRewardState send;
	send.dailyReward = getDailyRewardState();
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetGuanzhiRewardState), &send, sizeof(send));
}

void Guanzhi::requestGetDailyReward()
{
	if(0 == level())
		return;

	Reward reward = getDailyRewardState();
	if(reward == Reward::none)
		m_owner.sendSysChat("每日俸禄不可领取");
	else if(reward == Reward::got)
		m_owner.sendSysChat("每日俸禄已领取");	

	if(reward != Reward::canGet)
		return;

	const auto& cfg = GuanzhiConfig::me().guanzhiCfg;
	auto pos = cfg.m_guanzhiMap.find(level());
	if(pos == cfg.m_guanzhiMap.end())
		return;

	//发每日奖励
	std::vector<ObjItem> objVec;
	if(!RewardManager::me().getFixReward(pos->second.dailyRewardId, 1, m_owner.level(), m_owner.job(), objVec))
	{
		LOG_ERROR("官职, 获取固定奖励失败, 领取官职每日俸禄失败, name={}, roleId={}, rewardId={}, level={}, job={}",
				  m_owner.name(), m_owner.id(), pos->second.dailyRewardId, 
				  m_owner.level(), m_owner.job());
		return;
	}
	
	if(!m_owner.checkPutObj(objVec))
	{
		m_owner.sendSysChat("背包空间不足");
		return;
	}
	m_owner.putObj(objVec);
	
	setDailyRewardState(Reward::got);
	m_owner.sendSysChat("领取每日俸禄成功");
	return;
}

void Guanzhi::requestLevelUp()
{
	const uint8_t curLevel = level();
	const uint8_t nextLevel = curLevel + 1;
	if(curLevel >= MAX_GUANZHI_LEVEL)
		return;
	
	const auto& cfg = GuanzhiConfig::me().guanzhiCfg;
	auto pos = cfg.m_guanzhiMap.find(nextLevel);
	if(pos == cfg.m_guanzhiMap.end())
		return;

	if(pos->second.needLevel > m_owner.level())
	{
		m_owner.sendSysChat("等级不足");
		return;
	}

	uint64_t needZhangong = pos->second.needZhangong;
	uint32_t needGold = pos->second.needGold;

	if(!m_owner.checkMoney(MoneyType::money_7, needZhangong))
		return;

	if(!m_owner.checkMoney(MoneyType::money_2, needGold))
		return;

	if(!m_owner.reduceMoney(MoneyType::money_7, needZhangong, "官职晋升"))
		return;

	if(!m_owner.reduceMoney(MoneyType::money_2, needGold, "官职晋升"))
		return;

	setLevel(nextLevel);
	if(!updateGuanzhiLevelToDB())
		return;

	m_owner.addTitle(pos->second.titleId);
	m_owner.sendMainToMe();

	if(nextLevel >= 4)
	{
		m_owner.sendSysChat(ChannelType::screen_middle, "恭喜{}历经千辛万苦终于晋升为{}",
							m_owner.name(), pos->second.name);
	}
	else
	{
		m_owner.sendSysChat("恭喜你成功晋升为{}", pos->second.name);
	}

	//官职等级 0->1, 奖励状态改为可领取
	if(1 == level())
	{
		setDailyRewardState(Reward::canGet);
	}

	//发放晋升奖励
	std::vector<ObjItem> objVec;
	if(!RewardManager::me().getFixReward(pos->second.levelUpRewardId, 1, m_owner.level(), m_owner.job(), objVec))
	{
		LOG_ERROR("官职, 获取固定奖励失败, 发放官职晋升奖励失败, name={}, roleId={}, rewardId={}, level={}, job={}",
				  m_owner.name(), m_owner.id(), pos->second.levelUpRewardId, 
				  m_owner.level(), m_owner.job());
		return;
	}

	if(objVec.empty())
		return;

	if(!m_owner.checkPutObj(objVec))
	{
		std::string text = "由于背包空间不足, 通过邮件发放官职晋升奖励, 请注意查收";
		MailManager::me().send(m_owner.id(), "官职晋升奖励", text, objVec);
		return;
	}
	m_owner.putObj(objVec);
	return;
}

void Guanzhi::dealNotSameDay()
{
	if(0 == level())
		return;

	setDailyRewardState(Reward::canGet);
}

void Guanzhi::setDailyRewardState(Reward reward)
{
	if(reward == Reward::canGet && 0 == level())
	{
		reward = Reward::none;
	}

	m_owner.m_roleCounter.set(CounterType::guanzhiReward, static_cast<uint32_t>(reward));
	sendRewardState();
	return;
}

Reward Guanzhi::getDailyRewardState()
{
	uint32_t count = m_owner.m_roleCounter.get(CounterType::guanzhiReward);
	if((uint32_t)-1 == count)
		return Reward::none;

	return static_cast<Reward>(count);
}

bool Guanzhi::updateGuanzhiLevelToDB()
{
	PrivateRaw::UpdateGuanzhiLevelToDB send;
	send.roleId = m_owner.id();
	send.level = level();
	
	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateGuanzhiLevelToDB), &send, sizeof(send));
	LOG_TRACE("官职, 更新官职等级 to {}, {}, name={}, roleId={}, guanzhiLevel={}",
			  dbcachedId, ret ? "ok" : "falied",
			  m_owner.name(), send.roleId, send.level);

	return ret;
}

uint8_t Guanzhi::level() const
{
	return m_level;
}


void Guanzhi::setLevel(uint8_t level)
{
	m_level = level;

	calcAttribute();
}

void Guanzhi::calcAttribute()
{
	if(0 == level())
		return;

	const auto& cfg = GuanzhiConfig::me().guanzhiCfg;
	auto pos = cfg.m_guanzhiMap.find(level());
	if(pos == cfg.m_guanzhiMap.end())
		return;

	if(pos->second.rewardPropVec.empty())
		return;

	setAttribute(pos->second.rewardPropVec);
	return;
}

void Guanzhi::sendLevelUpSucess()
{
	PublicRaw::RetGuanzhiLevelUpSucess send;
	send.sucess = true;
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetGuanzhiLevelUpSucess), &send, sizeof(send));
	
	return;
}


}
