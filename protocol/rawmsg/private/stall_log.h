#ifndef PROTOCOL_RAWMSG_PRIVATE_STALL_LOG_HPP
#define PROTOCOL_RAWMSG_PRIVATE_STALL_LOG_HPP

#include "water/common/roledef.h"

#pragma pack(1)
namespace PrivateRaw{

//world -> db
//保存出售记录
struct SaveStallLog
{
    RoleId roleId;
    ArraySize size = 0;
    char logs[0];
};

//world -> db
//请求出售记录
struct WorldReqStallLog
{
    RoleId roleId;
};

//db -> world
//返回出售记录
struct DBRetStallLog
{
    RoleId roleId;
    ArraySize size = 0;
    char logs[0];
};

}
#pragma pack()
#endif

