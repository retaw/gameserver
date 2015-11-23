/*
 * Author: zhupengfei
 *
 * Created: 2015-09-25 11:10 +0800
 *
 * Modified: 2015-09-25 11:10 +0800
 *
 * Description: 活动相关的一些定义
 */


#ifndef WATER_COMMON_ACTIONDEF_HPP
#define WATER_COMMON_ACTIONDEF_HPP

#include "processes/world/pkdef.h"

#include <stdint.h>

//活动状态
enum class ActionState : uint8_t
{
	begin	= 1,
	end		= 2,
};

//活动类型
const uint8_t TOTAL_ACTION_NUM = 6;   //活动总数量
enum class ActionType : uint8_t
{
	none			= 0,	//表示非法值
	exp_area		= 1,	//自动经验区
	bubble_point	= 2,	//激情泡点副本
	first_apply		= 3,	//天下第一活动报名
	first_ready		= 4,	//天下第一活动准备
    world_boss      = 5,    //世界boss
    shabake         = 6,    //沙巴克
};


#endif
