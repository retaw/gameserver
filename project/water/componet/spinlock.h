/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-11 17:32 +0800
 *
 * Description: 基于CAS的自旋锁
 */

#ifndef WATER_COMPONET_SPINLOCK_HPP
#define WATER_COMPONET_SPINLOCK_HPP

#include "class_helper.h"

#include <atomic>

namespace water{
namespace componet{

class Spinlock
{
    NON_COPYABLE(Spinlock);
public:
    Spinlock();
    ~Spinlock() = default;

    void lock();
    void unlock();
    bool try_lock();

    bool locked();

private:
    std::atomic<uint32_t> m_flag;
//    uint32_t m_flag;
};


}}


#endif
