#ifndef PROTOCOL_RAWMSG_PRIVATE_RELAY_MSG_H
#define PROTOCOL_RAWMSG_PRIVATE_RELAY_MSG_H

#include "../commdef.h"

#include <cstring>


#pragma pack(1)
namespace PrivateRaw{

// * -> gateway
struct RelayMsgToClient
{
    RoleId rid;
    uint32_t msgCode;
    uint32_t msgSize;
    uint8_t msgData[0];
};

}
#pragma pack()


#endif
