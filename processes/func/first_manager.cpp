#include "first_manager.h"
#include "first_config.h"
#include "role.h"
#include "func.h"
#include "channel.h"
#include "role_manager.h"
#include "first_table_structure.h"

#include "water/componet/logger.h"

#include "protocol/rawmsg/public/first.h"
#include "protocol/rawmsg/public/first.codedef.public.h"

#include "protocol/rawmsg/private/first.h"
#include "protocol/rawmsg/private/first.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace func{

using namespace std::placeholders;

FirstManager::FirstManager()
: m_notifyTimeApply(EPOCH)
, m_notifyTimeReady(EPOCH)
, m_winnerId(0)
, m_winnerName("")
, m_winnerJob(Job::none)
, m_winnerSex(Sex::none)
{
}

FirstManager FirstManager::m_me;

FirstManager& FirstManager::me()
{
	return m_me;
}

void FirstManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestFirstApplyInfo, std::bind(&FirstManager::clientmsg_RequestFirstApplyInfo, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestApplyFirst, std::bind(&FirstManager::clientmsg_RequestApplyFirst, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestIntoFistMap, std::bind(&FirstManager::clientmsg_RequestIntoFirstMap, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestFirstWinnerInfo, std::bind(&FirstManager::clientmsg_RequestFirstWinnerInfo, this, _1, _2, _3));

	REG_RAWMSG_PRIVATE(UpdateFirstPlayerInfo, std::bind(&FirstManager::servermsg_UpdateFirstPlayerInfo, this, _1, _2));
}

void FirstManager::clientmsg_RequestFirstApplyInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestFirstApplyInfo*>(msgData);
	if(!rev)
		return;
	
	sendApplyInfo(roleId);
	return;
}

void FirstManager::clientmsg_RequestApplyFirst(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestApplyFirst*>(msgData);
	if(!rev)
		return;

	const auto& cfg = FirstConfig::me().firstCfg;
	if(cfg.needLevel > role->level())
	{
		role->sendSysChat("等级不足, 无法报名");
		return;
	}

	if(m_applySet.find(roleId) != m_applySet.end())
	{
		role->sendSysChat("已报名");
		return;
	}

	if(!isActionTimeApply())
	{
		role->sendSysChat("未到报名时间");
		return;
	}

	m_applySet.insert(roleId);
	sendApplyInfo(roleId);
	role->sendSysChat("报名成功");
	return;
}

void FirstManager::clientmsg_RequestIntoFirstMap(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestIntoFistMap*>(msgData);
	if(!rev)
		return;

	if(m_applySet.find(roleId) == m_applySet.end())
	{
		role->sendSysChat("您没有报名, 不可参与天下第一活动");
		return;
	}

	if(!isActionTimeReady())
	{
		role->sendSysChat("未到准备时间");
		return;
	}

	if(0 != role->teamId())
	{
		role->sendSysChat("你当前处于组队状态, 不可进入天下第一活动");
		return;
	}

	const auto& cfg = FirstConfig::me().firstCfg;
	if(0 == cfg.mapId)
		return;

	role->gotoOtherScene(cfg.mapId, Coord2D(0,0));
	return;
}

void FirstManager::clientmsg_RequestFirstWinnerInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestFirstWinnerInfo*>(msgData);
	if(!rev)
		return;

	PublicRaw::RetFirstWinnerInfo send;
	send.job = m_winnerJob;
	send.sex = m_winnerSex;
	std::memset(send.name, 0, NAME_BUFF_SZIE);
	m_winnerName.copy(send.name, NAME_BUFF_SZIE);
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetFirstWinnerInfo), &send, sizeof(send));
	return;
}

void FirstManager::servermsg_UpdateFirstPlayerInfo(const uint8_t* msgData, uint32_t msgSize)
{
	auto rev = reinterpret_cast<const PrivateRaw::UpdateFirstPlayerInfo*>(msgData);
	if(!rev)
		return;

	m_winnerId = rev->roleId;
	m_winnerName = rev->name;
	m_winnerJob = rev->job;
	m_winnerSex = rev->sex;

	if(!deleteAllFromDB())
	{
		LOG_ERROR("天下第一, 删除历史数据失败, 存储新数据失败! name={}, id={}, job={}, sex={}",
				  m_winnerName, m_winnerId, m_winnerJob, m_winnerSex);
		return;
	}

	insertToDB();
	return;
}

void FirstManager::regTimer()
{
    using namespace std::placeholders;
	Func::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&FirstManager::timerLoop, this, StdInterval::sec_1, _1));
}


void FirstManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
	switch(interval)
	{
		case StdInterval::sec_1:
			checkSendNotifyApply(now);
			checkSendNotifyReady(now);
			clearApplySet();
			break;
		default:
			break;
	}
	return;
}

void FirstManager::checkSendNotifyApply(const TimePoint& now)
{
	if(m_notifyTimeApply == EPOCH)
	{
		if(!isActionTimeApply())
			return;

		m_notifyTimeApply = now;
		Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "天下第一活动报名已经开始,玩家可以点击活动图标进行报名");
	}
	else 
	{
		TimePoint nextNotifyTime = m_notifyTimeApply + std::chrono::seconds {60 * 60};
		if(now >= nextNotifyTime && isActionTimeApply())
		{
			m_notifyTimeApply = now;
			Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "天下第一活动报名已经开始,玩家可以点击活动图标进行报名");
			return;
		}
	}

	if(m_notifyTimeApply != EPOCH && !isActionTimeApply())
		m_notifyTimeApply = EPOCH;

	return;
}

void FirstManager::checkSendNotifyReady(const TimePoint& now)
{
	if(m_notifyTimeReady == EPOCH)
	{
		if(!isActionTimeReady())
			return;

		m_notifyTimeReady = now;
		Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "天下第一活动即将开启, 请玩家做好准备");
	}
	else 
	{
		TimePoint nextNotifyTime = m_notifyTimeReady + std::chrono::seconds {60};
		if(now >= nextNotifyTime && isActionTimeReady())
		{
			m_notifyTimeReady = now;
			Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "天下第一活动即将开启, 请玩家做好准备");
			return;
		}
	}

	if(m_notifyTimeReady != EPOCH && !isActionTimeReady())
		m_notifyTimeReady = EPOCH;

	return;
}

void FirstManager::clearApplySet()
{
	if(m_applySet.empty())
		return;

	if(isActionBegin())
		m_applySet.clear();

	return;
}

bool FirstManager::isActionTimeApply()
{
	TimePoint now = Clock::now();
	::tm detail;
	if(!componet::timePointToTM(now, &detail))
		return false;

	const auto& cfg = FirstConfig::me().firstCfg;
	auto pos = cfg.m_actionMap.find(detail.tm_wday);
	if(pos == cfg.m_actionMap.end())
		return false;

	std::string dateStr = componet::date();
	if(dateStr.empty())
		return false;

	std::string applyBeginTime = dateStr + "-" + pos->second.applyBeginTime;
	std::string applyEndTime = dateStr + "-" + pos->second.applyEndTime;
	if(now >= componet::stringToTimePoint(applyBeginTime) 
	   && now <= componet::stringToTimePoint(applyEndTime))
		return true;

	return false;
}

bool FirstManager::isActionTimeReady()
{
	TimePoint now = Clock::now();
	::tm detail;
	if(!componet::timePointToTM(now, &detail))
		return false;

	const auto& cfg = FirstConfig::me().firstCfg;
	auto pos = cfg.m_actionMap.find(detail.tm_wday);
	if(pos == cfg.m_actionMap.end())
		return false;

	std::string dateStr = componet::date();
	if(dateStr.empty())
		return false;

	std::string readyBeginTime = dateStr + "-" + pos->second.readyTime;
	std::string readyEndTime = dateStr + "-" + pos->second.beginTime;
	if(now >= componet::stringToTimePoint(readyBeginTime) 
	   && now < componet::stringToTimePoint(readyEndTime))
		return true;

	return false;
}

bool FirstManager::isActionBegin()
{
	TimePoint now = Clock::now();
	::tm detail;
	if(!componet::timePointToTM(now, &detail))
		return false;

	const auto& cfg = FirstConfig::me().firstCfg;
	auto pos = cfg.m_actionMap.find(detail.tm_wday);
	if(pos == cfg.m_actionMap.end())
		return false;

	std::string dateStr = componet::date();
	if(dateStr.empty())
		return false;

	std::string beginTime = dateStr + "-" + pos->second.beginTime;
	std::string endTime = dateStr + "-" + pos->second.endTime;
	if(now >= componet::stringToTimePoint(beginTime) 
	   && now <= componet::stringToTimePoint(endTime))
		return true;

	return false;
}

void FirstManager::sendApplyInfo(RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	bool flag = false;
	if(m_applySet.find(roleId) != m_applySet.end())
		flag = true;

	PublicRaw::RetFirstApplyInfo send;
	send.applied = flag;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetFirstApplyInfo), &send, sizeof(send));
	return;
}


bool FirstManager::loadFromDB()
{
	try
	{
        mysqlpp::Query query = water::dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query <<"select * from first";
        std::vector<RowOfFirst> rowOfFirst;
        query.storein(rowOfFirst);
        if(1 == rowOfFirst.size())
        {
			m_winnerId = rowOfFirst[0].roleId;
			m_winnerName = rowOfFirst[0].name;
			m_winnerJob = static_cast<Job>(rowOfFirst[0].job);
			m_winnerSex = static_cast<Sex>(rowOfFirst[0].sex);
			LOG_TRACE("天下第一, load, 加载数据成功! name={}, id={}, job={}, sex={}",
					  m_winnerName, m_winnerId, m_winnerJob, m_winnerSex);
		}
		return true;

	}
	catch(const mysqlpp::Exception& er)
	{
        LOG_ERROR("天下第一, load, 加载数据失败, DB error:{}", er.what());
        return false;
	}
}

bool FirstManager::insertToDB()
{
    try
    {
		mysqlpp::Query query = water::dbadaptcher::MysqlConnectionPool::me().getconn()->query();
		RowOfFirst rowOfFirst(m_winnerId, m_winnerName, static_cast<uint8_t>(m_winnerJob), static_cast<uint8_t>(m_winnerSex));	
		query.insert(rowOfFirst);
		query.execute();
        LOG_DEBUG("天下第一, insert, 插入新数据成功! name={}, id={}, job={}, sex={}",
				  m_winnerName, m_winnerId, m_winnerJob, m_winnerSex);
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_DEBUG("天下第一, insert, 插入新数据失败! name={}, id={}, job={}, sex={}",
				  m_winnerName, m_winnerId, m_winnerJob, m_winnerSex);
        return false;
    }
}

bool FirstManager::deleteAllFromDB()
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from first";
        query.execute();
        LOG_DEBUG("天下第一, delete, 删除历史数据成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("天下第一, delete, 删除历史数据失败, DB error:{}", er.what());
        return false;
    }
}


}
