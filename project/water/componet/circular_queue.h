#ifndef WATER_BASE_CIRCULAR_QUEUE_HPP
#define WATER_BASE_CIRCULAR_QUEUE_HPP

#include "exception.h"

#include <vector>
#include <functional>

namespace water{
namespace componet{

DEFINE_EXCEPTION(GetFromEmptyCircularQueue, ExceptionBase)

template <typename T>
class CircularQueue
{
public:
    explicit CircularQueue(uint32_t maxSize = 10)
    : m_data(maxSize)
    {
    }

    ~CircularQueue()
    {
    }

    T& get()
    {
        if(empty())
            EXCEPTION(GetFromEmptyCircularQueue, "")

        return m_data[m_begin];
    }

    const T& get() const
    {
        if(empty())
            EXCEPTION(GetFromEmptyCircularQueue, "")

        return m_data[m_begin];
    }

    bool push(const T& item)
    {
        if(size() == maxSize())
            return false;

        m_data[m_end] = item;
        m_end = nextPos(m_end);

        ++m_size;
        return true;
    }

    void pop()
    {
        if(empty())
            return;

        m_begin = (m_begin + 1) % maxSize();
        --m_size;
    }

    inline uint32_t maxSize() const
    {
        return static_cast<uint32_t>(m_data.size());
    }

    inline uint32_t size() const
    {
        return m_size;
    }

    inline bool empty() const
    {
        return m_size == 0;
    }

    inline bool full() const
    {
        return size() == maxSize();
    }

    void traverse(std::function<bool (T&)> callback)
    {
        for(uint32_t i = m_begin, traversed = 0; traversed < size(); i = nextPos(i), ++traversed)
        {
            if(!callback(m_data[i]))
                break;
        }
    }

    void traverse(std::function<bool (const T& item)> callback) const
    {
        for(uint32_t i = m_begin, traversed = 0; traversed < size(); i = nextPos(i), ++traversed)
        {
            if(!callback(m_data[i]))
                break;
        }
    }

private:
    inline uint32_t nextPos(uint32_t pos) const
    {
        return (pos + 1) % maxSize();
    }
    

private:
    uint32_t m_begin = 0;
    uint32_t m_end = 0;
    uint32_t m_size = 0;
    std::vector<T> m_data;
};

}}

#endif //#ifndef WATER_BASE_CIRCULAR_QUEUE_HPP
