#include "hero_ids.h"
#include "scene_manager.h"
#include "fire.h"
#include "world.h"

namespace world{

HeroIDs::HeroIDs()
: m_lastHeroId(1)
{
}

HeroIDs& HeroIDs::me()
{
    static HeroIDs me;
    return me;
}

Hero::Ptr HeroIDs::getById(HeroId id)
{
    if(m_heroList.find(id) == m_heroList.end())
        return nullptr;
    return m_heroList[id];
}

HeroId HeroIDs::allocId()
{
    return m_lastHeroId++;
}

void HeroIDs::insertOrUpdate(Hero::Ptr hero)
{
	auto pos = m_heroList.find(hero->id());
	if(pos == m_heroList.end())
	{
		m_heroList.insert(std::make_pair(hero->id(), hero));
		return;
	}

	pos->second = hero;
	return;
}

void HeroIDs::erase(Hero::Ptr hero)
{
    m_heroList.erase(hero->id());
}

void HeroIDs::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::milliseconds(500), 
                         std::bind(&HeroIDs::timerLoop, this, _1));
}

void HeroIDs::timerLoop(const water::componet::TimePoint& now)
{
    auto it = m_heroList.begin();
    for(; it != m_heroList.end(); )
    {
        auto& hero = it->second;
        if(nullptr == hero)
        {
            m_heroList.erase(it++);
        }
        else if(hero->needErase())
        {
            FireManager::me().erase(hero->id());
            Scene::Ptr s = hero->scene();
            if(s != nullptr)
			{
                s->eraseHero(hero);
			}
			m_heroList.erase(it++);
        }
        else
            ++it;
    }
}

}
