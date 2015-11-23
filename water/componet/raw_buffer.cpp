#include "raw_buffer.h"

#include <cstring>


namespace water{
namespace componet{

RawBuffer::RawBuffer(void* ptr, size_type size)
: m_size(0), m_rawBuffer(reinterpret_cast<value_type*>(ptr)), m_bufSize(size) 
{
}

RawBuffer::RawBuffer(RawBuffer&& other)
{
    m_bufSize = other.m_bufSize;
    m_rawBuffer= other.m_rawBuffer;

    other.clear();
}

RawBuffer& RawBuffer::operator=(RawBuffer&& other)
{
    m_bufSize = other.m_bufSize;
    m_rawBuffer= other.m_rawBuffer;

    other.clear();

    return *this;
}

RawBuffer::size_type RawBuffer::size() const
{
    return m_size;
}

void RawBuffer::clear()
{
    m_size = 0;
}

RawBuffer::value_type* RawBuffer::data() const
{
    return m_rawBuffer;
}

RawBuffer::value_type RawBuffer::at(size_type pos) const
{
    if(pos >= m_bufSize)
        EXCEPTION(RawBufferException, "RawBuffer::at:const, out of range");
    return m_rawBuffer[pos];
}

RawBuffer::value_type& RawBuffer::at(size_type pos)
{
    if(pos >= m_bufSize)
        EXCEPTION(RawBufferException, "RawBuffer::at, out of range");
    return m_rawBuffer[pos];
}

RawBuffer::value_type RawBuffer::operator[](size_type pos) const
{
    return m_rawBuffer[pos];
}

RawBuffer::value_type& RawBuffer::operator[](size_type pos)
{
    return m_rawBuffer[pos];
}

void RawBuffer::assign(value_type* buf, size_type bufSize)
{
    m_rawBuffer = buf;
    m_bufSize = bufSize;
    m_size = 0;
}

void RawBuffer::swap(RawBuffer& other)
{
    size_type tempS = m_bufSize;
    m_bufSize = other.m_bufSize;
    other.m_bufSize = tempS;

    value_type* tempB = m_rawBuffer;
    m_rawBuffer = other.m_rawBuffer;
    other.m_rawBuffer = tempB;
}

RawBuffer::size_type RawBuffer::copy(value_type* buf, size_type len, size_type pos/* = 0*/) const
{
    if(m_size < pos)
        return 0;

    if(m_size - pos < len)
        EXCEPTION(RawBufferException, "RawBuffer::copy, 目标buffer空间不足");

    size_type ret = m_size - pos < len ? m_size - pos : len;
    std::memcpy(buf, m_rawBuffer + pos, ret);
    return ret;
}

RawBuffer::size_type RawBuffer::append(const value_type* buf, size_type len)
{
    if(m_bufSize - m_size < len)
        EXCEPTION(RawBufferException, "RawBuffer::append, 内部buffer空间不足");

    size_type ret = m_bufSize - m_size < len ? m_size - m_bufSize : len;
    std::memcpy(m_rawBuffer + m_size, buf, ret);
    m_size += ret;
    return ret;
}

}}
