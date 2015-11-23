#ifndef RAWMSG_PRIVATE_BUFF_HPP
#define RAWMSG_PRIVATE_BUFF_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

// world -> db
struct ModifyBuffData
{
    RoleId roleId   = 0;
    Job job;
    Job ownerJob;
    uint8_t sceneItem = 1;
    uint8_t ownerSceneItemType = 1;
    ModifyType modifyType;
    ArraySize size  = 0;
    BuffData data[0];
};

}

#pragma pack()


#endif
