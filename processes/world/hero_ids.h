#ifndef PROCESS_WORLD_HERO_IDS_H
#define PROCESS_WORLD_HERO_IDS_H

/*
 *
 * 场景上英雄ID管理器
 *
 */

#include "hero.h"

namespace world{

class HeroIDs
{
public:
    HeroIDs();
    ~HeroIDs() = default;

    static HeroIDs& me();

public:
    Hero::Ptr getById(HeroId);
    HeroId allocId();

	void insertOrUpdate(Hero::Ptr);
	void erase(Hero::Ptr);

    void regTimer();
    void timerLoop(const water::componet::TimePoint& now);

private:
    HeroId m_lastHeroId;
    std::unordered_map<HeroId, Hero::Ptr> m_heroList;
};

}

#endif

