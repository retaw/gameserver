#include "first_manager.h"
#include "first_config.h"
#include "role.h"
#include "world.h"
#include "channel.h"
#include "role_manager.h"
#include "scene_manager.h"
#include "roles_and_scenes.h"
#include "mail_manager.h"
#include "action_manager.h"
#include "reward_manager.h"

#include "water/componet/logger.h"

#include "protocol/rawmsg/public/first.h"
#include "protocol/rawmsg/public/first.codedef.public.h"

#include "protocol/rawmsg/private/first.h"
#include "protocol/rawmsg/private/first.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

FirstManager::FirstManager()
: m_applyEndTime(EPOCH)
, m_readyEndTime(EPOCH)
, m_endTime(EPOCH)
, m_state(GameState::none)
, m_mode(attack_mode::none)
, execGameOverFlag(false)
{
}

FirstManager FirstManager::m_me;

FirstManager& FirstManager::me()
{
	return m_me;
}

uint32_t FirstManager::getSpanSecOfApplyEnd() const
{
	if(m_applyEndTime == EPOCH)
		return 0;

	TimePoint now = Clock::now();
	if(now >= m_applyEndTime)
		return 0;

	return std::chrono::duration_cast<std::chrono::seconds>(m_applyEndTime - now).count(); 
}

uint32_t FirstManager::getSpanSecOfReadyEnd() const
{
	if(m_readyEndTime == EPOCH)
		return 0;

	TimePoint now = Clock::now();
	if(now >= m_readyEndTime)
		return 0;

	return std::chrono::duration_cast<std::chrono::seconds>(m_readyEndTime - now).count(); 
}

uint8_t FirstManager::getDuanweiType(RoleId roleId) const
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return 0;

	if(!isActionMap(role->scene()))
		return 0;

	uint32_t point = getPoint(roleId);
	if(point == (uint32_t)-1)
		return 0;

	const auto& cfg = FirstConfig::me().firstCfg;
	FirstConfig::First::RewardItem rewardItem;
	if(!cfg.getRewardItemByPoint(rewardItem, point))
		return 0;
	
	return rewardItem.type;
}

void FirstManager::afterRoleEnterScene(Role::Ptr role)
{
	if(role == nullptr)
		return;

	if(!isActionMap(role->scene()))
		return;

	if(!isActionTimeReady())
	{
		role->sendSysChat("未到天下第一活动准备时间");
		role->exitCopyMap();
		return;
	}

	PlayerItem temp;
	temp.roleId = role->id();
	temp.point = 0;
	temp.killNum = 0;
	temp.killTime = Clock::now();
	temp.rewardState = Reward::canGet;
	if(!m_playerSet.insert(temp).second)
	{
		role->sendSysChat("进副本失败");
		role->exitCopyMap();
	}

	//设置和平模式
	role->m_attackMode.setMode(attack_mode::peace, true);
	
	//广播个人九屏
	role->syncScreenDataTo9();
	return;
}

void FirstManager::beforeRoleLeaveScene(Role::Ptr role)
{
	if(role == nullptr)
		return;

	if(!isActionMap(role->scene()))
		return;

	PlayerItem playerItem;
	if(!getPlayerItem(playerItem, role->id()))
		return;

	if(m_state == GameState::ready)
	{
		m_playerSet.erase(playerItem);
		return;
	}
	else if(m_state == GameState::start)
	{
		const auto& cfg = FirstConfig::me().firstCfg;
		FirstConfig::First::RewardItem rewardItem;
		if(!cfg.getRewardItemByPoint(rewardItem, playerItem.point))
		{
			LOG_ERROR("天下第一, 中途退出, 获取奖励失败! 发放奖励失败! name={}, roleId={}, point={}",
					  role->name(), role->id(), playerItem.point);
			m_playerSet.erase(playerItem);
			return;
		}

		ObjItem obj;
		obj.tplId = rewardItem.giftId;
		obj.num = 1;
		obj.bind = Bind::yes;
		if(!role->putObj(obj.tplId, obj.num, obj.bind))
		{
			std::string text = "您参与了天下第一活动, 获得" + rewardItem.name + "段位礼包";
			MailManager::me().send(role->id(), "天下第一活动奖励", text, obj);
		}

		sendBattleResult(role, playerItem.killNum, rewardItem.giftId);
		m_playerSet.erase(playerItem);
	}

	return;
}

void FirstManager::roleBeKilled(Role::Ptr role, Role::Ptr attacker)
{
	if(role == nullptr || attacker == nullptr)
		return;

	if(!isActionMap(role->scene()) || !isActionMap(attacker->scene()))
		return;
	
	if(!isActionBegin())
		return;

	const auto& cfg = FirstConfig::me().firstCfg;
	if(0 == cfg.basePoint)
	{
		LOG_ERROR("天下第一, 配置错误, basePoint=0");
		return;
	}

	PlayerItem oldLoserItem;
	PlayerItem oldWinnerItem;
	if(!getPlayerItem(oldLoserItem, role->id())
	   || !getPlayerItem(oldWinnerItem, attacker->id()))
	{
		LOG_ERROR("天下第一, 死亡结算, 获取数据失败, loserName={}, loserId={}, winnerNmae={}, winnerId={}, m_playerSet={}",
				  role->name(), role->id(),
				  attacker->name(), attacker->id(), m_playerSet.size());
		return;
	}
	
	uint32_t addPoint = cfg.basePoint + oldLoserItem.point;
	uint32_t winnerOldPoint = oldWinnerItem.point;
	uint32_t winnerNewPoint = winnerOldPoint + addPoint;

	PlayerItem newWinnerItem = oldWinnerItem;
	newWinnerItem.point = winnerNewPoint; 
	newWinnerItem.killNum += 1;
	newWinnerItem.killTime = Clock::now();
	
	m_playerSet.erase(oldWinnerItem);
	m_playerSet.insert(newWinnerItem);

	sendGetPointNum(attacker, addPoint);
	if(getDuanweiTypeByPoint(winnerOldPoint) != getDuanweiTypeByPoint(winnerNewPoint))
	{
		attacker->syncScreenDataTo9();
	}

	//死亡结算, 返回战斗结果		
	FirstConfig::First::RewardItem loserRewardItem;
	if(!cfg.getRewardItemByPoint(loserRewardItem, oldLoserItem.point))
	{
		LOG_ERROR("天下第一, 获取奖励失败! 发放死亡奖励失败! loserName={}, loserId={}, loserPoint={}, winnerName={}, winnerId={}",
				  role->name(), role->id(), oldLoserItem.point, 
				  attacker->name(), attacker->id());
		return;
	}
	
	//winner
	attacker->addMoney(MoneyType::money_7, loserRewardItem.zhangong, "天下第一");

	//loser
	if(!role->putObj(loserRewardItem.giftId, 1, Bind::yes))
	{
		ObjItem obj;
		obj.tplId = loserRewardItem.giftId;
		obj.num = 1;
		obj.bind = Bind::yes;
		
		std::string text = "您参与了天下第一活动, 获得" + loserRewardItem.name + "段位礼包";
		MailManager::me().send(role->id(), "天下第一活动奖励", text, obj);
	}
	sendBattleResult(role, oldLoserItem.killNum, loserRewardItem.giftId);
	
	PlayerItem newLoserItem = oldLoserItem;
	newLoserItem.rewardState = Reward::got;
	m_playerSet.erase(oldLoserItem);
	m_playerSet.insert(newLoserItem);

	return;
}

void FirstManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&FirstManager::timerLoop, this, StdInterval::sec_1, _1));
    World::me().regTimer(std::chrono::seconds(5), 
                         std::bind(&FirstManager::timerLoop, this, StdInterval::sec_5, _1));
    World::me().regTimer(std::chrono::seconds(60), 
                         std::bind(&FirstManager::timerLoop, this, StdInterval::min_1, _1));
}

void FirstManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
	switch(interval)
	{
		case StdInterval::sec_1:
			timer(now);
			break;
		case StdInterval::sec_5:
			giveOutRandomReward();
			break;
		case StdInterval::min_1:
			sendTopListToAll();
			break;
		default:
			break;
	}
	return;
}

void FirstManager::timer(const TimePoint& now)
{
	checkAndSetState();

	switch(m_state)
	{
	case GameState::ready:
		ready();
		break;
	case GameState::start:
		start();
		break;
	case GameState::over:
		over();
		kickoutActionMap(now);
		break;
	case GameState::timeout:
		over();
		kickoutActionMap(now);
		break;
	default:
		break;
	}
}

void FirstManager::checkAndSetState()
{
	if(m_state != GameState::apply && isActionTimeApply())
	{
		m_state = GameState::apply;
		calcEndTime();
		ActionManager::me().setActionState(ActionType::first_apply, ActionState::begin);
	}
	else if(m_state == GameState::apply && !isActionTimeApply())
	{
		m_state = GameState::none;
		ActionManager::me().setActionState(ActionType::first_apply, ActionState::end);
	}
	else if(m_state != GameState::ready && isActionTimeReady())
	{
		m_state = GameState::ready;
		ActionManager::me().setActionState(ActionType::first_ready, ActionState::begin);
	}
	else if(m_state == GameState::start && !isActionBegin())
	{
		m_state = GameState::timeout;
	}

	return;
}

void FirstManager::calcEndTime()
{
	TimePoint now = Clock::now();
	::tm detail;
	if(!componet::timePointToTM(now, &detail))
		return;

	const auto& cfg = FirstConfig::me().firstCfg;
	auto pos = cfg.m_actionMap.find(detail.tm_wday);
	if(pos == cfg.m_actionMap.end())
		return;

	std::string dateStr = componet::date();
	if(dateStr.empty())
		return;

	std::string applyEndTime = dateStr + "-" + pos->second.applyEndTime;
	std::string readyEndTime = dateStr + "-" + pos->second.beginTime;
	std::string endTime = dateStr + "-" + pos->second.endTime;
	m_applyEndTime = componet::stringToTimePoint(applyEndTime);
	m_readyEndTime = componet::stringToTimePoint(readyEndTime);
	m_endTime = componet::stringToTimePoint(endTime);
	return;
}

void FirstManager::ready()
{
	if(!isActionBegin())
		return;

	m_state = GameState::start;
	ActionManager::me().setActionState(ActionType::first_ready, ActionState::end); 
	return;
}

void FirstManager::start()
{
	if(m_mode != attack_mode::all)
	{
		m_mode = attack_mode::all;
		setModeOfAll(attack_mode::all);
		sendSpanSecOfPkEndToAll();
	}

	uint32_t aliveNum = getAlivePlayerNum();
	if(0 == aliveNum || 1 == aliveNum)
	{
		m_state = GameState::over;
		return;
	}

	return;
}

void FirstManager::over()
{
	if(m_mode != attack_mode::peace)
	{
		m_mode = attack_mode::peace;
		setModeOfAll(attack_mode::peace);
	
		m_endTime = Clock::now();
	}

	//以下战斗结算逻辑，处理且仅处理一次
	if(execGameOverFlag)
		return;

	judgeWinner();

	const auto& cfg = FirstConfig::me().firstCfg;
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		if(iter->rewardState != Reward::canGet)
			continue;
	
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		FirstConfig::First::RewardItem rewardItem;
		if(!cfg.getRewardItemByPoint(rewardItem, iter->point))
		{
			LOG_ERROR("天下第一, 战斗结束, 获取奖励失败! 发放奖励失败! name={}, roleId={}, point={}",
					  role->name(), role->id(), iter->point);
			continue;
		}

		if(!role->putObj(rewardItem.giftId, 1, Bind::yes))
		{
			ObjItem obj;
			obj.tplId = rewardItem.giftId;
			obj.num = 1;
			obj.bind = Bind::yes;

			std::string text = "您参与了天下第一活动, 获得" + rewardItem.name + "段位礼包";
			MailManager::me().send(role->id(), "天下第一活动奖励", text, obj);
		}
		sendBattleResult(role, iter->killNum, rewardItem.giftId);
	}

	execGameOverFlag = true;
	return;
}

void FirstManager::setModeOfAll(attack_mode mode)
{
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		role->m_attackMode.setMode(mode, true);
	}
	
	return;
}

uint32_t FirstManager::getAlivePlayerNum()
{
	uint32_t count = 0;
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		if(!role->isDead())
			count += 1;
	}

	return count;
}

uint32_t FirstManager::getPoint(RoleId roleId) const
{
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		if(iter->roleId != roleId)
			continue;

		return iter->point;
	}

	return (uint32_t)-1;
}

bool FirstManager::getPlayerItem(PlayerItem& playerItem, RoleId roleId)
{
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		if(iter->roleId != roleId)
			continue;

		playerItem = *iter;
		return true;
	}

	return false;
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

bool FirstManager::isActionWorld()
{
	const auto& cfg = FirstConfig::me().firstCfg;
	if(0 == cfg.mapId)
		return false;

	if(SceneManager::me().getById(cfg.mapId) == nullptr)
		return false;
	
	return true;
}

bool FirstManager::isActionMap(Scene::Ptr scene) const
{
	if(scene == nullptr)
		return false;

	const auto& cfg = FirstConfig::me().firstCfg;
	if(0 == cfg.mapId)
		return false;

	if(scene->mapId() != cfg.mapId)
		return false;

	return true;
}

void FirstManager::giveOutRandomReward()
{
	if(!isActionWorld())
		return;

	if(m_playerSet.empty())
		return;

	const auto& cfg = FirstConfig::me().firstCfg;
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr || role->isDead())
			continue;

		std::vector<ObjItem> objVec;
		if(!RewardManager::me().getRandomReward(cfg.randomRewardId, 1, role->level(), role->job(), objVec))
		{
			LOG_ERROR("天下第一, 获取随机奖励失败, 发放奖励失败, name={}, roleId={}, level={}, job={}, rewardId={}",
					  role->name(), role->id(), role->level(),
					  role->job(), cfg.randomRewardId);
			continue;
		}
		
		if(objVec.empty())
			continue;

		if(!role->checkPutObj(objVec))
		{
			std::string text = "由于背包空间不足, 通过邮件发放天下第一定时奖励, 请注意查收";
			MailManager::me().send(role->id(), "天下第一定时奖励", text, objVec);
			continue;
		}
		role->putObj(objVec);
	}

	return;
}

void FirstManager::sendGetPointNum(Role::Ptr role, uint32_t point)
{
	if(role == nullptr)
		return;

	PublicRaw::RetGetPointNum send;
	send.point = point;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetGetPointNum), &send, sizeof(send));
	return;
}

void FirstManager::sendBattleResult(Role::Ptr role, uint32_t killNum, uint32_t rewardId)
{
	std::string name = "";
	if(m_state == GameState::over || m_state == GameState::timeout)
		name = m_winnerName;

	PublicRaw::RetFirstBattleResult send;
	send.killNum = killNum;
	send.rewardId = rewardId;
	std::memset(send.winnerName, 0, NAME_BUFF_SZIE);
	name.copy(send.winnerName, NAME_BUFF_SZIE);
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetFirstBattleResult), &send, sizeof(send));
	return;
}

void FirstManager::sendTopListToAll()
{
	if(m_playerSet.empty())
		return;

	if(m_state == GameState::apply || m_state == GameState::ready)
		return;

	std::vector<uint8_t> buf;
	buf.reserve(256);
	buf.resize(sizeof(PublicRaw::RetFirstTopList));

	auto* msg = reinterpret_cast<PublicRaw::RetFirstTopList*>(buf.data());
	msg->size = 0;

	uint32_t rankNum = 0;
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		if(rankNum >= 3)
			break;

		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		if(role->isDead())
			continue;

		rankNum += 1;

		buf.resize(buf.size() + sizeof(PublicRaw::RetFirstTopList::TopList));
		auto* msg  = reinterpret_cast<PublicRaw::RetFirstTopList*>(buf.data());

		msg->data[msg->size].rank = rankNum;
		msg->data[msg->size].killNum = iter->killNum;
		std::memset(msg->data[msg->size].name, 0, NAME_BUFF_SZIE);
		role->name().copy(msg->data[msg->size].name, NAME_BUFF_SZIE);
		
		++msg->size;
	}

	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		msg->myRank = getRank(iter->roleId);
		msg->myKillNum = iter->killNum;
		msg->myDuanweiType = getDuanweiTypeByPoint(iter->point);
		role->sendToMe(RAWMSG_CODE_PUBLIC(RetFirstTopList), buf.data(), buf.size());
	}
}

uint32_t FirstManager::getRank(RoleId roleId)
{
	uint32_t rankNum = 1;
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		if(role->isDead())
			continue;

		if(iter->roleId != roleId)
			rankNum += 1;

		return rankNum;
	}

	return 0;
}

uint8_t FirstManager::getDuanweiTypeByPoint(uint32_t point)
{
	const auto& cfg = FirstConfig::me().firstCfg;
	FirstConfig::First::RewardItem rewardItem;
	if(!cfg.getRewardItemByPoint(rewardItem, point))
		return 0;
	
	return rewardItem.type;
}

void FirstManager::judgeWinner()
{
	if(m_playerSet.empty())
		return;

	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		if(role->isDead())
			continue;

		m_winnerName = role->name();
		const auto& cfg = FirstConfig::me().firstCfg;
		role->m_title.addTitle(cfg.winnerTitleId);
		
		ObjItem obj;
		obj.tplId = cfg.winnerGiftId;
		obj.num = 1;
		obj.bind = Bind::yes;

		if(!role->putObj(obj.tplId, obj.num, obj.bind))
		{
			std::string text = "由于背包空间不足, 天下第一获胜者奖励将通过邮件发送, 请注意查收";
			MailManager::me().send(role->id(), "系统邮件", text, obj);
		}

		Channel::me().sendSysNotifyToGlobal(ChannelType::screen_middle, "恭喜 {} 在天下第一活动中获得第一名, 获得了天下第一称号, 以及丰富的奖励", role->name());
		LOG_TRACE("天下第一, 天下第一获得者: name={}, roleId={}, killNum={}, point={}, titleId={}, rewardId={}",
				  role->name(), role->id(), iter->killNum, iter->point, 
				  cfg.winnerTitleId, cfg.randomRewardId);

		//world -> func
		PrivateRaw::UpdateFirstPlayerInfo send;
		send.roleId = role->id();
		send.job = role->job();
		send.sex = role->sex();
		std::memset(send.name, 0, NAME_BUFF_SZIE);
		role->name().copy(send.name, NAME_BUFF_SZIE);

		ProcessIdentity funcId("func", 1);
		World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(UpdateFirstPlayerInfo), &send, sizeof(send));
		
		sendTopListToAll();
		break;
	}

	return;
}

void FirstManager::sendSpanSecOfPkEndToAll()
{
	if(m_playerSet.empty())
		return;

	if(m_endTime == EPOCH)
		return;

	TimePoint now = Clock::now();
	if(now >= m_endTime)
		return;

	uint32_t spanSec = std::chrono::duration_cast<std::chrono::seconds>(m_endTime - now).count();

	PublicRaw::RetSpanSecOfPkEnd send;
	send.sec = spanSec;
	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end(); ++iter)
	{
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
			continue;

		role->sendToMe(RAWMSG_CODE_PUBLIC(RetSpanSecOfPkEnd), &send, sizeof(send));
	}

	return;
}

void FirstManager::kickoutActionMap(const TimePoint& now)
{
	if(m_endTime == EPOCH)
		return;

	if(m_playerSet.empty())
	{
		reset();
		return;
	}

	const auto& cfg = FirstConfig::me().firstCfg;
	TimePoint needGobackTime = m_endTime + std::chrono::seconds {cfg.kickoutSec};
	if(now < needGobackTime)
		return;

	for(auto iter = m_playerSet.begin(); iter != m_playerSet.end();)
	{
		Role::Ptr role = RoleManager::me().getById(iter->roleId);
		if(role == nullptr)
		{
			++iter;
			continue;
		}

		iter = m_playerSet.erase(iter);
		if(role->isDead())
		{
			role->relive();
		}

		role->exitCopyMap(); 
	}

	//重置
	reset();
	return;
}

void FirstManager::reset()
{
	m_applyEndTime = EPOCH;
	m_readyEndTime = EPOCH;
	m_endTime = EPOCH;
	m_state = GameState::none;
	m_mode = attack_mode::none;
	execGameOverFlag = false;
	m_winnerName.clear();
	m_playerSet.clear();

	return;
}

}
