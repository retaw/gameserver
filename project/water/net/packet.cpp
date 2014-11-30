#include "packet.h"
#include <cstring>

namespace water{
namespace net{

const Packet::SizeType Packet::MIN_SIZE;
const Packet::SizeType Packet::MAX_SIZE;
const Packet::SizeType Packet::MIN_CONTENT_SIZE;
const Packet::SizeType Packet::MAX_CONTENT_SIZE;
const Packet::SizeType Packet::HEAD_SIZE;

void Packet::setContent(const void* content, SizeType contentSize)
{
    setContentSize(contentSize);
    std::memcpy(data() + sizeof(contentSize), content, contentSize);
}

void Packet::setContentSize(SizeType size)
{
    resize(sizeof(size) + size);
    *reinterpret_cast<SizeType*>(data()) = size;
}

const uint8_t* Packet::getContent() const
{
    return data() + sizeof(SizeType);
}

Packet::SizeType Packet::getContentSize() const
{
    return *reinterpret_cast<const SizeType*>(data());
}

}}
