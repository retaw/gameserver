/*
 * Author: LiZhaojia - dantezhu@vip.qq.com
 *
 * Last modified: 2014-09-07 08:51 +0800
 *
 * Description: 环形无锁队列， 支持多生产者和多消费者， 效率比 单生产者单消费者的版本 低
 */

#ifndef WATER_BASE_M_LOCK_FREE_CIRCULAR_QUEUE_MM_HPP
#define WATER_BASE_M_LOCK_FREE_CIRCULAR_QUEUE_MM_HPP

#include <vector>
#include <atomic>

#include <iostream>
using std::cerr;
using std::endl;

namespace water{
namespace componet{

template <typename T>
class LockFreeCircularQueueMPMC final //不可作为基类
{
    struct Cell
    {
        enum class Status : uint8_t
        {
            writing,
            reading,
            empty,
            full,
        };

        Cell()
        : status(Status::empty) ,t()
        {
        }

        std::atomic<Status> status;
        T t;
    };

public:
    explicit LockFreeCircularQueueMPMC(uint64_t powArg = 16)
    : m_begin(0), m_end(0), m_maxSize(1u << (powArg < 24 ? powArg : 24)), m_data(m_maxSize)
    {
    }
    ~LockFreeCircularQueueMPMC() = default;

    //noncopyable
    LockFreeCircularQueueMPMC(const LockFreeCircularQueueMPMC&) = delete;
    LockFreeCircularQueueMPMC& operator=(const LockFreeCircularQueueMPMC&) = delete;

    bool isLockFree() const
    {
        return m_data[0].status.is_lock_free();
    }

    bool push(const T& item)
    {
        int i = 0;
        while(true) //尝试锁定队尾为可写
        {
            if(i++ == 5) //最大尝试次数
                return false;

            uint32_t oldEnd = m_end.load(std::memory_order_acquire);//队尾
            uint32_t index = realIndex(oldEnd);

            typename Cell::Status oldStatus = m_data[index].status.load(std::memory_order_acquire);
            if(oldStatus != Cell::Status::empty)
                continue;

            if(!m_data[index].status.compare_exchange_weak(oldStatus, Cell::Status::writing))
                continue;

            //队尾后移
            if(!m_end.compare_exchange_weak(oldEnd, oldEnd + 1))
            {
                m_data[index].status.store(Cell::Status::empty, std::memory_order_relaxed);
                continue;
            }

            //数据放入队列
            m_data[index].t = item;
            m_data[index].status.store(Cell::Status::full, std::memory_order_release);
            break;
        }
        return true;
    }

    bool pop(T* t)
    {
        int i = 1;
        while(true) //尝试锁定队首为可读
        {
            if(i++ == 5) //最大尝试次数
                return false;

            uint32_t oldBegin = m_begin.load(std::memory_order_acquire);//队首
            uint_fast32_t index = realIndex(oldBegin);

            typename Cell::Status oldStatus = m_data[index].status.load(std::memory_order_acquire);
            if(oldStatus != Cell::Status::full)
                continue;

            if(!m_data[index].status.compare_exchange_weak(oldStatus, Cell::Status::reading))
                continue;

            //队首后移
            if(!m_begin.compare_exchange_weak(oldBegin, oldBegin + 1))
            {
                m_data[index].status.store(Cell::Status::full, std::memory_order_relaxed);
                continue;
            }

            //取出节结点内的数据
            *t = m_data[index].t;
            m_data[index].t = T();
            m_data[index].status.store(Cell::Status::empty, std::memory_order_release);
            break;
        }
        return true;
    }

    bool empty() const
    {
        return m_data[realIndex(m_begin.load(std::memory_order_relaxed))].status.load(std::memory_order_relaxed) == Cell::Status::empty;
    }

    bool full() const
    {
        return m_data[realIndex(m_end.load(std::memory_order_relaxed))].status.load(std::memory_order_relaxed) == Cell::Status::full;
    }

    inline uint32_t maxSize() const
    {
        return m_maxSize;
    }

private:
    inline uint32_t realIndex(uint64_t index) const
    {
        return index & (maxSize() - 1);
    }

private:
    std::atomic<uint32_t> m_begin;
    std::atomic<uint32_t> m_end;
    const uint_fast32_t m_maxSize;
    std::vector<Cell> m_data;
};

}}

#endif //#ifndef WATER_BASE_CIRCULAR_QUEUE_HPP
