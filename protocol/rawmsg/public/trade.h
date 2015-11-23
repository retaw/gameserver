/*
 * Author: zhupengfei
 *
 * Created: 2015-10-19 14:30 +0800
 *
 * Modified: 2015-10-19 14:30 +0800
 *
 * Description: 面对面交易相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_TRADE_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_TRADE_MSG_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求交易
struct RequestTrade
{
	RoleId destRoleId;
};  

//s -> c
//通知玩家收到交易申请
struct NotifyReceiveTradeAsk
{
};

//c -> s
//请求交易申请列表
struct RequestTradeAskList
{
};

//s -> c
//返回交易申请列表
struct RetTradeAskList
{
	ArraySize size = 0;

	struct AskerItem
	{
		RoleId askerId;
		char askerName[NAME_BUFF_SZIE];
		uint32_t level;
	} data[0];
};

//c -> s
//拒绝交易请求
struct RefuseTrade
{
	RoleId askerId;
};

//c -> s
//拒绝所有交易请求
struct RefuseAllTrade
{
};

//c -> s
//同意交易请求
struct AgreeTrade
{
	RoleId askerId;
};

//s -> c
//返回同意交易请求,双方可进行交易
struct RetAgreeTradeToEachOther
{
};

//c -> s
//请求输入交易货币
struct RequestInputTradeMoney
{
	MoneyType type;
	uint32_t num;
};

//c -> s
//请求放入交易物品
struct RequestPutTradeObj
{
	uint16_t cell;
};

//c -> s
//请求移除交易物品
struct RequestRemoveTradeObj
{
	uint16_t cell;
};

//c -> s 
//请求交易信息
struct RequestTradeInfo
{
};

struct TradeInfo
{
	RoleId roleId;
	char name[NAME_BUFF_SZIE];
	uint32_t yuanbao;
	uint32_t jinbi;
	uint8_t tradeState = 0;	//1解锁 2锁定 3确认

	ArraySize size = 0;
	struct TradeObj
	{
		TplId tplId;
		uint16_t num;
		uint16_t cell;
		uint32_t skillId;
		uint8_t strongLevel;
		uint8_t luckyLevel;
	} data[0];
};

//s -> c
//返回双方的交易信息
struct RetTradeInfo
{
	TradeInfo askerInfo;
	TradeInfo acceptorInfo;
};

//c -> s
//请求锁定交易
struct RequestLockTrade
{
};

//c -> s
//请求解锁交易
struct RequestUnlockTrade
{
};

//c -> s
//请求取消交易
struct RequestCancelTrade
{
};

//s -> c
//通知玩家交易取消
struct NotifyPlayerTradeCancel
{
};

//c -> s
//确认交易
struct ConfirmTrade
{
};

//s -> c
//通知玩家交易成功
struct NotifyPlayerTradeSucess
{
};


}

#pragma pack()

#endif


