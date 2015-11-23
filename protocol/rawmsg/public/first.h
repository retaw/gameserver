/*
 * Author: zhupengfei
 *
 * Created: 2015-10-09 11:40 +0800
 *
 * Modified: 2015-10-09 11:40 +0800
 *
 * Description: 天下第一相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_FIRST_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_FIRST_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求个人报名信息
struct RequestFirstApplyInfo
{
};

//s -> c
//返回个人报名信息
struct RetFirstApplyInfo
{
	bool applied = false;
};

//c -> s
//请求报名天下第一活动
struct RequestApplyFirst
{
};

//c -> s
//请求进入天下第一副本
struct RequestIntoFistMap
{
};

//s -> c
//返回PK活动剩余时间
struct RetSpanSecOfPkEnd
{
	uint32_t sec = 0;	//剩余时间
};

//s -> c
//返回本次杀人获得的积分
struct RetGetPointNum
{
	uint32_t point;
};

//s -> c
//返回战斗结果
struct RetFirstBattleResult
{
	uint32_t killNum;
	uint32_t rewardId;
	char winnerName[NAME_BUFF_SZIE];	//name为空时代表活动未结束
};

//s -> c
//返回榜单
struct RetFirstTopList
{
	uint32_t myRank = 0;
	uint32_t myKillNum = 0;
	uint8_t myDuanweiType = 0;

	ArraySize size = 0;
	struct TopList
	{
		uint8_t rank;
		uint32_t killNum;
		char name[NAME_BUFF_SZIE];
	} data[0];
};

//c -> s
//请求天下第一获胜者信息
struct RequestFirstWinnerInfo
{
};

//s -> c
//返回天下第一获胜者信息
struct RetFirstWinnerInfo
{
	Job job;
	Sex sex;
	char name[NAME_BUFF_SZIE];
};

}

#pragma pack()


#endif
