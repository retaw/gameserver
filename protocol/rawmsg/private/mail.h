#ifndef RAWMSG_PRIVATE_MAIL_HPP
#define RAWMSG_PRIVATE_MAIL_HPP

#include "water/common/maildef.h"
#include "water/common/commdef.h"
#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

// world -> db
// 通过name发送邮件
struct WriteMailByName
{
    char receiver[MAX_NAME_SZIE];       //接收者
    MailInfo info;
};


// world -> db
// 通过id发送邮件
struct WriteMailById
{
    RoleId roleId;
    MailInfo info;
};


//world -> db
struct UpdateMail
{
    RoleId roleId;
    ArraySize size = 0;
    struct MailModify
    {
        uint32_t mailIndex;
        uint8_t state;
    } data[0];
};

//world -> db
struct EraseMail
{
    RoleId roleId;
    ArraySize size = 0;
    uint32_t mailIndex[0];
};

//db -> world
//在线玩家有新接收的邮件时需要db同步到world
struct NewMailToWorld
{
    RoleId roleId;
    MailInfo info;
};

//db -> world
//删除邮件(超出上限, 服务器主动删除)
struct ServerEraseMail
{
    RoleId roleId;
};

}

#pragma pack()


#endif
