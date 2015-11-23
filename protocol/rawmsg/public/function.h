/*
 * 功能模块通用消息定义
 */

#ifndef PROTOCOL_RAWMGS_PUBLIC_FUNCTION_HPP
#define PROTOCOL_RAWMGS_PUBLIC_FUNCTION_HPP

#include "water/common/funcdef.h"

#pragma pack(1)

namespace PublicRaw{

//s -> c
//下发已开放的功能图标
struct RetOpenedFunctionList
{
};

//s -> c
//新开放功能通知
struct NotifyNewFunctionOpen
{
};

//s -> c
//刷新单个功能图标状态
struct RefreshFunctionIconState
{
    FunctionItem item;
    IconState state;
};

//s -> c
//返回所有功能图标状态
struct RetAllFunctionIconState
{
    ArraySize size = 0;
    struct FunctionIconInfo
    {
        FunctionItem item;
        IconState state;
    } data[0];
};


}

#pragma pack()

#endif

