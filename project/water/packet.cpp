#include "packet.h"
#include <cstring>

namespace water{


const Packet::SizeType Packet::MIN_SIZE;
const Packet::SizeType Packet::MAX_SIZE;
const Packet::SizeType Packet::MIN_CONTENT_SIZE;
const Packet::SizeType Packet::MAX_CONTENT_SIZE;

void Packet::setContent(const void* content, SizeType contentLen)
{
    resize(sizeof(contentLen) + contentLen);

    SizeType shift = 0;
    std::memcpy(data() + shift, &contentLen, sizeof(contentLen));
    shift += sizeof(contentLen);
    std::memcpy(data() + shift, content, contentLen);
}

};
