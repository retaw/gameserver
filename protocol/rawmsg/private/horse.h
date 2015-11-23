#ifndef RAWMSG_PRIVATE_HORSE_HPP
#define RAWMSG_PRIVATE_HORSE_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

// world -> db
struct ModifyHorseData
{
    RoleId roleId   = 0;
    uint16_t size  = 0;
    uint8_t buf[0]; //序列化数据
};

}

#pragma pack()


#endif
