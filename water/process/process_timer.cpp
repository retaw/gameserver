#include "process_timer.h"

#include "componet/exception.h"
#include "componet/logger.h"

#include <thread>
#include <chrono>

namespace water{
namespace process{

ProcessTimer::ProcessTimer()
: m_suspend(false)
{
}

bool ProcessTimer::exec()
{
    while(checkSwitch())
    {
        if(m_suspend)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        try
        {
            m_timer.tick();
        }
        catch(const componet::ExceptionBase& ex)
        {
            LOG_ERROR("timer, unexcept exception: [{}]", ex.what());
        }
    }

    return true;
}

bool ProcessTimer::isSuspend() const
{
    return m_suspend;
}

void ProcessTimer::suspend()
{
    m_suspend = true;
}

void ProcessTimer::resume()
{
    m_suspend = false;
}

int64_t ProcessTimer::precision() const
{
    return m_timer.precision();
}

void ProcessTimer::regEventHandler(std::chrono::milliseconds interval, const EventHandler& handler)
{
    m_timer.regEventHandler(interval, handler);
}


}}

