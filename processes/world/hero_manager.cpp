#include "hero_manager.h"
#include "hero_config.h"
#include "hero_ids.h"
#include "role.h"
#include "scene.h"
#include "world.h"
#include "pet_manager.h"
#include "role_manager.h"

#include "water/common/roledef.h"
#include "water/componet/logger.h"

#include "protocol/rawmsg/public/hero_scene.h"
#include "protocol/rawmsg/public/hero_scene.codedef.public.h"

#include "protocol/rawmsg/private/hero.h"
#include "protocol/rawmsg/private/hero.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"  

#include <chrono>

namespace world{

HeroManager::HeroManager(Role& owner, TimePoint recallHeroTime)
: m_owner(owner)
, m_recallTimePoint(recallHeroTime)
{
}

void HeroManager::loadFromDB(const std::vector<HeroInfoPra> createdHeroVec)
{
	if(createdHeroVec.empty())
		return;

	for(auto iter = createdHeroVec.begin(); iter != createdHeroVec.end(); ++iter)
	{
		auto pos = m_createdHeroMap.find(iter->job);
		if(pos != m_createdHeroMap.end())
			continue;

		m_createdHeroMap.insert(std::make_pair(iter->job, *iter));
	}

	for(auto pos = m_createdHeroMap.begin(); pos != m_createdHeroMap.end(); ++pos)
	{
		requestHeroSerializeData(pos->first);
	}
	return;
}

void HeroManager::afterRoleEnterScene()
{
	sendSummonHeroNeedSpanSec();
	
	if(m_owner.m_horse.isRide() || !m_owner.getSummonHeroFlag())
		return;

	if(getSummonHero() == nullptr)
	{	
		requestSummonHero(m_owner.getDefaultCallHero());
	}

	return;
}

void HeroManager::beforeRoleLeaveScene()
{
	passiveRecallHero();
}

void HeroManager::roleOffline()
{
    Hero::Ptr hero = getSummonHero();
    if(nullptr != hero)
        hero->m_buffM.processOffline();
	requestRecallHero();
}

void HeroManager::roleDie()
{
	requestRecallHero();
}

void HeroManager::timerLoop(StdInterval interval, const water::componet::TimePoint& now)
{
    Hero::Ptr hero = getSummonHero();
    if(nullptr == hero)
        return;

    hero->timerLoop(interval, now);
}

void HeroManager::requestCreateHero(Job job, Sex sex)
{
	if(m_createdHeroMap.size() >= MAX_HERO_NUM)
		return;
	
	auto pos = m_createdHeroMap.find(job);
	if(pos != m_createdHeroMap.end())
		return;

	const auto& cfg = HeroConfig::me().heroCfg;
	if(cfg.m_needLevel > m_owner.level())
	{
		m_owner.sendSysChat("等级不足");
		return;
	}

	auto iter = cfg.m_heroMap.find(static_cast<uint8_t>(job));
	if(iter == cfg.m_heroMap.end())
		return;

	//非第一次创建英雄则消耗材料
	if(!m_createdHeroMap.empty())
	{
		const uint8_t createNum = m_createdHeroMap.size() + 1; 
		auto it = cfg.m_consumeMap.find(createNum);
		if(it == cfg.m_consumeMap.end())
			return;

		if(it->second.needLevel > m_owner.level())
		{
			m_owner.sendSysChat("等级不足");
			return;
		}

		uint32_t needTplId = it->second.needTplId;
		uint16_t needNum = it->second.needNum;
		uint16_t objNum = m_owner.m_packageSet.getObjNum(needTplId, PackageType::role);
		if(needNum > objNum)
		{
			m_owner.sendSysChat("材料不足");
			return;
		}
		
		if(!m_owner.m_packageSet.eraseObj(needTplId, needNum, PackageType::role, "创建英雄"))
			return;
	}

	HeroInfoPra temp;
	temp.job = job;
	temp.sex = sex;
	temp.level = 1;
	temp.exp = 0;
	temp.hp = 0;
	temp.mp = 0;
	temp.turnLife = TurnLife::zero;
    temp.clother = 0;

	m_createdHeroMap.insert(std::make_pair(temp.job, temp));

	insertHeroInfoToDB(temp);
	requestHeroSerializeData(job);
	sendCreatedHeroList();
	m_owner.sendSysChat(ChannelType::screen_middle, "恭喜玩家{}创建{}，从此行走江湖又多一助力",
					  m_owner.name(), iter->second.name);
	return;
}

void HeroManager::sendCreatedHeroList()
{
	std::vector<uint8_t> buf;
	buf.reserve(64);
	buf.resize(sizeof(PublicRaw::RetCreatedHeroList));

	auto* msg  = reinterpret_cast<PublicRaw::RetCreatedHeroList*>(buf.data());
	msg->size = 0;

	for(auto pos = m_createdHeroMap.begin(); pos != m_createdHeroMap.end(); ++pos)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetCreatedHeroList::CreatedHero));
		auto* msg  = reinterpret_cast<PublicRaw::RetCreatedHeroList*>(buf.data());
	
		msg->data[msg->size].job = pos->first;
		msg->data[msg->size].sex = pos->second.sex;
		++msg->size;
	}
	
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetCreatedHeroList), buf.data(), buf.size());
	return;
}

void HeroManager::requestSummonHero(Job job)
{
	if(getSummonHero() != nullptr || job == Job::none)
		return;

	Scene::Ptr s = m_owner.scene(); 
	if(s == nullptr)
		return;

	if(!m_owner.getSummonHeroFlag())
	{
		const auto& cfg = HeroConfig::me().heroCfg;
		TimePoint canSummonTime = getRecallTimePoint() + std::chrono::seconds {cfg.m_needSpanSec};
		if(componet::Clock::now() < canSummonTime)
		{
			m_owner.sendSysChat("英雄已被召回，召唤冷却中");
			return;
		}
	}

	if(m_createdHeroMap.find(job) == m_createdHeroMap.end())
	{
		m_owner.sendSysChat("未创建此英雄, 不可召唤");
		return;
	}

	auto pos = m_herosMap.find(job);
	if(pos == m_herosMap.end())
		return;

	Hero::Ptr hero = pos->second;
	if(hero == nullptr)
		return;

	hero->markErase(false);
	hero->setDir(m_owner.dir());
	hero->reset();
	HeroIDs::me().insertOrUpdate(hero);
	
	Coord2D rolePos = m_owner.pos();
	Coord2D heroPos = (rolePos.neighbor(Direction::leftup)).neighbor(Direction::leftup);
	if(!s->addHero(hero, heroPos, 5)) 
    {
        HeroIDs::me().erase(hero);
		return;
    }
	
	hero->afterEnterScene();
	setSummonHero(hero);
    PetManager::me().summonPet(hero->petTplId(), hero->petSkillId(), hero->petLevel(), hero);
    m_owner.setSummonHeroFlag(true);
	m_owner.sendSysChat("召唤英雄成功");

	return;
}

void HeroManager::requestHeroSerializeData(Job job)
{
	PrivateRaw::ReqHeroSerializeData send;
	send.roleId = m_owner.id();
    send.job = job;

	ProcessIdentity dbcachedId("dbcached", 1);
	World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(ReqHeroSerializeData), &send, sizeof(send));
    LOG_DEBUG("英雄, 向DB请求序列化数据, role({}, {}), hero job={}", 
			  m_owner.name(), send.roleId, send.job);
	return;
}

void HeroManager::retHeroSerializeData(const PrivateRaw::RetHeroSerializeData* rev)
{
    LOG_DEBUG("英雄, DB返回序列化数据, roleId={}, hero job={}, rev->bufsize={}",
			  rev->roleId, rev->job, rev->size);

	if(m_owner.id() != rev->roleId)
		return;
	
	if(m_herosMap.find(rev->job) != m_herosMap.end())
		return;

	auto pos = m_createdHeroMap.find(rev->job);
	if(pos == m_createdHeroMap.end())
	{
		m_owner.sendSysChat("未创建此英雄, 不可召唤");
		return;
	}

	Role::Ptr role = RoleManager::me().getById(m_owner.id());
	if(role == nullptr)
		return;

    Hero::Ptr hero = std::make_shared<Hero>(HeroIDs::me().allocId(), role, pos->second, rev);
	if(hero == nullptr)
	{	
		LOG_ERROR("英雄, 召唤失败, job={}, role=({}, {}, {})", 
				  rev->job, m_owner.name(), m_owner.id(), m_owner.account());
		return;
	}

	m_herosMap.insert(std::make_pair(hero->job(), hero));
	hero->m_packageSet.setOwner(hero);
	hero->sendMainToMe();
    hero->m_skillM.unlockSkill(false);
    hero->sendSkillListToMe();
	return;
}

/*
 * 用户行为，要求主动删除场景上英雄
 */
void HeroManager::requestRecallHero()
{
	m_owner.setSummonHeroFlag(false);

	Hero::Ptr hero = getSummonHero(); 
	if(hero == nullptr)
		return;

	setRecallTimePoint(componet::Clock::now());
	
    auto pet = hero->pet();
    if(nullptr != pet)
        pet->erase(true);
	
	//保存hero下线数据
	saveHeroOffline(hero);
	hero->beforeLeaveScene();

    hero->markErase(true); //召回英雄, 视英雄死亡
    setSummonHero(nullptr);
	m_owner.sendSysChat("英雄已被召回");
	return;
}

/*
 * 服务器逻辑被动删除场景英雄(summonHeroFlag保持之前状态)
 */
void HeroManager::passiveRecallHero()
{
	Hero::Ptr hero = getSummonHero(); 
	if(hero == nullptr)
		return;

    auto pet = hero->pet();
    if(nullptr != pet)
        pet->erase(false);
	
	//保存hero下线数据
	saveHeroOffline(hero);
	hero->beforeLeaveScene();
	setSummonHero(nullptr);

	Scene::Ptr s = hero->scene();
	if(s != nullptr)
	{
		s->eraseHero(hero);
	}

	HeroIDs::me().erase(hero);
}

void HeroManager::requestHeroChangePos(Job job, Coord1D posX, Coord1D posY, componet::Direction dir, MoveType type)
{
	Hero::Ptr hero = getSummonHero();
	if(hero == nullptr)
		return;

	if(hero->job() != job)
		return;

	if(hero->isDead() || hero->needErase())
		return;

	hero->changePos(Coord2D(posX, posY), dir, type);
	return;
}

void HeroManager::requestSetDefaultCallHero(Job job)
{
	auto pos = m_createdHeroMap.find(job);
	if(pos == m_createdHeroMap.end())
		return;

	m_owner.setDefaultCallHero(job);
	//m_owner.sendSysChat("常用英雄修改为{}", pos->second.name);
	return;
}

HeroInfoPra HeroManager::getDefaultHeroInfoPra() const
{
    HeroInfoPra param;
    std::memset(&param, 0, sizeof(param));

    auto iter = m_createdHeroMap.find(m_owner.getDefaultCallHero());
    if(iter == m_createdHeroMap.end())
        return param;

	param = iter->second;
    return param;
}

void HeroManager::updateDefaultHeroClother(TplId clotherId)
{
    if(m_createdHeroMap.find(m_owner.getDefaultCallHero()) == m_createdHeroMap.end())
        return;
    
	HeroInfoPra& param = m_createdHeroMap[m_owner.getDefaultCallHero()];
    param.clother = clotherId;

    PrivateRaw::UpdateHeroClothes send;
    send.roleId = m_owner.id();
    send.job = param.job;
    send.clother = clotherId;

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateHeroClothes), &send, sizeof(send));
}

TplId HeroManager::getDefaultHeroClother() const
{
	auto pos = m_createdHeroMap.find(m_owner.getDefaultCallHero());
	if(pos == m_createdHeroMap.end())
		return 0;

	return pos->second.clother;
}

void HeroManager::upgradeSkill(TplId skillId)
{
	Hero::Ptr hero = getDefaultHero();
    if(nullptr == hero)
    {
        LOG_ERROR("英雄, 技能升级, 获取Hero::Ptr失败, m_herosMap={}, role=({}, {}, {})",
				  m_owner.name(), m_owner.id(), m_owner.account());
        return;
    }
    
	hero->m_skillM.upgradeSkill(skillId, hero);
	return;
}

void HeroManager::strengthenSkill(TplId skillId)
{
	Hero::Ptr hero = getDefaultHero();
    if(nullptr == hero)
    {
        LOG_ERROR("英雄, 技能强化, 获取Hero::Ptr失败, m_herosMap={}, role=({}, {}, {})",
				  m_owner.name(), m_owner.id(), m_owner.account());
        return;
    }
    
	hero->m_skillM.strengthenSkill(skillId, hero);
	return;
}

void HeroManager::setSummonHero(Hero::Ptr hero)
{
	m_summonHero = hero;
}

Hero::Ptr HeroManager::getSummonHero() const
{
	return m_summonHero;
}

Hero::Ptr HeroManager::getHeroByJob(Job job) const
{
	auto pos = m_herosMap.find(job);
	if(pos == m_herosMap.end())
		return nullptr;

	return pos->second;
}

Hero::Ptr HeroManager::getDefaultHero() const
{
	Job job = m_owner.getDefaultCallHero();
	if(job == Job::none)
		return nullptr;

	return getHeroByJob(job);		
}

void HeroManager::setRecallTimePoint(TimePoint time)
{
	m_recallTimePoint = time;
	sendSummonHeroNeedSpanSec();
}

TimePoint HeroManager::getRecallTimePoint() const
{
	return m_recallTimePoint;
}

void HeroManager::sendSummonHeroNeedSpanSec()
{
	if(m_owner.getSummonHeroFlag())
		return;

	const auto& cfg = HeroConfig::me().heroCfg;
	TimePoint canSummonTime = getRecallTimePoint() + std::chrono::seconds {cfg.m_needSpanSec};
	TimePoint now = componet::Clock::now();
	if(now >= canSummonTime)
		return;

	uint32_t needSpanSec = (std::chrono::duration_cast<std::chrono::seconds>(canSummonTime - now).count());
	PublicRaw::RetSummonHeroNeedSpanSec send;
	send.needSpanSec = needSpanSec;
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetSummonHeroNeedSpanSec), &send, sizeof(send));
}

void HeroManager::insertHeroInfoToDB(HeroInfoPra info)
{
	PrivateRaw::InsertHeroInfo send;
	send.roleId = m_owner.id();
	send.data.job = info.job; 
	send.data.sex = info.sex;
	send.data.level = info.level;
	send.data.exp = 0;
	send.data.hp = 0;
	send.data.mp = 0;
	send.data.turnLife = TurnLife::zero;
    send.data.clother = info.clother;

	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(InsertHeroInfo), &send, sizeof(send));
	LOG_TRACE("英雄, send insertHeroInfo to {}, {}, roleId={}, job={}, sex={}, level={}, exp={}, hp={}, mp={},, role=({}, {}, {})",
			  dbcachedId, ret ? "ok" : "falied",
			  send.roleId, send.data.job, send.data.sex,
			  send.data.level, send.data.exp, send.data.hp, send.data.mp,
			  m_owner.name(), m_owner.id(), m_owner.account());
	
	return;
}

void HeroManager::saveHeroOffline(Hero::Ptr hero) const
{
	PrivateRaw::SaveHeroOffline send;
	send.roleId = m_owner.id();
	send.job = hero->job(); 
	send.hp = hero->getHp();
	send.mp = hero->getMp();
	send.recallTimePoint = getRecallTimePoint();
    send.clother = getDefaultHeroClother();
    send.petTplId = hero->petTplId();

	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(SaveHeroOffline), &send, sizeof(send));
	LOG_TRACE("英雄, 召回, send SaveHeroOffline to {}, {}, roleId={}, job={}, hp={}, mp={}, recallTimePoint={}, role=({}, {}, {})",
			  dbcachedId, ret ? "ok" : "falied",
			  send.roleId, send.job, send.hp, send.mp, toUnixTime(send.recallTimePoint),
			  m_owner.name(), m_owner.id(), m_owner.account());
	
	return;
}


}
