/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-13 21:37 +0800
 *
 * Description:  和packet connection搭配，实现tcp消息的分包收发。
 *               conn的工作方式: 
 *               接收：一直尽量从socket读取packet->size()长度的数据写入packet。
 *               发送：一直尽量从packet读取packet->size()长度的数据写入socket。
 *
 *               于是packet的使用方式：
 *               1 接收：
 *               packet_connection会将packet设置一个size，然后从socket中读数据并写入packet->data(),
 *               并保证不超出packet->size()， 同时，会调用packet->addCursor()，来告知packet到底写
 *               了多少数据。
 *               对于发送和接收方都约定好的定长协议，上面的已经ok了。
 *               对于变长协议，针对不同的变长协议格式要实现一个packet的派生类，在开始接收时，
 *               预留一个包头大小的size，然后通过override addCursor()函数，
 *               可以依据收到的包头的内容来动态调整packet的size，
 *               从而实现依据包头信息动态调整接收包的大小
 *
 *               2 发送：
 *               直接将数据写入packet->data()中，设定好size，发送即可
 */

#ifndef WATER_NET_PACKET_H
#define WATER_NET_PACKET_H

#include <vector>

#include "componet/class_helper.h"

#include "net_exception.h"

namespace water{
namespace net{

DEFINE_EXCEPTION(PacketCursorOutOfRange, net::NetException);

class Packet
{
public:
    typedef uint32_t SizeType;
    static const SizeType HEAD_SIZE = sizeof(SizeType);
    static const SizeType MIN_SIZE = sizeof(HEAD_SIZE);
    static const SizeType MAX_SIZE = 65535;
    static const SizeType MIN_CONTENT_SIZE = 0;
    static const SizeType MAX_CONTENT_SIZE = MAX_SIZE - MIN_SIZE;

    //作为发送接收缓冲时的类型
    enum class BuffType : uint8_t  {send, recv};

public:
    TYPEDEF_PTR(Packet);
    CREATE_FUN_MAKE(Packet);

    explicit Packet();
    explicit Packet(SizeType size);
    explicit Packet(const void* data, SizeType size);

    //得到内部buff的地址
    uint8_t* data();
    const uint8_t* data() const;

    //填充数据
    void assign(const void* data, SizeType size);

    //把数据拷出
    SizeType copy(void* buff, SizeType maxSize);

    //重新设置buff的长度
    void resize(SizeType size);
    //得到buff的长度
    SizeType size() const;

public://socket收发使用的接口
    void initBuffType(BuffType type);
    //得到已处理标记的位置
    SizeType getCursor() const;
    //后移已处理标记
    virtual void addCursor(SizeType add);

protected:
    std::vector<uint8_t> m_buf;

    //在发送和接收消息时使用, 指向未处理的数据
    SizeType m_cursor;
    BuffType m_type;
};

}}

#endif

