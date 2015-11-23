#ifndef RAWMSG_PRIVATE_TASK_HPP
#define RAWMSG_PRIVATE_TASK_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

//world -> db
struct UpdateAllTaskInfoToDB
{
    RoleId roleId;
    ArraySize size = 0;
    uint8_t buf[0]; //序列化数据
};

struct UpdateFactionTaskInfo
{
    RoleId roleId;
    ArraySize size;
    char data[0];
};

}

#pragma pack()


#endif
