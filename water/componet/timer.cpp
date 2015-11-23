#include "timer.h"

#include <thread>
//#include <mutex>

namespace water{
namespace componet{

Timer::Timer()
//:m_wakeUp(EPOCH)
{
}

void Timer::tick()
{

    TimePoint nearestWakeUp = EPOCH;
    m_lock.lock();
    for(auto& pair : m_eventHandlers)
    {
        TimePoint now = TheClock::now();
        TheClock::time_point wakeUp = pair.second.lastEmitTime + pair.first;

        if(now >= wakeUp) //执行一次定时事件
        {
            pair.second.lastEmitTime = now;
            pair.second.event(now);
            wakeUp = now + pair.first;
        }
        if(nearestWakeUp == EPOCH || wakeUp < nearestWakeUp)
            nearestWakeUp = wakeUp;
    }
    m_lock.unlock();

    TimePoint now = TheClock::now();
    if(nearestWakeUp > now)
        std::this_thread::sleep_until(nearestWakeUp);
}

int64_t Timer::precision() const
{
    return TheClock::period::den;
}

Timer::RegID Timer::regEventHandler(std::chrono::milliseconds interval,
                            const std::function<void (const TimePoint&)>& handler)
{
    std::lock_guard<componet::Spinlock> lock(m_lock);

    auto& info = m_eventHandlers[interval];
    auto eventId = info.event.reg(handler);
    info.lastEmitTime = EPOCH;
    return {interval, eventId};
}

void Timer::unregEventHandler(RegID id)
{
    std::lock_guard<componet::Spinlock> lock(m_lock);

    auto it = m_eventHandlers.find(id.first);
    if(it == m_eventHandlers.end())
        return;

    it->second.event.unreg(id.second);
}

}}
