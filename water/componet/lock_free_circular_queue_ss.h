/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-22 19:19 +0800
 *
 * Description: 环形无锁队列， 仅支持单生产者和单消费者并发，效率考虑，长度为2的幂，最大为65536
 */

#ifndef WATER_BASE_LOCK_FREE_CIRCULAR_QUEUE_SS_HPP
#define WATER_BASE_LOCK_FREE_CIRCULAR_QUEUE_SS_HPP

#include "class_helper.h"

#include <vector>
#include <atomic>

namespace water{
namespace componet{

template <typename T>
class LockFreeCircularQueueSPSC final //不可作为基类
{
    struct Cell
    {
        enum class Status : uint_fast32_t 
        {
            empty, 
            full, 
        };

        Cell()
        : status(Status::empty), t()
        {
        }

        std::atomic<Status> status;
        T t;
    };

public:
    TYPEDEF_PTR(LockFreeCircularQueueSPSC);

    explicit LockFreeCircularQueueSPSC(uint32_t powArg = 10) //队列长度为 pow(2, powArg)
    : m_begin(0), m_end(0), m_maxSize(1u << (powArg < 24 ? powArg : 24)), m_data(m_maxSize)
    {
    }
    ~LockFreeCircularQueueSPSC() = default;

    //noncopyable
    LockFreeCircularQueueSPSC(const LockFreeCircularQueueSPSC&) = delete;
    LockFreeCircularQueueSPSC& operator=(const LockFreeCircularQueueSPSC&) = delete;

    bool isLockFree() const
    {
        return m_data[0].status.is_lock_free();
    }

    bool push(const T& item)
    {
        const uint_fast32_t pos = realPos(m_end);
        int i = 0;
        while(true)
        {
            if(++i == 3) //最多尝试3次数依然队列满，返回失败，无法放入
                return false;

            //尾部为空，即队列非满，可以push
            if(m_data[pos].status.load(std::memory_order_acquire) == Cell::Status::empty)
                break;
        };

        //队尾后移
        m_end++;

        //向节点填入数据
        m_data[pos].t = item;

        //标记为已放入并将提交内存修改，令所有线程可见
        m_data[pos].status.store(Cell::Status::full, std::memory_order_release);
        return true;
    }

    bool pop(T* t)
    {
        const uint_fast32_t pos = realPos(m_begin);
        int i = 0;
        while(true)
        {
            if(i++ == 3) //最多尝试3次数依然队列空，返回失败，无法取出
                return false;

            //pos（头部）位置非空，即队列非空，可以pop
            if(m_data[pos].status.load(std::memory_order_acquire) == Cell::Status::full)
                break;
        }

        //队首后移
        m_begin++;

        //取出节点内的数据
        *t = m_data[pos].t;
        m_data[pos].t = T();

        //标记为已取出并将提交内存修改，令所有线程可见
        m_data[pos].status.store(Cell::Status::empty,
                                      std::memory_order_release);
        return true;
    }

    bool empty() const
    {
        return m_data[realPos(m_begin)].status.load(std::memory_order_relaxed) == Cell::Status::empty;
    }

    bool full() const
    {
        return m_data[realPos(m_end)].status.load(std::memory_order_relaxed) == Cell::Status::full;
    }

    uint64_t maxSize() const
    {
        return static_cast<uint64_t>(m_data.size());
    }

private:
    uint64_t realPos(uint64_t pos) const
    {
        return pos & (maxSize() - 1);
    }

private:
    uint_fast32_t m_begin;
    uint_fast32_t m_end;
    const uint_fast32_t m_maxSize;
    std::vector<Cell> m_data;
};

}}

#endif //#ifndef WATER_BASE_CIRCULAR_QUEUE_HPP
