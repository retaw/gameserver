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

#include "../componet/class_helper.h"

namespace water{
namespace net{



class Packet final : public std::vector<uint8_t>
{
public:
    TYPEDEF_PTR(Packet);
    CREATE_FUN_MAKE(Packet);

    static const uint32_t MIN_SIZE = 4;
    static const uint32_t MAX_SIZE = 65535;
    static const uint32_t MAX_MSG_SIZE = 65531;

public:
    void setMsg(const void* msg, uint32_t msgLen);

    const uint8_t* getMsg() const
    {
        return data();
    }

    uint32_t msgSize() const
    {
        return *reinterpret_cast<const uint32_t*>(data());
    }
};


}};


#endif
