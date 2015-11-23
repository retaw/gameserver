#include "process_thread.h"

#include <functional>
#include <thread>

namespace water{
namespace process{


ProcessThread::ProcessThread()
: m_switch(static_cast<uint8_t>(Switch::off))
{
}

void ProcessThread::run()
{
    m_switch.store(static_cast<uint8_t>(Switch::on), std::memory_order_release);
    std::packaged_task<bool()> task(std::bind(&ProcessThread::exec, this));
    m_threadReturnValue = task.get_future();
    std::thread(std::move(task)).detach();

//    m_threadReturnValue = std::async(std::lanch::async, std::mem_fn(&ProcessThread::exec), this);
}

void ProcessThread::stop()
{
    uint8_t s = static_cast<uint8_t>(Switch::on);
    if(m_switch.compare_exchange_strong(s, static_cast<uint8_t>(Switch::off), std::memory_order_release))
        e_close(this);
}

bool ProcessThread::wait()
{
    return m_threadReturnValue.get();
}

ProcessThread::WaitRet ProcessThread::wait(bool* returnValue, std::chrono::milliseconds timeout)
{
    if(m_threadReturnValue.wait_for(timeout) != std::future_status::ready)
        return WaitRet::timeout;

    *returnValue = m_threadReturnValue.get();
    return WaitRet::done;
}

bool ProcessThread::checkSwitch() const
{
    return m_switch.load(std::memory_order_relaxed) == static_cast<uint8_t>(Switch::on);
}


}}

