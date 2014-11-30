/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-13 21:37 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_PACKET_H
#define WATER_NET_PACKET_H

#include <vector>

#include "componet/class_helper.h"

namespace water{
namespace net{


class Packet final : public std::vector<uint8_t>
{
public:
    TYPEDEF_PTR(Packet);
    CREATE_FUN_MAKE(Packet);

    typedef uint32_t SizeType;
    static const SizeType HEAD_SIZE = sizeof(SizeType);
    static const SizeType MIN_SIZE = sizeof(HEAD_SIZE);
    static const SizeType MAX_SIZE = 65535;
    static const SizeType MIN_CONTENT_SIZE = 0;
    static const SizeType MAX_CONTENT_SIZE = MAX_SIZE - MIN_SIZE;


public:
    void setContent(const void* content, SizeType contentSize);
    void setContentSize(SizeType size);
    const uint8_t* getContent() const;
    SizeType getContentSize() const;

};


}}


#endif
