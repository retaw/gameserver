/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-24 11:18 +0800
 *
 * Description: 定时器线程
 */

#ifndef WATER_PROCESS_PROCESSTIMER_H
#define WATER_PROCESS_PROCESSTIMER_H

#include "componet/datetime.h"
#include "componet/event.h"
#include "componet/spinlock.h"
#include "componet/timer.h"

#include "process_thread.h"

#include <functional>

namespace water{
namespace process{

class ProcessTimer : public ProcessThread
{
public:
    using EventHandler = std::function<void (const componet::TimePoint&)>;

    ProcessTimer();
    ~ProcessTimer() = default;

    bool exec() override;

    bool isSuspend() const;
    void suspend();
    void resume();

    int64_t precision() const;

    //注册一个触发间隔
    void regEventHandler(std::chrono::milliseconds interval, const EventHandler& handle);
public:
    componet::Event<void (ProcessTimer*)> e_stop;

private:
    componet::Timer m_timer;
    volatile bool m_suspend; //只有进程的主线程会修改这个值，就不用atomic了
};

}}

#endif
