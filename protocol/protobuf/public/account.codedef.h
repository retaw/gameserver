#ifndef PROTOCOL_PROTOBUF_PUBLIC_CODE_ACCOUNT_HPP
#define PROTOCOL_PROTOBUF_PUBLIC_CODE_ACCOUNT_HPP

#include <stdint.h>
#include "account.pb.h"

namespace PublicProto
{
    extern const uint32_t codeC_FastSignUp;
    extern const C_FastSignUp tempC_FastSignUp;
    extern const uint32_t codeC_SignUpWithAccount;
    extern const C_SignUpWithAccount tempC_SignUpWithAccount;
    extern const uint32_t codeS_SignUpRet;
    extern const S_SignUpRet tempS_SignUpRet;
    extern const uint32_t codeC_LoginByAccount;
    extern const C_LoginByAccount tempC_LoginByAccount;
    extern const uint32_t codeC_FastLogin;
    extern const C_FastLogin tempC_FastLogin;
    extern const uint32_t codeS_LoginRet;
    extern const S_LoginRet tempS_LoginRet;
    extern const uint32_t codeC_SelectPlatform;
    extern const C_SelectPlatform tempC_SelectPlatform;
    extern const uint32_t codeS_SelectPlatformRet;
    extern const S_SelectPlatformRet tempS_SelectPlatformRet;
}

#endif