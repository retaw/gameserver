/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 20:34 +0800
 *
 * Description: 将一个块内存做一个包装， 使之可以直接用于序列化的buff
 */


#ifndef WATER_BASE_RAW_BUFFER_H
#define WATER_BASE_RAW_BUFFER_H

#include "exception.h"


namespace water{
namespace componet{

DEFINE_EXCEPTION(RawBufferException, ExceptionBase)

class RawBuffer
{
public:
    typedef uint32_t size_type;
    typedef uint8_t value_type;

    explicit RawBuffer(void* ptr, size_type size);

    RawBuffer() = delete;
    ~RawBuffer() = default;

    RawBuffer(const RawBuffer&) = delete;
    RawBuffer& operator=(const RawBuffer&) = delete;

    RawBuffer(RawBuffer&& other);

    RawBuffer& operator=(RawBuffer&& other);

    size_type size() const;

    void clear();

    value_type* data() const;

    value_type at(size_type pos) const;
    value_type& at(size_type pos);

    value_type operator[](size_type pos) const;
    value_type& operator[](size_type pos);

    void assign(value_type* buf, size_type bufSize);

    void swap(RawBuffer& other);

    size_type copy(value_type* buf, size_type len, size_type pos = 0) const;

    size_type append(const value_type* buf, size_type len);

private:
    size_type m_size = 0;
    value_type* m_rawBuffer = nullptr;
    size_type m_bufSize = 0;
};

}}

#endif
