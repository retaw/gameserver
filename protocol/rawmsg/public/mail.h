#ifndef PROTOCOL_RAWMSG_PUBLIC_MAIL_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_MAIL_MSG_H

#pragma pack(1)

#include "water/common/maildef.h"

namespace PublicRaw{

//c -> s
//请求邮件列表
struct RequestMailList
{
};

//s -> c
//返回邮件列表
struct RetMailList
{
    ArraySize mailSize = 0;
    struct MailData
    {
        uint32_t index;
        char title[MAX_MAIL_TITLE_SIZE];
        uint8_t state;
        uint32_t sendtime;
    } data[0];
};

//s -> c
//通知端有新的未读取邮件
struct NotifyNewMail
{
};

//c -> s
//请求查看某个邮件详细信息
struct RequestMailDetailInfo
{
    uint32_t index;
};

//s -> c
//返回邮件详细信息
struct RetMailDetailInfo
{
    uint32_t index;
    char title[MAX_MAIL_TITLE_SIZE];
    char text[MAX_MAIL_TEXT_SIZE];
    uint8_t state;
    uint32_t sendtime;
    struct
    {
        uint32_t id;
        uint32_t num;
        uint8_t bind;
    } obj[MAX_MAIL_OBJ_NUM];
};

//c -> s
//请求提取邮件附件(支持批量提取)
struct RequestGetMailObj
{
    ArraySize size = 0;
    uint32_t index[0];
};

//s -> c
//刷新邮件状态
struct RefreshMailState
{
    ArraySize size = 0;
    struct MailStateInfo
    {
        uint32_t mailIndex;
        uint8_t state;
    } data[0];
};

//c -> s
//请求删除邮件(可以批量删除)
struct RequestDeleteMail
{
    ArraySize size = 0;
    uint32_t index[0];
};

}

#pragma pack()

#endif

