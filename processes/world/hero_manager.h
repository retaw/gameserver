/*
 * Author: zhupengfei
 *
 * Created: 2015-06-16 11:00 +0800  
 *
 * Modified: 2015-06-16  11:00 +0800
 *
 * Description: 角色的英雄管理器
 */

#ifndef PROCESS_WORLD_HERO_MANAGER_HPP
#define PROCESS_WORLD_HERO_MANAGER_HPP

#include "hero.h"

#include "water/common/herodef.h"
#include "water/componet/class_helper.h"
#include "water/process/process_id.h"

#include "protocol/rawmsg/private/package.h"
#include "protocol/rawmsg/private/package.codedef.private.h"

#include <memory>

namespace world{

using namespace water;
using water::process::ProcessIdentity;

class HeroManager
{
public:
	explicit HeroManager(Role& owner, TimePoint recallHeroTime);
	~HeroManager() = default;

public:
	void loadFromDB(const std::vector<HeroInfoPra> createdHeroVec);

	void afterRoleEnterScene();
	void beforeRoleLeaveScene();
	void roleOffline();
	void roleDie();
	void timerLoop(StdInterval interval, const water::componet::TimePoint& now);

	void requestCreateHero(Job job, Sex sex);
	void sendCreatedHeroList();
	void requestSummonHero(Job job);

	void retHeroSerializeData(const PrivateRaw::RetHeroSerializeData* rev);
	
	//玩家主动要求删除英雄
	void requestRecallHero();
    //被动删除英雄(非玩家行为)
    void passiveRecallHero();

	void requestHeroChangePos(Job job, Coord1D posX, Coord1D posY, componet::Direction, MoveType type);
	void requestSetDefaultCallHero(Job job);

public:
    //获取默认英雄基础属性
    HeroInfoPra getDefaultHeroInfoPra() const;
    //更新英雄衣服
    void updateDefaultHeroClother(TplId clotherId);
    //获得英雄衣服
    TplId getDefaultHeroClother() const;

    void upgradeSkill(TplId skillId);
    void strengthenSkill(TplId skillId);

	Hero::Ptr getSummonHero() const;
	Hero::Ptr getHeroByJob(Job job) const;
	Hero::Ptr getDefaultHero() const;

private:
	void requestHeroSerializeData(Job job);
	void setSummonHero(Hero::Ptr hero);
	
	void setRecallTimePoint(TimePoint time);
	TimePoint getRecallTimePoint() const;

	void sendSummonHeroNeedSpanSec();
	void insertHeroInfoToDB(HeroInfoPra info);
	void saveHeroOffline(Hero::Ptr hero) const;

private:
	Role& m_owner;
	TimePoint m_recallTimePoint;				//召回时间点

private:
	std::map<Job, HeroInfoPra> m_createdHeroMap;	//已创建的英雄	
	std::map<Job, Hero::Ptr> m_herosMap;			//已创建的英雄
	Hero::Ptr m_summonHero;							//当前召唤出的英雄
};


}
#endif
