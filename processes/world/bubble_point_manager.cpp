#include "bubble_point_manager.h"
#include "bubble_point_config.h"
#include "role.h"
#include "world.h"
#include "channel.h"
#include "role_manager.h"
#include "scene_manager.h"
#include "roles_and_scenes.h"
#include "reward_manager.h"
#include "action_manager.h"
#include "mail_manager.h"

#include "protocol/rawmsg/public/bubble_point.h"
#include "protocol/rawmsg/public/bubble_point.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

BubblePointManager::BubblePointManager()
: m_notifyTime(EPOCH)
, m_actionEndTime(EPOCH)
, m_nextSendSpecialRewardTime(EPOCH)
, m_setActionBegin(false)
, m_setActionEnd(false)
{
}

BubblePointManager BubblePointManager::m_me;

BubblePointManager& BubblePointManager::me()
{
	return m_me;
}

void BubblePointManager::requestIntoScene(Role::Ptr role)
{
	const auto &cfg = BubblePointConfig::me().bubblePointCfg;
	if(0 == cfg.mapId || isActionMap(role->scene()))
		return;

	if(cfg.needLevel > role->level())
	{
		role->sendSysChat("等级不足");
		return;
	}

	if(!isActionBegin())
	{
		role->sendSysChat("活动未开启");
		return;
	}

	RolesAndScenes::me().gotoOtherScene(role->id(), cfg.mapId, Coord2D(0,0));
	return;
}


void BubblePointManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&BubblePointManager::timerLoop, this, StdInterval::sec_1, _1));
    World::me().regTimer(std::chrono::seconds(3), 
                         std::bind(&BubblePointManager::timerLoop, this, StdInterval::sec_3, _1));
}


void BubblePointManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
	switch(interval)
	{
		case StdInterval::sec_1:
			checkAndSetActionState(now);
			checkSendNotify(now);
			kickoutActionMap(now);
			break;
		case StdInterval::sec_3:
			giveOutExpReward();
			break;
		default:
			break;
	}
	return;
}

void BubblePointManager::checkAndSetActionState(const TimePoint& now)
{
	if(!m_setActionBegin && isActionBegin())
	{
		m_setActionBegin = true;
		m_setActionEnd = false;
		calcActionEndTimePoint(now);
		ActionManager::me().setActionState(ActionType::bubble_point, ActionState::begin);
		LOG_TRACE("活动, 激情泡点, 设置活动开启");
	}
	else if(m_setActionBegin && !m_setActionEnd  && !isActionBegin())
	{
		m_setActionEnd = true;
		m_setActionBegin = false;
		m_specialPointMap.clear();
		ActionManager::me().setActionState(ActionType::bubble_point, ActionState::end);
	
		m_nextSendSpecialRewardTime = EPOCH;
		LOG_TRACE("活动, 激情泡点, 设置活动结束");
	}

	{//设置下次发放特殊点奖励的时间, 并发放奖励	
		if(!m_setActionBegin)
			return;

		if(m_nextSendSpecialRewardTime == EPOCH || now >= m_nextSendSpecialRewardTime)
		{
			if(m_nextSendSpecialRewardTime != EPOCH)
			{
				giveOutSpecialReward(now);
			}
			const auto& cfg = BubblePointConfig::me().bubblePointCfg;
			m_nextSendSpecialRewardTime = now + std::chrono::seconds {cfg.span};
			sendGetSpecialRewardNeedSecToAll();
		}
	}

	return;
}

void BubblePointManager::checkSendNotify(const TimePoint& now)
{
	if(!isActionWorld())
		return;

	if(m_notifyTime == EPOCH)
	{
		if(!inActionNotifyTime(now))
			return;

		m_notifyTime = now;
		Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "5分钟后, 激情燃烧的时间到了, 速度进入副本, 占据资源点, 将会获得大量奖励");
	}
	else 
	{
		TimePoint nextNotifyTime = m_notifyTime + std::chrono::seconds {60 * 5};
		if(now >= nextNotifyTime && inActionNotifyTime(now))
		{
			m_notifyTime = now;
			Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "激情活动开了, 点击活动图标进入副本");
			return;
		}
	}

	if(m_notifyTime != EPOCH && !inActionNotifyTime(now))
		m_notifyTime = EPOCH;

	return;
}

bool BubblePointManager::inActionNotifyTime(const TimePoint& now) const
{
	std::string dateStr = componet::date();
	if(dateStr.empty())
		return false;

	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	for(auto iter = cfg.m_actionVec.begin(); iter != cfg.m_actionVec.end(); ++iter)
	{
		std::string notifyTime = dateStr + "-" + iter->notifyTime;
		std::string endTime = dateStr + "-" + iter->endTime;
		if(now >= componet::stringToTimePoint(notifyTime) 
		   && now < componet::stringToTimePoint(endTime))
			return true;
	}

	return false;
}

bool BubblePointManager::isActionBegin() const
{
	std::string dateStr = componet::date();
	if(dateStr.empty())
		return false;

	TimePoint now = Clock::now();
	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	for(auto iter = cfg.m_actionVec.begin(); iter != cfg.m_actionVec.end(); ++iter)
	{
		std::string beginTime = dateStr + "-" + iter->beginTime;
		std::string endTime = dateStr + "-" + iter->endTime;
		if(now >= componet::stringToTimePoint(beginTime) 
		   && now < componet::stringToTimePoint(endTime))
			return true;
	}

	return false;
}

void BubblePointManager::calcActionEndTimePoint(const TimePoint& now)
{
	std::string dateStr = componet::date();
	if(dateStr.empty())
		return;

	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	for(auto iter = cfg.m_actionVec.begin(); iter != cfg.m_actionVec.end(); ++iter)
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


bool BubblePointManager::isActionWorld()
{
	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	if(0 == cfg.mapId)
		return false;

	if(SceneManager::me().getById(cfg.mapId) == nullptr)
		return false;
	
	return true;
}

bool BubblePointManager::isActionMap(Scene::Ptr scene)
{
	if(scene == nullptr)
		return false;

	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	if(0 == cfg.mapId)
		return false;

	if(scene->mapId() != cfg.mapId)
		return false;

	return true;
}

void BubblePointManager::afterRoleEnterScene(Role::Ptr role)
{
	if(role == nullptr)
		return;

	if(!isActionMap(role->scene()))
		return;

	if(!isActionBegin())
	{
		role->sendSysChat("激情泡点活动未开启");
		role->exitCopyMap();
		return;
	}

	//注册人物移动事件
	auto regId = role->e_onPosChanged.reg(std::bind(&BubblePointManager::dealRoleChangePos, this, _1, _2, _3));
	m_roleChangePosEventRegId.insert(std::make_pair(role->id(), regId));	

	sendSpecialPointInfoToMe(role);
	sendGetSpecialRewardNeedSecToMe(role);
	return;
}

void BubblePointManager::beforeRoleLeaveScene(Role::Ptr role)
{
	if(role == nullptr)
		return;

	if(!isActionMap(role->scene()))
		return;

	//反注册人物移动事件
	auto eventIdIter = m_roleChangePosEventRegId.find(role->id());
	if(eventIdIter != m_roleChangePosEventRegId.end())
	{
		role->e_onPosChanged.unreg(eventIdIter->second);
		m_roleChangePosEventRegId.erase(eventIdIter);
	}

	for(auto pos = m_specialPointMap.begin(); pos != m_specialPointMap.end(); ++pos)
	{
		if(pos->second == nullptr)
			continue;

		if(pos->second->id() == role->id())
		{
			m_specialPointMap.erase(pos);
			sendSpecialPointInfoToAll();
			break;
		}
	}

	clearBuff(role);
	return;
}

void BubblePointManager::dealRoleChangePos(Role::Ptr role, Coord2D oldPos, Coord2D newPos)
{
	if(role == nullptr)
		return;

	if(!isActionMap(role->scene()))
		return;

	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	if(cfg.m_specialMap.empty())
		return;

	auto oldPosIter = cfg.m_specialMap.find(oldPos);
	if(oldPosIter != cfg.m_specialMap.end())
	{
		m_specialPointMap[oldPos] = nullptr;
		role->m_buffM.unshowBuff(oldPosIter->second.buffId);
		sendSpecialPointInfoToAll();
		return;
	}

	auto iter = cfg.m_specialMap.find(newPos);
	if(iter != cfg.m_specialMap.end())
	{
		m_specialPointMap[newPos] = role;
		role->m_buffM.showBuff(iter->second.buffId);
		sendSpecialPointInfoToAll();
	}

	return;
}

void BubblePointManager::roleBeKilled(Role::Ptr role)
{
	if(role == nullptr)
		return;

	if(!isActionMap(role->scene()))
		return;

	role->reliveByDefaultReliveArea(true);
	return;
}

void BubblePointManager::giveOutExpReward()
{
	if(!isActionWorld())
		return;

	if(!isActionBegin())
		return;

	if(m_roleChangePosEventRegId.empty())
		return;

	for(auto pos = m_roleChangePosEventRegId.begin(); pos != m_roleChangePosEventRegId.end(); ++pos)
	{
		Role::Ptr role = RoleManager::me().getById(pos->first);
		if(role == nullptr || role->isDead())
			continue;

		const uint32_t baseExp = getBaseRewardExp(role->level());
		if(0 == baseExp)
			continue;

		uint32_t totalExp = baseExp;
		if(role->inArea(AreaType::security))
		{
			totalExp = SAFE_DIV(baseExp, 2);
		}
		else if(role->inArea(AreaType::exp))
		{
			uint32_t percent = getExpPercent(role->pos());
			if(0 == percent)
				continue;

			totalExp = SAFE_DIV(baseExp * percent, 100); 
		}

		role->addExp(totalExp);
		role->sendMainToMe();
	}

	return;
}

void BubblePointManager::giveOutSpecialReward(const TimePoint& now)
{
	if(m_nextSendSpecialRewardTime == EPOCH)
		return;

	if(m_nextSendSpecialRewardTime > now)
		return;

	if(!isActionWorld())
		return;

	if(m_specialPointMap.empty())
		return;

	for(auto pos = m_specialPointMap.begin(); pos != m_specialPointMap.end(); ++pos)
	{
		Role::Ptr role = pos->second;
		if(role == nullptr)
			continue;
	
		uint32_t rewardId = getSpecialRewardId(pos->first);
		std::vector<ObjItem> objVec;
		if(!RewardManager::me().getRandomReward(rewardId, 1, role->level(), role->job(), objVec))
		{
			LOG_ERROR("激情泡点, 获取随机奖励失败, 发放特殊点奖励失败, name={}, roleId={}, level={}, job={}, rewardId={}",
					  role->name(), role->id(), role->level(), role->job(), rewardId);
			continue;
		}
	
		if(objVec.empty())
			continue;

		if(!role->checkPutObj(objVec))
		{
			std::string text = "由于背包空间不足, 通过邮件发放激情泡点特殊点奖励, 请注意查收";
			MailManager::me().send(role->id(), "激情泡点特殊点奖励", text, objVec);
			
		}
		else
		{
			role->putObj(objVec);
		}
		LOG_TRACE("激情泡点, 发放特殊点奖励成功, name={}, roleId={}, pos=[{},{}], rewardId={}, objVec={}",
				  role->name(), role->id(), pos->first.x, pos->first.y, 
				  rewardId, objVec.size());
	}

	return;
}

uint32_t BubblePointManager::getBaseRewardExp(uint32_t level)
{
	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	for(auto iter = cfg.m_noramlVec.begin(); iter != cfg.m_noramlVec.end(); ++iter)
	{
		if(level >= iter->minLevel && level <= iter->maxLevel)
			return iter->addExp;
	}

	return 0;
}

uint32_t BubblePointManager::getExpPercent(Coord2D pos)
{
	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	auto iter = cfg.m_specialMap.find(pos);
	if(iter == cfg.m_specialMap.end())
		return 0;

	return iter->second.percent;
}

uint32_t BubblePointManager::getSpecialRewardId(Coord2D pos)
{
	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	auto iter = cfg.m_specialMap.find(pos);
	if(iter == cfg.m_specialMap.end())
		return 0;

	return iter->second.rewardId;
}

std::string BubblePointManager::getPointName(Coord2D pos)
{
	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	auto iter = cfg.m_specialMap.find(pos);
	if(iter == cfg.m_specialMap.end())
		return "";

	return iter->second.name;
}

void BubblePointManager::sendSpecialPointInfoToAll()
{
	std::vector<uint8_t> buf;
	fillSpecialPointMsg(&buf);

	for(auto pos = m_roleChangePosEventRegId.begin(); pos != m_roleChangePosEventRegId.end(); ++pos)
	{
		Role::Ptr role = RoleManager::me().getById(pos->first);
		if(role == nullptr)
			continue;

		role->sendToMe(RAWMSG_CODE_PUBLIC(RetSpecialPointInfo), buf.data(), buf.size());
	}
	return;
}

void BubblePointManager::sendSpecialPointInfoToMe(Role::Ptr role)
{
	if(role == nullptr)
		return;

	std::vector<uint8_t> buf;
	fillSpecialPointMsg(&buf);

	role->sendToMe(RAWMSG_CODE_PUBLIC(RetSpecialPointInfo), buf.data(), buf.size());
	return;
}

void BubblePointManager::fillSpecialPointMsg(std::vector<uint8_t>* buf)
{
	buf->clear();
	buf->reserve(512);
	buf->resize(sizeof(PublicRaw::RetSpecialPointInfo));

	auto* msg = reinterpret_cast<PublicRaw::RetSpecialPointInfo*>(buf->data());
	msg->size = 0;

	for(auto pos = m_specialPointMap.begin(); pos != m_specialPointMap.end(); ++pos)
	{
		Role::Ptr role = pos->second;
		if(role == nullptr)
			continue;

		buf->resize(buf->size() + sizeof(PublicRaw::RetSpecialPointInfo::PointItem));
		auto* msg  = reinterpret_cast<PublicRaw::RetSpecialPointInfo*>(buf->data());

		std::string pointName = getPointName(pos->first);
		std::memset(msg->data[msg->size].pointName, 0, NAME_BUFF_SZIE);
		std::memset(msg->data[msg->size].roleName, 0, NAME_BUFF_SZIE);
		pointName.copy(msg->data[msg->size].pointName, NAME_BUFF_SZIE);
		role->name().copy(msg->data[msg->size].roleName, NAME_BUFF_SZIE);
		
		++msg->size;
	}

	return;
}

void BubblePointManager::sendGetSpecialRewardNeedSecToAll()
{
	if(m_nextSendSpecialRewardTime == EPOCH)
		return;

	for(auto pos = m_roleChangePosEventRegId.begin(); pos != m_roleChangePosEventRegId.end(); ++pos)
	{
		Role::Ptr role = RoleManager::me().getById(pos->first);
		if(role == nullptr)
			continue;

		sendGetSpecialRewardNeedSecToMe(role);	
	}

	return;
}

void BubblePointManager::sendGetSpecialRewardNeedSecToMe(Role::Ptr role)
{
	if(role == nullptr)
		return;

	if(m_nextSendSpecialRewardTime == EPOCH)
		return;

	TimePoint now = Clock::now();
	if(now > m_nextSendSpecialRewardTime)
		return;

	uint32_t needSec = std::chrono::duration_cast<std::chrono::seconds>(m_nextSendSpecialRewardTime - now).count();
		
	PublicRaw::RetGetSpecialRewardNeedSec send;
	send.sec = needSec;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetGetSpecialRewardNeedSec), &send, sizeof(send));
	return;
}

void BubblePointManager::kickoutActionMap(const TimePoint& now)
{
	if(m_actionEndTime == EPOCH)
		return;

	if(m_roleChangePosEventRegId.empty())
		return;

	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	TimePoint needGobackTime = m_actionEndTime + std::chrono::seconds {cfg.kickoutSec + 5};
	if(now < needGobackTime)
		return;

	for(auto pos = m_roleChangePosEventRegId.begin(); pos != m_roleChangePosEventRegId.end();)
	{
		Role::Ptr role = RoleManager::me().getById(pos->first);
		if(role == nullptr)
		{
			++pos;
			continue;
		}

		role->e_onPosChanged.unreg(pos->second);
		pos = m_roleChangePosEventRegId.erase(pos);

		clearBuff(role);
		role->exitCopyMap(); 
	}
	
	return;
}

uint32_t BubblePointManager::getSpanSecOfActionEnd() const
{
	if(m_actionEndTime == EPOCH)
		return 0;

	TimePoint now = Clock::now();
	if(now >= m_actionEndTime)
		return 0;

	return std::chrono::duration_cast<std::chrono::seconds>(m_actionEndTime - now).count(); 
}

void BubblePointManager::clearBuff(Role::Ptr role)
{
	if(role == nullptr)
		return;

	const auto& cfg = BubblePointConfig::me().bubblePointCfg;
	for(auto pos = cfg.m_specialMap.begin(); pos != cfg.m_specialMap.end(); ++pos)
	{
		role->m_buffM.unshowBuff(pos->second.buffId);
	}

	return;
}


}
