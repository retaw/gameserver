/*
 *
 * 功能模的相关定义
 *
 */

#ifndef WATER_COMMON_FUNC_DEF_H
#define WATER_COMMON_FUNC_DEF_H

#include "commdef.h"


enum class FunctionItem : uint8_t
{
    world_boss      = 10,    //世界boss
};


enum class IconState : uint8_t
{
    no      = 0,    //没有奖励可领取
    yes     = 1,    //有奖励可领取
};


#endif
