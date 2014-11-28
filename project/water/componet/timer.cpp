#include "timer.h"

#include <thread>
//#include <mutex>

namespace water{
namespace componet{

Timer::Timer()
:m_switch(Switch::off)
{
}

void Timer::run()
{
    m_switch.store(Switch::on);
    while(m_switch.load(std::memory_order_relaxed) == Switch::on)
    {
        TimePoint wakeUp = EPOCH;

        m_lock.lock();
        for(auto& pair : m_eventHandlers)
        {
            TimePoint now = TheClock::now();
            TheClock::time_point nextWakeUp = pair.second.lastEmitTime + pair.first;

            if(now >= nextWakeUp) //执行一次定时事件
            {
                pair.second.lastEmitTime = now;
                pair.second.event(now);
                nextWakeUp = now + pair.first;
            }
            if(wakeUp == EPOCH || nextWakeUp < wakeUp)
                wakeUp = nextWakeUp;
        }
        m_lock.unlock();

        TimePoint now = TheClock::now();
        if(wakeUp > now)
            std::this_thread::sleep_until(wakeUp);
    }
}

void Timer::stop()
{
    m_switch.store(Switch::off);
}

int64_t Timer::precision() const
{
    return TheClock::period::den;
}

void Timer::regEventHandler(std::chrono::milliseconds interval,
                            const std::function<void (const TimePoint&)>& handler)
{
    m_lock.lock();
    auto& info = m_eventHandlers[interval];
    info.event.reg(handler);
    info.lastEmitTime = EPOCH;
    m_lock.unlock();
}


}}
