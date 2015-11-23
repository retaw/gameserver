#ifndef RAWMSG_PRIVATE_COUNTER_HPP
#define RAWMSG_PRIVATE_COUNTER_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

// world -> db
struct ModifyCounterInfo
{
    RoleId roleId   = 0;
    ArraySize size  = 0;
    char buf[0];
};

}

#pragma pack()


#endif
