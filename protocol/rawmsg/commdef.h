#ifndef PROTOCOL_RAWMSG_COMMODDEF_H
#define PROTOCOL_RAWMSG_COMMODDEF_H

#include <cstring>
#include <cstdint>

const uint32_t MAX_ACCOUNT_SZIE  = 32; 
const uint32_t MAX_NAME_SZIE     = 32; 
const uint32_t ACCOUNT_BUFF_SZIE = MAX_ACCOUNT_SZIE + 1;
const uint32_t NAME_BUFF_SZIE    = MAX_ACCOUNT_SZIE + 1;

typedef uint16_t ArraySize;
const ArraySize MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE = 3;

typedef uint64_t RoleId;
typedef uint64_t LoginId;
typedef uint32_t TplId;
typedef uint16_t TaskId;


enum class LoginRetCode : uint8_t
{   
    successful          = 0, //成功
    failed              = 1, //未知原因的失败
    invalidRoleName     = 2, //非法角色名
    conflictedRoleName  = 3, //角色名冲突
    beReplaced          = 4, //被挤下线
};

enum class StdInterval
{
    infinite = 0,   //即无限长时间
    msec_100 = 100,
    msec_300 = 300,
    msec_500 = 500,
    sec_1    = 1000,
    sec_3    = 3000,
    sec_5    = 5000,
    sec_15   = 15000,
    sec_30   = 30000,
    min_1    = 60000,
    min_5    = 300000,
    min_10   = 600000,
    min_15   = 900000,
};

//职业
enum class Job : uint8_t
{
    none     = 0,   //不限制职业
    warrior  = 1,   //战士
    magician = 2,   //法师
};

//性别
enum class Sex : uint8_t
{
    none   = 0,     //不限制性别
    male   = 1,
    female = 2,
};

//基本角色信息
struct RoleBasicData
{
    RoleBasicData()
    {
        std::memset(this, 0, sizeof(*this));
    }   
    RoleId id;
    char name[NAME_BUFF_SZIE];
    Job job;
    Sex sex; 
};

#endif
