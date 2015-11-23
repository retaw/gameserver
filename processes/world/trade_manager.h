/*
 * Author: zhupengfei
 *
 * Created: 2015-10-20 10:50 +0800
 *
 * Modified: 2015-10-20 10:50 +0800
 *
 * Description: 处理面对面交易相关消息及逻辑
 */

#ifndef PROCESS_WORLD_TRADE_MANAGER_HPP
#define PROCESS_WORLD_TRADE_MANAGER_HPP

#include "object.h"

#include "water/common/roledef.h"
#include "water/componet/datetime.h"
#include "water/componet/class_helper.h"

#include <cstdint>
#include <list>
#include <map>
#include <unordered_map>

namespace world{

using namespace water;
using namespace water::componet;
using water::componet::TimePoint;

enum class TradeState : uint8_t
{
	none	= 0,
	unlock	= 1,
	lock	= 2,
	confim	= 3,
	cancle	= 4,
};

class Trade : public std::enable_shared_from_this<Trade>
{
public:
	TYPEDEF_PTR(Trade);

public:
	explicit Trade(uint64_t tradeId, RoleId askerId, RoleId acceptorId);

	~Trade() = default;

public:
	uint64_t getTadeId() const;
	bool isRoleInTheTrade(RoleId roleId) const;
	TradeState getTradeState(RoleId roleId) const;
	bool needErase() const;
	
public:
	void cancleTrade(RoleId roleId);

private:
	void setTradeState(RoleId roleId, TradeState state);
	void markErase();

public:
	void requestTradeInfo(RoleId roleId);
	void requestInputMoney(RoleId roleId, MoneyType type, uint32_t num);
	void requestPutObj(RoleId roleId, uint16_t cell);
	void requestRemoveObj(RoleId roleId, uint16_t cell);

	void requestLockTrade(RoleId roleId);
	void requestUnlockTrade(RoleId roleId);
	void requestCancelTrade(RoleId roleId);
	void requestConfimTrade(RoleId roleId);

private:
	bool tradeObj();
	bool tradeMoney();

	bool isBothPlayerConfim();
	bool checkTradeMoney();
	bool checkPutTradeObj();

	bool getTradeObj(RoleId roleId, std::vector<ObjItem>& tradeObjVec);
	RoleId getTheOtherRoleId(RoleId roleId);
	uint32_t getTradeMoneyNum(RoleId roleId, MoneyType type);
	void cancelFixCellOfAll();
	
	void fillTradeInfo(RoleId roleId, std::vector<uint8_t>* buf);

private:
	void sendTradeInfoToAll();
	void sendTradeInfoToMe(RoleId roleId);

	void sendTradeCancelToAll();
	void sendTradeSucessToAll();

private:
	uint64_t m_tradeId;
	RoleId m_askerId;
	RoleId m_acceptorId;
	bool m_needErase;

private:
	struct TradeObjItem
	{
		uint16_t cell; 
		TplId tplId;
		uint16_t num;
		uint32_t skillId;
		uint8_t strongLevel;
		uint8_t luckyLevel;
	};

	std::unordered_map<RoleId, std::vector<TradeObjItem> > m_tradeObjMap; 
	std::unordered_map<RoleId, std::map<MoneyType, uint32_t> > m_tradeMoneyMap;
	std::unordered_map<RoleId, TradeState> m_stateMap;
};



class TradeManager
{
public:
	TradeManager();
	~TradeManager() = default;
    static TradeManager& me();
private:
	static TradeManager m_me;

public:
    void regMsgHandler();

private:
	//请求交易
	void clientmsg_RequestTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求交易申请列表
	void clientmsg_RequestTradeAskList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//拒绝交易请求
	void clientmsg_RefuseTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//拒绝所有交易请求
	void clientmsg_RefuseAllTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//同意交易请求
	void clientmsg_AgreeTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求交易信息
	void clientmsg_RequestTradeInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求输入货币
	void clientmsg_RequestInputTradeMoney(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求放入交易物品
	void clientmsg_RequestPutTradeObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求移除交易物品
	void clientmsg_RequestRemoveTradeObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求锁定交易
	void clientmsg_RequestLockTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求解锁交易
	void clientmsg_RequestUnlockTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求取消交易
	void clientmsg_RequestCancelTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//确认交易
	void clientmsg_ConfirmTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	bool checkInSameScreen(RoleId sourceId, RoleId destId);
	bool checkTradeOverTime(RoleId askerId, RoleId acceptorId);
	void eraseTradeAsk(RoleId askerId);

	bool updateAcceptorAskList(RoleId askerId, RoleId acceptorId);
	Trade::Ptr getTradePtrByRoleId(RoleId roleId);

private:
	void sendTradeAskToAcceptor(RoleId askerId, RoleId acceptorId);
	void sendAskListToMe(RoleId roleId);
	void sendAgreeTradeToEachOther(RoleId askerId, RoleId acceptorId);	

public:
	void cancleTrade(RoleId roleId);

	void regTimer();

private:
    void timerLoop(StdInterval interval, const componet::TimePoint& now);
	void timer();
	

private:
	struct DestItem
	{
		RoleId destRoleId;
		TimePoint deleteTime;
	};
	std::unordered_map<RoleId, std::vector<DestItem> > m_askerMap;	
	std::unordered_map<RoleId, TimePoint> m_askerTimeMap;

	struct AskerItem
	{
		RoleId askerId;
		std::string askerName;
		uint32_t level;
		TimePoint askTime;
	};
	std::unordered_map<RoleId, std::list<AskerItem> > m_acceptorMap;

private:
	static uint64_t m_tradeId;
	std::unordered_map<RoleId, uint64_t> m_tradeMap;
	std::map<uint64_t, Trade::Ptr> m_tradePtrMap;
};


}

#endif
