#include "role_manager.h"

#include "world.h"

namespace world{

RoleManager& RoleManager::me()
{
    static RoleManager me;
    return me;
}

void RoleManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::milliseconds(100),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::msec_100, _1));
    World::me().regTimer(std::chrono::milliseconds(200),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::msec_300, _1));
    World::me().regTimer(std::chrono::milliseconds(500),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::msec_500, _1));

    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&RoleManager::timerLoop, this, StdInterval::sec_1, _1));
    World::me().regTimer(std::chrono::seconds(3),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::sec_3, _1));
    World::me().regTimer(std::chrono::seconds(5),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::sec_5, _1));
    World::me().regTimer(std::chrono::seconds(15),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::sec_15, _1));
    World::me().regTimer(std::chrono::seconds(30),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::sec_30, _1));

    World::me().regTimer(std::chrono::minutes(1),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::min_1, _1));
    World::me().regTimer(std::chrono::minutes(5),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::min_5, _1));
    World::me().regTimer(std::chrono::minutes(10),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::min_10, _1));
    World::me().regTimer(std::chrono::minutes(15),
                         std::bind(&RoleManager::timerLoop, this, StdInterval::min_15, _1));
}

void RoleManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
    for(auto it = begin(); it != end(); ++it)
        (*it)->timerLoop(interval, now);
}

}
