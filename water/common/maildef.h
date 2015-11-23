#ifndef WATER_COMPONET_MAILDEF_H
#define WATER_COMPONET_MAILDEF_H


#include "objdef.h"

//单个邮件最大的附件数量
const uint8_t MAX_MAIL_OBJ_NUM      = 5;
//邮件标题最多字节
const uint16_t MAX_MAIL_TITLE_SIZE  = 64;
//邮件内容最多字节
const uint16_t MAX_MAIL_TEXT_SIZE   = 512;

//每个角色邮件存储数量上限
const uint16_t MAX_MAIL_DB_NUM      = 50;


enum class MailState : uint8_t
{
    unopen      = 1,    //未读取
    opened      = 2,    //已读取
    existobj    = 4,    //有附件可以提取
};

#endif

