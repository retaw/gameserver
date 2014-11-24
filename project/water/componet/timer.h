/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-24 11:18 +0800
 *
 * Description: 定时器
 */

#include "datetime.h"
#include "event.h"

#include <map>
#include <atomic>
#include <functional>

namespace water{
namespace componet{

class Timer final
{
    //typedef std::chrono::high_resolution_clock TheClock;
    typedef Clock TheClock;
public:
    Timer();
    ~Timer() = default;

    void run();
    void stop();

    int64_t precision() const;

    //注册一个触发间隔
    void regEventHandler(std::chrono::milliseconds interval,
                         const std::function<void (const TimePoint&)>& handle);

private:
    struct EventHandlers
    {
        Event<void (const TimePoint&)> event;
        TheClock::time_point lastEmitTime;
    };

    std::map<TheClock::duration, EventHandlers> m_eventHandlers;

    enum class Switch : uint8_t {on, off};
    std::atomic<Switch> m_switch;
};

}}
