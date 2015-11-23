#include "tcp_packet.h"
#include <cstring>

namespace water{
namespace process{

TcpPacket::TcpPacket()
: Packet(sizeof(SizeType))
{
}

void TcpPacket::setContent(const void* content, SizeType contentSize)
{
    const SizeType packetSize = sizeof(SizeType) + contentSize;
    resize(packetSize);
    std::memcpy(data(), &packetSize, sizeof(contentSize));
    std::memcpy(data() + sizeof(contentSize), content, contentSize);
}

void* TcpPacket::content()
{
    if(m_type == Packet::BuffType::send && m_cursor != size())
        return nullptr;

    if(size() < sizeof(SizeType))
        return nullptr;

    return data() + sizeof(SizeType);
}

TcpPacket::SizeType TcpPacket::contentSize() const
{
    if(m_type == Packet::BuffType::send && m_cursor != size())
        return 0;

    if(size() < sizeof(SizeType))
        return 0;

    return size() - sizeof(SizeType);
}


void TcpPacket::addCursor(SizeType add)
{
    m_cursor += add;
    //当收到一个SizeType长度的数据时, 即得知了这个包的实际总长度，
    //把包大小预留为实际大小，以便继续接收
    if(m_type == Packet::BuffType::recv && m_cursor == sizeof(SizeType))
    {
        SizeType packetSize = *reinterpret_cast<const SizeType*>(data());
        resize(packetSize);
    }
}

}}
