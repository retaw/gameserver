/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-27 16:44 +0800
 *
 * Description:  Process的线程组件的统一启动和终止接口，实现线程对象的建立和执行的分离
                 此对象本身，仅可被一个线程使用
                 非通用组件，专为class Process设计，不推荐在其他地方复用
 */

#ifndef WATER_PROCESS_THREAD_H
#define WATER_PROCESS_THREAD_H

#include "componet/event.h"
#include "componet/class_helper.h"

#include <atomic>
#include <future>
#include <chrono>

namespace water{
namespace process{

class ProcessThread
{
public:
    TYPEDEF_PTR(ProcessThread)

    NON_COPYABLE(ProcessThread);

    ProcessThread();
    virtual ~ProcessThread() = default;

public:
    void run();
    void stop();

    //阻塞直到线程执行函数exec结束，返回exec的返回值，exec抛出的异常将由本函数抛出
    bool wait();
    //上述函数带超时的版本
    enum class WaitRet {timeout, done};
    WaitRet wait(bool* returnValue, std::chrono::milliseconds timeout);

public:
    componet::Event<void (ProcessThread*)> e_close;

protected:
    bool checkSwitch() const;
private:
    //线程函数，返回true表示正常退出，返回false表示异常退出
    virtual bool exec() = 0;
    
private:
    std::future<bool> m_threadReturnValue;
    enum class Switch : uint8_t {on, off};
    std::atomic<uint8_t> m_switch; //gcc开启优化时, atd::atomic不支持用枚举类型特化
};

}}

#endif
