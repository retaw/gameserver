#ifndef RAWMSG_PRIVATE_SUNDRY_HPP
#define RAWMSG_PRIVATE_SUNDRY_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

// world -> db
// 杂项数据存储
struct SundryToDB
{
    RoleId roleId;
    uint64_t size;
    char data[0];
};

// world -> db
// 杂项数据存储,频繁更新
struct TimerSundryToDB
{
    RoleId roleId;
    uint64_t size;
    char data[0];
};

}

#pragma pack()


#endif
