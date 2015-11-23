/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-25 11:52 +0800
 *
 * Modified: 2015-03-25 11:52 +0800
 *
 * Description: 一些基本数据类型和常量定义, 全局可见
 */

#ifndef WATER_COMPONET_BASEDEF_H
#define WATER_COMPONET_BASEDEF_H

#include <stdint.h>

const uint32_t MAX_ACCOUNT_SZIE  = 32;
const uint32_t MAX_NAME_SZIE     = 32;
const uint32_t ACCOUNT_BUFF_SZIE = MAX_ACCOUNT_SZIE + 1;
const uint32_t NAME_BUFF_SZIE    = MAX_ACCOUNT_SZIE + 1;

typedef uint16_t ArraySize;
const ArraySize MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE = 3;

typedef uint64_t PKId;
typedef uint64_t ObjectId;
typedef uint32_t TplId;
typedef uint16_t TaskId;
typedef uint32_t TriggerId;


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
	sec_3	 = 3000,
	sec_5    = 5000,
    sec_15   = 15000,
    sec_30   = 30000,
    min_1    = 60000,
    min_5    = 300000,
    min_10   = 600000,
    min_15   = 900000,
};


template<typename T1, typename T2>
auto SAFE_SUB(const T1& x, const T2& y) -> decltype(x - y)
{
	return x > y ? x - y : 0;
}

template<typename T1, typename T2>
auto SAFE_DIV(const T1& x, const T2& y) -> decltype(x / y)
{
	return y != 0 ? x / y : 0;
}

template<typename T1, typename T2>
auto SAFE_MOD(const T1& x, const T2& y) -> decltype(x % y)
{
	return y != 0 ? x % y : 0;
}

template<typename T1, typename T2>
auto MAX(const T1& x, const T2& y) -> decltype(x + y)
{
	return x > y ? x : y;
}

template<typename T1, typename T2>
auto MIN(const T1& x, const T2& y) ->decltype(x + y)
{
	return x < y ? x : y;
}

#endif

