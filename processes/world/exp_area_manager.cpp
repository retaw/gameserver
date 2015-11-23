#include "exp_area_manager.h"
#include "exp_area_config.h"
#include "role_manager.h"
#include "world.h"
#include "channel.h"
#include "action_manager.h"

#include "protocol/rawmsg/public/exp_area.h"
#include "protocol/rawmsg/public/exp_area.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

ExpAreaManager::ExpAreaManager()
: m_notifyTime(EPOCH)
, m_actionEndTime(EPOCH)
, m_setActionBegin(false)
, m_setActionEnd(false)
{
}

ExpAreaManager ExpAreaManager::m_me;

ExpAreaManager& ExpAreaManager::me()
{
	return m_me;
}


void ExpAreaManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestAutoAddExpList, std::bind(&ExpAreaManager::clientmsg_RequestAutoAddExpList, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestOpenAutoAddExp, std::bind(&ExpAreaManager::clientmsg_RequestOpenAutoAddExp, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestCloseAutoAddExp, std::bind(&ExpAreaManager::clientmsg_RequestCloseAutoAddExp, this, _1, _2, _3))
}

//请求泡点信息
void ExpAreaManager::clientmsg_RequestAutoAddExpList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestAutoAddExpList*>(msgData);
	if(!rev)
		return;

	role->m_expArea.sendAutoAddExpList();
	return;
}

//请求开启自动加经验
void ExpAreaManager::clientmsg_RequestOpenAutoAddExp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestOpenAutoAddExp*>(msgData);
	if(!rev)
		return;

	role->m_expArea.requestOpenAutoAddExp(rev->type);
	return;
}

//请求关闭自动加经验
void ExpAreaManager::clientmsg_RequestCloseAutoAddExp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestCloseAutoAddExp*>(msgData);
	if(!rev)
		return;

	role->m_expArea.requestCloseAutoAddExp();
	return;
}


void ExpAreaManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&ExpAreaManager::timerLoop, this, StdInterval::sec_1, _1));
}


void ExpAreaManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
	switch(interval)
	{
		case::StdInterval::sec_1:
			checkSendNotify(now);
			break;
		default:
			break;
	}
	return;
}

void ExpAreaManager::checkSendNotify(const TimePoint& now)
{
	if(m_notifyTime == EPOCH)
	{
		if(!inActionNotifyTime(now))
			return;

		m_notifyTime = now;
		Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "5分钟后, 双倍泡点活动开启, 参与活动将获得大量经验");
	}
	else
	{
		TimePoint nextNotifyTime = m_notifyTime + std::chrono::seconds {300};
		if(now >= nextNotifyTime && inActionNotifyTime(now))
		{
			m_notifyTime = now;
			Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "双倍泡点活动开启, 请前往泡点区域, 参与泡点活动");
			return;
		}
	}

	if(m_notifyTime != EPOCH && !inActionNotifyTime(now))
		m_notifyTime = EPOCH;

	return;
}

void ExpAreaManager::checkAndSetActionState(const TimePoint& now)
{
	if(!m_setActionBegin && isActionBegin(now))
	{
		m_setActionBegin = true;
		m_setActionEnd = false;
		calcActionEndTime(now);
		ActionManager::me().setActionState(ActionType::exp_area, ActionState::begin);
	}
	else if(m_setActionBegin && !m_setActionEnd  && !isActionBegin(now))
	{
		m_setActionEnd = true;
		m_setActionBegin = false;
		ActionManager::me().setActionState(ActionType::exp_area, ActionState::end);
	}

	return;
}

bool ExpAreaManager::inActionNotifyTime(const TimePoint& now) const
{
	std::string dateStr = componet::date();
	if(dateStr.empty())
		return false;

	const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
	for(auto iter = cfg.actionVec.begin(); iter != cfg.actionVec.end(); ++iter)
	{
		std::string notifyTime = dateStr + "-" + iter->notifyTime;
		std::string endTime = dateStr + "-" + iter->endTime;
		if(now >= componet::stringToTimePoint(notifyTime) 
		   && now < componet::stringToTimePoint(endTime))
			return true;
	}

	return false;
}

bool ExpAreaManager::isActionBegin(const TimePoint& now) const
{
	std::string dateStr = componet::date();
	if(dateStr.empty())
		return false;

	const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
	for(auto iter = cfg.actionVec.begin(); iter != cfg.actionVec.end(); ++iter)
	{
		std::string beginTime = dateStr + "-" + iter->beginTime;
		std::string endTime = dateStr + "-" + iter->endTime;
		if(now >= componet::stringToTimePoint(beginTime) 
		   && now < componet::stringToTimePoint(endTime))
			return true;
	}

	return false;
}

void ExpAreaManager::calcActionEndTime(const TimePoint& now)
{
	std::string dateStr = componet::date();
	if(dateStr.empty())
		return;

	const auto& cfg = ExpAreaConfig::me().m_expAreaCfg;
	for(auto iter = cfg.actionVec.begin(); iter != cfg.actionVec.end(); ++iter)
	{
		std::string beginTime = dateStr + "-" + iter->beginTime;
		std::string endTime = dateStr + "-" + iter->endTime;
		if(now >= componet::stringToTimePoint(beginTime) 
		   && now < componet::stringToTimePoint(endTime))
		{
			m_actionEndTime = componet::stringToTimePoint(endTime);
			break;
		}
	}

	return;
}

uint32_t ExpAreaManager::getSpanSecOfActionEnd() const
{
	if(m_actionEndTime == EPOCH)
		return 0;

	TimePoint now = Clock::now();
	if(now >= m_actionEndTime)
		return 0;

	return std::chrono::duration_cast<std::chrono::seconds>(m_actionEndTime - now).count();
}


}
