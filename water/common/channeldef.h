/*
 * Author: zhupengfei
 *
 * Created: 2015-06-10 15:39 +0800
 *
 * Modified: 2015-06-10 15:39 +0800
 *
 * Description: 物品相关的一些类型和常量定义
 */


#ifndef WATER_COMMON_CHANNELDEF_HPP
#define WATER_COMMON_CHANNELDEF_HPP

#include <stdint.h>

enum class ChannelType : uint8_t
{
	global				= 1,    //世界
	scene				= 2,    //同场景(附近)
	gang				= 3,    //帮会
	team				= 4,    //队伍
	system				= 5,	//系统(屏幕左下,显示系统针对个人消息反馈，及全世界公告)
	private_chat		= 6,    //私聊
	trumpet				= 7,    //喇叭
	system_msgbox		= 8,	//系统消息框
	screen_top			= 9,    //屏幕顶部(显示系统公告，全世界)
	screen_middle		= 10,   //屏幕中部(显示系统公告, 全世界)
	screen_right_down	= 11,	//屏幕右下(显示系统针对个人消息反馈)
};


#endif
