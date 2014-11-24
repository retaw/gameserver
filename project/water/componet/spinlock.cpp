#include "spinlock.h"

#include <thread>


namespace water{
namespace componet{

Spinlock::Spinlock()
    :m_flag(0)
{
}

void Spinlock::lock()
{
    uint32_t flag = 0;
    while(!m_flag.compare_exchange_weak(flag, 1, std::memory_order_relaxed))
    {
        std::this_thread::yield();
        flag = 0;
    }
    /*
    while(!__atomic_compare_exchange_n(&m_flag, &flag, 1, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED))
    {
        std::this_thread::yield();
        flag = 0;
    }
    */
}

void Spinlock::unlock()
{
    m_flag.store(0, std::memory_order_relaxed);
//    __atomic_store_n(&m_flag, 0, __ATOMIC_RELAXED);
}

bool Spinlock::try_lock()
{
    uint32_t flag = 0;
    return m_flag.compare_exchange_strong(flag, 1, std::memory_order_relaxed);
//    return __atomic_compare_exchange_n(&m_flag, &flag, 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}

bool Spinlock::locked()
{
    return m_flag.load(std::memory_order_relaxed) == 1;
}

}}
