/*
 * Author: zhupengfei
 *
 * Created: 2015-07-22 14:05 +0800
 *
 * Modified: 2015-07-22 14:05 +0800
 *
 * Description: 官职相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_GUANZHI_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_GUANZHI_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求官职奖励状态
struct RequestGuanzhiRewardState
{
};

//s -> c
//返回官职奖励状态
struct RetGuanzhiRewardState
{
	Reward dailyReward = Reward::none;	//0没有奖励	1可领取	2已领取
};

//c -> s
//请求领取每日奖励
struct RequestGetGuanzhiReward
{
};

//c -> s
//请求晋升官职
struct RequestGuanzhiLevelUp
{
};

//s -> c
//返回官职晋升成功
struct RetGuanzhiLevelUpSucess
{
	bool sucess = false;
};

}

#pragma pack()


#endif
