/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-24 11:18 +0800
 *
 * Description: 定时器
 */

#include "datetime.h"
#include "event.h"
#include "spinlock.h"

#include <map>
#include <set>
#include <mutex>
#include <functional>

namespace water{
namespace componet{

class Timer
{
    //typedef std::chrono::high_resolution_clock TheClock;
    typedef Clock TheClock;
public:
    typedef std::pair<TheClock::duration, Event<void (const TimePoint&)>::RegID> RegID;

    Timer();
    ~Timer() = default;

    void tick();

    int64_t precision() const;

    //注册一个触发间隔
    RegID regEventHandler(std::chrono::milliseconds interval,
                         const std::function<void (const TimePoint&)>& handle);
    void unregEventHandler(RegID);
public:
    Event<void (Timer*)> e_stop;

private:
    struct EventHandlers
    {
        Event<void (const TimePoint&)> event;
        TheClock::time_point lastEmitTime;
    };

    Spinlock m_lock;
    std::map<TheClock::duration, EventHandlers> m_eventHandlers;

    std::set<RegID> m_unregedId;
};

}}
