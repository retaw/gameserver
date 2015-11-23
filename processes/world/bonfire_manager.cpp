#include "bonfire_manager.h"
#include "bonfire_config.h"
#include "role_manager.h"
#include "world.h"
#include "channel.h"
#include "scene.h"
#include "trigger.h"
#include "trigger_cfg.h"
#include "reward_manager.h"
#include "mail_manager.h"

#include "protocol/rawmsg/public/bonfire.h"
#include "protocol/rawmsg/public/bonfire.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

BonfireManager::BonfireManager()
{
}

BonfireManager BonfireManager::m_me;

BonfireManager& BonfireManager::me()
{
	return m_me;
}


void BonfireManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestBonfireTeamInfo, std::bind(&BonfireManager::clientmsg_RequestBonfireTeamInfo, this, _1, _2, _3));
	
	REG_RAWMSG_PUBLIC(RequestBonfireOwnerName, std::bind(&BonfireManager::clientmsg_RequestBonfireOwnerName, this, _1, _2, _3));
	
	REG_RAWMSG_PUBLIC(RequestJoinBonfire, std::bind(&BonfireManager::clientmsg_RequestJoinBonfire, this, _1, _2, _3));

}

//请求篝火队伍信息
void BonfireManager::clientmsg_RequestBonfireTeamInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestBonfireTeamInfo*>(msgData);
	if(!rev)
		return;

	sendTeamInfo(roleId, rev->triggerId);
	return;
}

//请求篝火主人名字
void BonfireManager::clientmsg_RequestBonfireOwnerName(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestBonfireOwnerName*>(msgData);
	if(!rev)
		return;

	TriggerId triggerId = rev->triggerId;
	auto pos = m_bonfireMap.find(triggerId);
	if(pos == m_bonfireMap.end())
		return;

	std::string ownerName = getOwnerName(triggerId);
	
	PublicRaw::RetBonfireOwnerName send;
	send.triggerId = triggerId;
	std::memset(send.ownerName, 0, NAME_BUFF_SZIE);
	ownerName.copy(send.ownerName, NAME_BUFF_SZIE);
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetBonfireOwnerName), &send, sizeof(send));
}

//请求加入篝火
void BonfireManager::clientmsg_RequestJoinBonfire(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestJoinBonfire*>(msgData);
	if(!rev)
		return;

	Trigger::Ptr trigger = TriggerManager::me().getById(rev->triggerId);
	if(trigger == nullptr)
		return;

	const auto& cfg = BonfireConfig::me().bonfireCfg;
	auto iter = cfg.m_levelMap.find(trigger->triggerTplId());
	if(iter == cfg.m_levelMap.end())
		return;

	if(iter->second > role->level())
	{
		role->sendSysChat("等级不足, 需要{}级", iter->second);
		return;
	}

	auto pos = m_bonfireMap.find(rev->triggerId);
	if(pos == m_bonfireMap.end())
		return;

	if(pos->second.size() >= 5)
	{
		role->sendSysChat("烤火队列已满");
		return;
	}

	if(m_drinkMap.find(roleId) != m_drinkMap.end())
	{
		role->sendSysChat("已在篝火队列");
		return;
	}

	TeamItem temp;
	temp.roleId = role->id();
	temp.job = role->job();
	temp.sex = role->sex();
	temp.name = role->name();
	temp.isOwner = false;

	pos->second.push_back(temp);
	m_drinkMap.insert(std::make_pair(roleId, WineType::none));

	sendTeamInfo(roleId, rev->triggerId);
	role->sendSysChat("加入篝火队列成功");
	return;
}


bool BonfireManager::summonBonfire(RoleId roleId, BonfireType type, TplId tplId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return false;

	const auto& cfg = BonfireConfig::me().bonfireCfg;
	auto pos = cfg.m_levelMap.find(tplId);
	if(pos == cfg.m_levelMap.end())
		return false;

	if(pos->second > role->level())
	{
		role->sendSysChat("等级不足, 需要{}级", pos->second);
		return false;
	}

	if(m_drinkMap.find(roleId) != m_drinkMap.end())
	{
		role->sendSysChat("不可引燃多个篝火");
		return false;
	}

	Scene::Ptr s = role->scene();
	if(s == nullptr)
		return false;

	Trigger::Ptr trigger = s->summonTrigger(tplId, role->pos(), 3);
	if(trigger == nullptr)
		return false;

	if(type == BonfireType::high)
	{
		role->sendSysChat("{} 玩家在 {} 引燃了 神木篝火, 大家速度前去烤火啊",
						  role->name(), trigger->pos());
	}

	TeamItem temp;
	temp.roleId = roleId;
	temp.job = role->job();
	temp.sex = role->sex();
	temp.name = role->name();
	temp.isOwner = true;
	m_bonfireMap[trigger->id()].push_back(temp);
	m_drinkMap.insert(std::make_pair(roleId, WineType::none));

	return true;
}

bool BonfireManager::drinkWine(RoleId roleId, WineType type)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return false;

	const auto& cfg = BonfireConfig::me().bonfireCfg;
	auto iter = cfg.m_drinkMap.find(static_cast<uint8_t>(type));
	if(iter == cfg.m_drinkMap.end())
		return false;

	auto pos = m_drinkMap.find(roleId);
	if(pos == m_drinkMap.end())
		return false;

	if(m_drinkMap.find(roleId) == m_drinkMap.end())
		return false;

	m_drinkMap[roleId] = type;
	role->sendSysChat("已使用{}, 篝火经验提升{}%", iter->second.name, iter->second.percent);
	
	PublicRaw::RetDrinkWineResult send;
	send.wine = static_cast<uint8_t>(type);
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetDrinkWineResult), &send, sizeof(send));
	return true;
}

void BonfireManager::bonfireLeaveScene(TriggerId triggerId)
{
	auto pos = m_bonfireMap.find(triggerId);
	if(pos == m_bonfireMap.end())
		return;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		//删除m_drinkMap中对应玩家
		m_drinkMap.erase(iter->roleId);

		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		PublicRaw::NotifyBonfireEnd send;
		send.end = true;
		role->sendToMe(RAWMSG_CODE_PUBLIC(NotifyBonfireEnd), &send, sizeof(send));
	}

	m_bonfireMap.erase(pos);
}

uint32_t BonfireManager::getLimitCount(RoleId roleId) const
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return 0;

	const auto& cfg = BonfireConfig::me().bonfireCfg;
	auto pos = cfg.m_vipMap.find(role->vipLevel());
	if(pos == cfg.m_vipMap.end())
		return 0;

	return pos->second;
}


void BonfireManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(5), 
                         std::bind(&BonfireManager::timerLoop, this, StdInterval::sec_5, _1));
}

void BonfireManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
	switch(interval)
	{
		case::StdInterval::sec_5:
			giveoutReward();
			break;
		default:
			break;
	}
	return;
}

void BonfireManager::giveoutReward()
{
	const auto& cfg = BonfireConfig::me().bonfireCfg;
	for(auto pos = m_bonfireMap.begin(); pos != m_bonfireMap.end(); ++pos)
	{
		Trigger::Ptr trigger = TriggerManager::me().getById(pos->first);
		if(trigger == nullptr)
			continue;
	
		uint32_t teamPercent = getTeamPercent(pos->second.size());
		if((uint32_t)- 1 == teamPercent)
			return;

		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			Role::Ptr role = RoleManager::me().getById(iter->roleId);
			if(role == nullptr)
				continue;

			if(role->isDead())
				continue;

			if(!checkInBonfireRange(pos->first, role->pos()))
				continue;

			uint32_t baseRewardExp = getBaseRewardExp(pos->first, role->level());
			uint32_t drinkPercent = getDrinkPercent(role->id());
			if((uint32_t)-1 == baseRewardExp || (uint32_t)-1 == drinkPercent)
				return;
		
			uint32_t totalPercent = teamPercent + drinkPercent;
			uint64_t totalRewardExp = baseRewardExp + SAFE_DIV(baseRewardExp * totalPercent, 100);
			role->addExp(totalRewardExp);
		
			//发放喝酒奖励
			if(getDrinkWineType(role->id()) == WineType::none)
				continue;

			std::vector<ObjItem> objVec;
			if(!RewardManager::me().getRandomReward(cfg.drinkRewardId, 1, role->level(), role->job(), objVec))
			{
				LOG_ERROR("日常任务, 获取随机奖励失败, 发放惊喜奖励失败, name={}, roleId={}, level={}, job={}, rewardId={}",
						  role->name(), role->id(), role->level(), 
						  role->job(), cfg.drinkRewardId);
				continue;
			}

			if(objVec.empty())
				continue;

			if(!role->checkPutObj(objVec))
			{
				std::string text = "由于背包空间不足, 通过邮件发放篝火喝酒奖励, 请注意查收";
				MailManager::me().send(role->id(), "篝火喝酒奖励", text, objVec);
				continue;
			}
			role->putObj(objVec);
		}
	}

	return;
}

uint32_t BonfireManager::getBaseRewardExp(TriggerId triggerId, uint32_t level)
{
	Trigger::Ptr trigger = TriggerManager::me().getById(triggerId);
	if(trigger == nullptr)
		return (uint32_t)-1;

	const auto& cfg = BonfireConfig::me().bonfireCfg;
	auto pos = cfg.m_rewardMap.find(trigger->triggerTplId());
	if(pos == cfg.m_rewardMap.end())
		return (uint32_t)-1;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		if(level >= iter->minLevel && level <= iter->maxLevel)
			return iter->addExp;
	}

	return (uint32_t)-1;
}

uint32_t BonfireManager::getTeamPercent(uint8_t num)
{
	const auto& cfg = BonfireConfig::me().bonfireCfg;
	auto pos = cfg.m_teamMap.find(num);
	if(pos == cfg.m_teamMap.end())
		return (uint32_t)-1;

	return pos->second;
}

uint32_t BonfireManager::getDrinkPercent(RoleId roleId)
{
	auto pos = m_drinkMap.find(roleId);
	if(pos == m_drinkMap.end())
		return 0;

	if(pos->second == WineType::none)
		return 0;

	const auto& cfg = BonfireConfig::me().bonfireCfg;
	auto iter = cfg.m_drinkMap.find(static_cast<uint8_t>(pos->second));
	if(iter == cfg.m_drinkMap.end())
		return (uint32_t)-1;

	return iter->second.percent;
}

bool BonfireManager::checkInBonfireRange(TriggerId triggerId, const Coord2D& pos)
{
	Trigger::Ptr trigger = TriggerManager::me().getById(triggerId);
	if(trigger == nullptr)
		return false;

	TriggerTpl::Ptr triggerTpl = TriggerCfg::me().getById(trigger->triggerTplId());
	if(triggerTpl == nullptr)
		return false;

	const Coord2D& center = trigger->pos();
	uint16_t radius = triggerTpl->radius;

	if(std::abs(center.x - pos.x) <= radius && std::abs(center.y - pos.y) <= radius)
		return true;

	return false;
}


void BonfireManager::sendTeamInfo(RoleId roleId, TriggerId triggerId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto pos = m_bonfireMap.find(triggerId);
	if(pos == m_bonfireMap.end())
		return;

	Trigger::Ptr trigger = TriggerManager::me().getById(triggerId);
	if(trigger == nullptr)
		return;

	std::vector<uint8_t> buf;
	buf.reserve(256);
	buf.resize(sizeof(PublicRaw::RetBonfireTeamInfo));

	auto* msg = reinterpret_cast<PublicRaw::RetBonfireTeamInfo*>(buf.data());
	msg->triggerId = triggerId;
	msg->sec = trigger->leftTime();
	msg->wine = static_cast<uint8_t>(getDrinkWineType(roleId));

	msg->size = 0;
	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetBonfireTeamInfo::TeamItem));
		auto* msg  = reinterpret_cast<PublicRaw::RetBonfireTeamInfo*>(buf.data());

		msg->data[msg->size].job = iter->job;
		msg->data[msg->size].sex = iter->sex;
		std::memset(msg->data[msg->size].name, 0, NAME_BUFF_SZIE);
		iter->name.copy(msg->data[msg->size].name, NAME_BUFF_SZIE);
		
		++msg->size;
	}

	role->sendToMe(RAWMSG_CODE_PUBLIC(RetBonfireTeamInfo), buf.data(), buf.size());
	return;
}

std::string BonfireManager::getOwnerName(TriggerId triggerId)
{
	auto pos = m_bonfireMap.find(triggerId);
	if(pos == m_bonfireMap.end())
		return "";

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		if(!iter->isOwner)
			continue;

		return iter->name;
	}

	return "";
}

WineType BonfireManager::getDrinkWineType(RoleId roleId)
{
	auto pos = m_drinkMap.find(roleId);
	if(pos == m_drinkMap.end())
		return WineType::none;

	return pos->second;
}

}
