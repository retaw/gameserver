#include "packet.h"
#include <cstring>

namespace water{
namespace net{


const uint32_t Packet::MAX_SIZE;

void Packet::setMsg(const void* msg, uint32_t msgLen)
{
    resize(sizeof(msgLen) + msgLen);

    uint32_t shift = 0;
    std::memcpy(data() + shift, &msgLen, sizeof(msgLen));
    shift += sizeof(msgLen);
    std::memcpy(data() + shift, msg, msgLen);
}

}};
