/*
 * Description: 日志buffer，一个buffer可存多条日志
 */
#ifndef WATER_BASE_LOG_BUFFER_HPP
#define WATER_BASE_LOG_BUFFER_HPP

#include <stdint.h>
#include <string.h>

namespace water {
namespace componet{

const uint32_t BIG_BUFFER_SIZE = 6400 * 1024 * 10;  //64K
const uint32_t REMAIN_NOTIFY_BUFFER_SIZE = 6400 * 1024;  //6K  //剩余6K空间的时候就要通知读队列了

class WriterBuffer
{
public:
    WriterBuffer(uint32_t size=BIG_BUFFER_SIZE) 
    : m_data(new char[size]),
    m_cur(m_data),
    m_size(size)
    {
    }
    ~WriterBuffer()
    {
        delete [] m_data;
    }

    void put(const char* msg, uint32_t len)
    {
        if (remain() > len)
        {
            memcpy(m_cur, msg, len);
            m_cur += len;
        }
    }

    const bool empty() const
    {
        return this->length() == 0;
    }

    const char* data() { return m_data; }

    uint32_t length() const { return m_cur - m_data; }

    uint32_t remain() const { return static_cast<uint32_t>(end() - m_cur); }

    void reset() { m_cur = m_data; }

private:
    const char* end() const { return m_data + m_size; }

private:
    char* m_data;
    char* m_cur;
    uint32_t m_size;
};

}}
#endif //#define WATER_BASE_LOG_BUFFER_HPP

