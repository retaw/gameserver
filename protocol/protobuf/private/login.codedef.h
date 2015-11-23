#ifndef PROTOCOL_PROTOBUF_PRIVATE_CODE_LOGIN_HPP
#define PROTOCOL_PROTOBUF_PRIVATE_CODE_LOGIN_HPP

#include <stdint.h>
#include "login.pb.h"

namespace PrivateProto
{
    extern const uint32_t codeCheckAccountAndToken;
    extern const CheckAccountAndToken tempCheckAccountAndToken;
    extern const uint32_t codeUserIntoWorld;
    extern const UserIntoWorld tempUserIntoWorld;
    extern const uint32_t codeRetUserIntoWorld;
    extern const RetUserIntoWorld tempRetUserIntoWorld;
    extern const uint32_t codeUserOffline;
    extern const UserOffline tempUserOffline;
}

#endif