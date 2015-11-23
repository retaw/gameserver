#include "trade_manager.h"
#include "role_manager.h"
#include "world.h"
#include "channel.h"
#include "scene.h"
#include "screen.h"

#include "protocol/rawmsg/public/trade.h"
#include "protocol/rawmsg/public/trade.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

Trade::Trade(uint64_t tradeId, RoleId askerId, RoleId acceptorId)
: m_tradeId(tradeId)
, m_askerId(askerId)
, m_acceptorId(acceptorId)
, m_needErase(false)
{
	m_stateMap[m_askerId] = TradeState::unlock;
	m_stateMap[m_acceptorId] = TradeState::unlock;
}

uint64_t Trade::getTadeId() const
{
	return m_tradeId;
}

bool Trade::isRoleInTheTrade(RoleId roleId) const
{
	if(roleId == m_askerId || roleId == m_acceptorId)
		return true;

	return false;
}

void Trade::setTradeState(RoleId roleId, TradeState state)
{
	m_stateMap[roleId] = state;
}

TradeState Trade::getTradeState(RoleId roleId) const
{
	auto pos = m_stateMap.find(roleId);
	if(pos == m_stateMap.end())
		return TradeState::none;

	return pos->second;
}

void Trade::markErase()
{
	m_needErase = true;
}

bool Trade::needErase() const
{
	if(getTradeState(m_askerId) == TradeState::cancle
	   || getTradeState(m_acceptorId) == TradeState::cancle)
		return true;

	return m_needErase;
}

void Trade::cancleTrade(RoleId roleId)
{
	auto pos = m_stateMap.find(roleId);
	if(pos == m_stateMap.end())
		return;

	setTradeState(roleId, TradeState::cancle);
	sendTradeCancelToAll();
	return;
}

void Trade::requestTradeInfo(RoleId roleId)
{
	sendTradeInfoToMe(roleId);
	return;
}

void Trade::requestInputMoney(RoleId roleId, MoneyType type, uint32_t num)
{
	if(getTradeState(roleId) != TradeState::unlock)
		return;

	if(type != MoneyType::money_2 && type != MoneyType::money_4)
		return;
	
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	uint64_t moneyNum = role->getMoney(type);
	if(0 == moneyNum)
		return;

	if(num > moneyNum)
		num = moneyNum;

	auto& iter = m_tradeMoneyMap[roleId];
	iter[type] = num;
	
	//刷新交易信息
	sendTradeInfoToAll();
	return;
}

void Trade::requestPutObj(RoleId roleId, uint16_t cell)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	if(getTradeState(roleId) != TradeState::unlock)
		return;

	auto pos = m_tradeObjMap.find(roleId);
	if(pos != m_tradeObjMap.end() && pos->second.size() >= 12)
		return;

	if(role->getBindByCell(cell) != Bind::no)
	{
		role->sendSysChat("该物品不能交易");
		return;
	}

	//检查格子是否锁定
	if(role->m_packageSet.isCellFixed(cell, PackageType::role))
		return;

	Object::Ptr obj = role->getObjByCell(cell);
	if(obj == nullptr)
		return;

	TradeObjItem temp;
	temp.cell = cell;
	temp.tplId = obj->tplId();
	temp.num = role->getObjNumByCell(cell);
	temp.skillId = obj->skillId();
	temp.strongLevel = obj->strongLevel();
	temp.luckyLevel = obj->luckyLevel();

	m_tradeObjMap[roleId].push_back(temp);
	//设置背包格子锁定状态
	role->m_packageSet.fixCell(cell, PackageType::role);

	sendTradeInfoToAll();
	return;
}

void Trade::requestRemoveObj(RoleId roleId, uint16_t cell)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	if(getTradeState(roleId) != TradeState::unlock)
		return;

	auto pos = m_tradeObjMap.find(roleId);
	if(pos == m_tradeObjMap.end())
		return;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		if(iter->cell != cell)
			continue;

		//解除背包格子锁定状态
		role->m_packageSet.cancelFixCell(cell, PackageType::role);
		pos->second.erase(iter);
		break;
	}

	sendTradeInfoToAll();
	return;
}

void Trade::requestLockTrade(RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	if(getTradeState(roleId) != TradeState::unlock)
		return;

	setTradeState(roleId, TradeState::lock);
	sendTradeInfoToAll();
	return;
}

void Trade::requestUnlockTrade(RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	if(getTradeState(roleId) != TradeState::lock)
		return;

	setTradeState(roleId, TradeState::unlock);
	sendTradeInfoToAll();
	return;
}

void Trade::requestCancelTrade(RoleId roleId)
{
	Role::Ptr other = RoleManager::me().getById(getTheOtherRoleId(roleId));
	if(other != nullptr)
	{
		other->sendSysChat("交易已取消");
	}

	setTradeState(roleId, TradeState::cancle);
	sendTradeCancelToAll();
	return;
}

void Trade::requestConfimTrade(RoleId roleId)
{
	if(getTradeState(roleId) != TradeState::lock)
		return;

	setTradeState(roleId, TradeState::confim);
	sendTradeInfoToAll();
	if(!isBothPlayerConfim())
		return;

	Role::Ptr asker = RoleManager::me().getById(m_askerId);
	Role::Ptr acceptor = RoleManager::me().getById(m_acceptorId);
	if(asker == nullptr || acceptor == nullptr)
	{
		sendTradeCancelToAll();
		return;
	}

	//检查双方货币
	if(!checkTradeMoney())
	{
		sendTradeCancelToAll();
		return;
	}

	//检查背包空间
	if(!checkPutTradeObj())
	{
		sendTradeCancelToAll();
		return;
	}

	//交易物品
	if(!tradeObj())
	{
		sendTradeCancelToAll();
		return;
	}

	//交易货币
	if(!tradeMoney())
	{
		sendTradeCancelToAll();
		return;
	}

	//通知双方交易成功
	sendTradeSucessToAll();

	//删除此交易
	markErase();
	return;
}

bool Trade::tradeObj()
{
	for(auto pos = m_tradeObjMap.begin(); pos != m_tradeObjMap.end(); ++pos)
	{
		Role::Ptr self = RoleManager::me().getById(pos->first);
		Role::Ptr other = RoleManager::me().getById(getTheOtherRoleId(pos->first));
		if(self == nullptr || other == nullptr)
			return false;

		if(pos->second.empty())
			continue;

		std::vector<ObjItem> tradeObjVec;
		if(!getTradeObj(pos->first, tradeObjVec))
			return false;

		//自己背包中删除交易物品
		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			if(nullptr == self->m_packageSet.eraseFixedCellObj(iter->cell, PackageType::role, "交易"))
				return false;
		}
		
		//对方背包中放入交易物品
		other->putObj(tradeObjVec);
	}

	return true;
}

bool Trade::tradeMoney()
{
	for(auto pos = m_tradeMoneyMap.begin(); pos != m_tradeMoneyMap.end(); ++pos)
	{
		Role::Ptr self = RoleManager::me().getById(pos->first);
		Role::Ptr other = RoleManager::me().getById(getTheOtherRoleId(pos->first));
		if(self == nullptr || other == nullptr)
			return false;

		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			if(!self->reduceMoney(iter->first, iter->second, "交易, otherName={}, otherId={}", other->name(), other->id()))
				return false;

			other->addMoney(iter->first, iter->second, "交易, otherName={}, otherId={}", 
							self->name(), self->id());
		}
	}

	return true;
}


bool Trade::isBothPlayerConfim()
{
	if(m_stateMap[m_askerId] == TradeState::confim 
	   && m_stateMap[m_acceptorId] == TradeState::confim)
		return true;

	return false;
}

bool Trade::checkTradeMoney()
{
	for(auto pos = m_tradeMoneyMap.begin(); pos != m_tradeMoneyMap.end(); ++pos)
	{
		Role::Ptr role = RoleManager::me().getById(pos->first);
		if(role == nullptr)
			return false;

		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			if(iter->second > role->getMoney(iter->first))
				return false;
		}
	}

	return true;
}

bool Trade::checkPutTradeObj()
{
	for(auto pos = m_tradeObjMap.begin(); pos != m_tradeObjMap.end(); ++pos)
	{
		Role::Ptr self = RoleManager::me().getById(pos->first);
		Role::Ptr other = RoleManager::me().getById(getTheOtherRoleId(pos->first));
		if(self == nullptr || other == nullptr)
			return false;
		
		std::vector<ObjItem> tradeObjVec;
		if(!getTradeObj(pos->first, tradeObjVec))
		{
			self->sendSysChat("交易取消");
			other->sendSysChat("交易取消");
			return false;
		}

		if(tradeObjVec.empty())
			continue;

		if(!other->checkPutObj(tradeObjVec))
		{
			self->sendSysChat("对方背包空间不足, 交易取消");
			other->sendSysChat("背包空间不足, 交易取消");
			return false;
		}
	}

	return true;
}

bool Trade::getTradeObj(RoleId roleId, std::vector<ObjItem>& tradeObjVec)
{
	tradeObjVec.clear();
	auto pos = m_tradeObjMap.find(roleId);
	if(pos == m_tradeObjMap.end())
		return false;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		ObjItem temp;
		temp.tplId = iter->tplId;
		temp.num = iter->num;
		temp.bind = Bind::no;
		temp.skillId = iter->skillId;
		temp.strongLevel = iter->strongLevel;
		temp.luckyLevel = iter->luckyLevel;

		tradeObjVec.push_back(temp);
	}

	return true;
}

RoleId Trade::getTheOtherRoleId(RoleId roleId)
{
	if(roleId == m_askerId)
	{
		return m_acceptorId;
	}
	else if(roleId == m_acceptorId) 
	{
		return m_askerId;
	}

	return 0;
}

uint32_t Trade::getTradeMoneyNum(RoleId roleId, MoneyType type)
{
	auto pos = m_tradeMoneyMap.find(roleId);
	if(pos == m_tradeMoneyMap.end())
		return 0;

	if(pos->second.empty())
		return 0;

	auto iter = pos->second.find(type);
	if(iter == pos->second.end())
		return 0;

	return iter->second;
}

void Trade::cancelFixCellOfAll()
{
	for(auto pos = m_tradeObjMap.begin(); pos != m_tradeObjMap.end(); ++pos)
	{
		Role::Ptr role = RoleManager::me().getById(pos->first);
		if(role == nullptr)
			continue;

		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			role->m_packageSet.cancelFixCell(iter->cell, PackageType::role);
		}
	}

	m_tradeObjMap.clear();
	return;
}

void Trade::fillTradeInfo(RoleId roleId, std::vector<uint8_t>* buf)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	buf->clear();
	buf->reserve(512);
	buf->resize(sizeof(PublicRaw::TradeInfo));

	auto* msg = reinterpret_cast<PublicRaw::TradeInfo*>(buf->data());
	msg->roleId = role->id();
	std::memset(msg->name, 0, NAME_BUFF_SZIE);
	role->name().copy(msg->name, NAME_BUFF_SZIE);
	msg->yuanbao = getTradeMoneyNum(roleId, MoneyType::money_4);
	msg->jinbi = getTradeMoneyNum(roleId, MoneyType::money_2);
	msg->tradeState = static_cast<uint8_t>(getTradeState(roleId));

	msg->size = 0;
	auto pos = m_tradeObjMap.find(roleId);
	if(pos != m_tradeObjMap.end())
	{
		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			buf->resize(buf->size() + sizeof(PublicRaw::TradeInfo::TradeObj));
			auto* msg  = reinterpret_cast<PublicRaw::TradeInfo*>(buf->data());

			msg->data[msg->size].cell = iter->cell;
			msg->data[msg->size].tplId = iter->tplId;
			msg->data[msg->size].num = iter->num;
			msg->data[msg->size].skillId = iter->skillId;
			msg->data[msg->size].strongLevel = iter->strongLevel;
			msg->data[msg->size].luckyLevel = iter->luckyLevel;

			++msg->size;
		}
	}
	
	return;
}

void Trade::sendTradeInfoToAll()
{
	sendTradeInfoToMe(m_askerId);
	sendTradeInfoToMe(m_acceptorId);
	return;
}

void Trade::sendTradeInfoToMe(RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	//获取申请者交易信息
	std::vector<uint8_t> askerBuf;
	fillTradeInfo(m_askerId, &askerBuf);

	//获取接受者交易信息
	std::vector<uint8_t> acceptorBuf;
	fillTradeInfo(m_acceptorId, &acceptorBuf);

	std::vector<uint8_t> buf;
	buf.resize(sizeof(PublicRaw::RetTradeInfo) - 2 * sizeof(PublicRaw::TradeInfo) + askerBuf.size() + acceptorBuf.size());

	auto* msg = reinterpret_cast<PublicRaw::RetTradeInfo*>(buf.data());
	std::memcpy(&msg->askerInfo, askerBuf.data(), askerBuf.size());
	void* acceptorData = reinterpret_cast<char*>(&msg->askerInfo) + askerBuf.size();
	std::memcpy(acceptorData, acceptorBuf.data(), acceptorBuf.size());
	
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetTradeInfo), buf.data(), buf.size());
	return;
}

void Trade::sendTradeCancelToAll()
{
	Role::Ptr asker = RoleManager::me().getById(m_askerId);
	Role::Ptr acceptor = RoleManager::me().getById(m_acceptorId);

	PublicRaw::NotifyPlayerTradeCancel send;
	if(asker != nullptr)
	{
		asker->sendToMe(RAWMSG_CODE_PUBLIC(NotifyPlayerTradeCancel), &send, sizeof(send));
	}

	if(acceptor != nullptr)
	{
		acceptor->sendToMe(RAWMSG_CODE_PUBLIC(NotifyPlayerTradeCancel), &send, sizeof(send));
	}

	cancelFixCellOfAll();
	return;
}

void Trade::sendTradeSucessToAll()
{
	Role::Ptr asker = RoleManager::me().getById(m_askerId);
	Role::Ptr acceptor = RoleManager::me().getById(m_acceptorId);

	PublicRaw::NotifyPlayerTradeSucess send;
	if(asker != nullptr)
	{
		asker->sendToMe(RAWMSG_CODE_PUBLIC(NotifyPlayerTradeSucess), &send, sizeof(send));
		asker->sendSysChat("交易成功");
	}

	if(acceptor != nullptr)
	{
		acceptor->sendToMe(RAWMSG_CODE_PUBLIC(NotifyPlayerTradeSucess), &send, sizeof(send));
		acceptor->sendSysChat("交易成功");
	}

	return;
}

/****************************************************************************/
uint64_t TradeManager::m_tradeId = 0;
TradeManager::TradeManager()
{
}

TradeManager TradeManager::m_me;

TradeManager& TradeManager::me()
{
	return m_me;
}


void TradeManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestTrade, std::bind(&TradeManager::clientmsg_RequestTrade, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestTradeAskList, std::bind(&TradeManager::clientmsg_RequestTradeAskList, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RefuseTrade, std::bind(&TradeManager::clientmsg_RefuseTrade, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RefuseAllTrade, std::bind(&TradeManager::clientmsg_RefuseAllTrade, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(AgreeTrade, std::bind(&TradeManager::clientmsg_AgreeTrade, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestTradeInfo, std::bind(&TradeManager::clientmsg_RequestTradeInfo, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestInputTradeMoney, std::bind(&TradeManager::clientmsg_RequestInputTradeMoney, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestPutTradeObj, std::bind(&TradeManager::clientmsg_RequestPutTradeObj, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestRemoveTradeObj, std::bind(&TradeManager::clientmsg_RequestRemoveTradeObj, this, _1, _2, _3));


	REG_RAWMSG_PUBLIC(RequestLockTrade, std::bind(&TradeManager::clientmsg_RequestLockTrade, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestUnlockTrade, std::bind(&TradeManager::clientmsg_RequestUnlockTrade, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestCancelTrade, std::bind(&TradeManager::clientmsg_RequestCancelTrade, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(ConfirmTrade, std::bind(&TradeManager::clientmsg_ConfirmTrade, this, _1, _2, _3));
}

//请求交易
void TradeManager::clientmsg_RequestTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	if(role->level() < 50)
	{
		role->sendSysChat("等级不足, 不能交易");
		return;
	}

	auto rev = reinterpret_cast<const PublicRaw::RequestTrade*>(msgData);
	if(!rev)
		return;

	RoleId destRoleId = rev->destRoleId;
	if(m_askerTimeMap.find(roleId) != m_askerTimeMap.end())
	{
		TimePoint lastAskTime = m_askerTimeMap[roleId];
		TimePoint canAskTime = lastAskTime + std::chrono::seconds {15};
		TimePoint now = Clock::now();
		if(canAskTime > now)
		{
			uint32_t needSec = std::chrono::duration_cast<std::chrono::seconds>(canAskTime - now).count(); 
			role->sendSysChat("{}秒后可再次申请交易", needSec);
			return;
		}
	}

	if(!checkInSameScreen(roleId, destRoleId))
		return;

	DestItem temp;
	temp.destRoleId = destRoleId;
	temp.deleteTime = Clock::now() + std::chrono::seconds {60 * 2};

	auto pos = m_askerMap.find(roleId);
	if(pos != m_askerMap.end())
	{
		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			if(iter->destRoleId != destRoleId)
				continue;
		
			pos->second.erase(iter);
			break;
		}
	}

	m_askerMap[roleId].push_back(temp);
	m_askerTimeMap[roleId] = Clock::now();

	//更新被请求者的申请列表
	if(!updateAcceptorAskList(roleId, destRoleId))
		return;

	//通知玩家收到交易申请
	sendTradeAskToAcceptor(roleId, destRoleId);
	role->sendSysChat("交易申请已发送");
	return;
}

//请求交易申请列表
void TradeManager::clientmsg_RequestTradeAskList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestTradeAskList*>(msgData);
	if(!rev)
		return;

	sendAskListToMe(roleId);
	return;
}

//拒绝交易请求
void TradeManager::clientmsg_RefuseTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RefuseTrade*>(msgData);
	if(!rev)
		return;

	auto pos = m_acceptorMap.find(roleId);
	if(pos == m_acceptorMap.end())
		return;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		if(iter->askerId != rev->askerId)
			continue;

		Role::Ptr askerPtr = RoleManager::me().getById(iter->askerId);
		if(askerPtr != nullptr)
		{
			askerPtr->sendSysChat("对方拒绝了交易申请");
		}
		
		pos->second.erase(iter);
		sendAskListToMe(roleId);
		break;
	}

	return;
}

//拒绝所有交易请求
void TradeManager::clientmsg_RefuseAllTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto pos = m_acceptorMap.find(roleId);
	if(pos == m_acceptorMap.end())
		return;

	for(auto iter = pos->second.begin(); iter != pos->second.end();)
	{
		Role::Ptr askerPtr = RoleManager::me().getById(iter->askerId);
		if(askerPtr != nullptr)
		{
			askerPtr->sendSysChat("对方拒绝了交易申请");
		}
		
		iter = pos->second.erase(iter);
	}

	sendAskListToMe(roleId);
	return;
}

//同意交易请求
void TradeManager::clientmsg_AgreeTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::AgreeTrade*>(msgData);
	if(!rev)
		return;

	if(m_tradeMap.find(roleId) != m_tradeMap.end())
	{
		role->sendSysChat("不可同时与多人交易");
		return;
	}

	if(m_tradeMap.find(rev->askerId) != m_tradeMap.end())
	{
		role->sendSysChat("对方正在交易中");
		return;
	}

	if(!checkTradeOverTime(rev->askerId, roleId))
	{
		eraseTradeAsk(rev->askerId);
		sendAskListToMe(roleId);
		return;
	}

	if(!checkInSameScreen(roleId, rev->askerId))
		return;

	m_tradeId = m_tradeId + 1;
	Trade::Ptr tradePtr = std::make_shared<Trade>(m_tradeId, roleId, rev->askerId);
	if(tradePtr == nullptr)
		return;

	if(!m_tradePtrMap.insert(std::make_pair(m_tradeId, tradePtr)).second)
		return;

	m_tradeMap[roleId] = m_tradeId;
	m_tradeMap[rev->askerId] = m_tradeId;

	//通知两个玩家可以进行交易
	sendAgreeTradeToEachOther(rev->askerId, roleId);

	//交易列表中删除此条交易申请
	eraseTradeAsk(rev->askerId);

	return;
}

//请求交易信息
void TradeManager::clientmsg_RequestTradeInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;

	if(!tradePtr->isRoleInTheTrade(roleId))
		return;

	tradePtr->requestTradeInfo(roleId);
	return;
}

//请求输入货币
void TradeManager::clientmsg_RequestInputTradeMoney(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestInputTradeMoney*>(msgData);
	if(!rev)
		return;

	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;
	
	if(!tradePtr->isRoleInTheTrade(roleId))
		return;

	//检查交易状态
	TradeState state = tradePtr->getTradeState(roleId);
	if(state == TradeState::lock || state == TradeState::confim)
	{
		role->sendSysChat("不可更改货币");
		return;
	}
	
	tradePtr->requestInputMoney(roleId, rev->type, rev->num);
	return;
}

//请求放入交易物品
void TradeManager::clientmsg_RequestPutTradeObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestPutTradeObj*>(msgData);
	if(!rev)
		return;

	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;

	if(!tradePtr->isRoleInTheTrade(roleId))
		return;

	//检查交易状态
	TradeState state = tradePtr->getTradeState(roleId);
	if(state == TradeState::lock || state == TradeState::confim)
	{
		role->sendSysChat("不可更改物品");
		return;
	}

	tradePtr->requestPutObj(roleId, rev->cell);
	return;
}

//请求移除交易物品
void TradeManager::clientmsg_RequestRemoveTradeObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestRemoveTradeObj*>(msgData);
	if(!rev)
		return;

	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;

	if(!tradePtr->isRoleInTheTrade(roleId))
		return;

	//检查交易状态
	TradeState state = tradePtr->getTradeState(roleId);
	if(state == TradeState::lock || state == TradeState::confim)
	{
		role->sendSysChat("不可更改物品");
		return;
	}

	tradePtr->requestRemoveObj(roleId, rev->cell);
	return;
}

//请求锁定交易
void TradeManager::clientmsg_RequestLockTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestLockTrade*>(msgData);
	if(!rev)
		return;

	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;

	if(!tradePtr->isRoleInTheTrade(roleId))
		return;

	//检查交易状态
	TradeState state = tradePtr->getTradeState(roleId);
	if(state == TradeState::none || state == TradeState::lock)
		return;

	tradePtr->requestLockTrade(roleId);
	return;
}

//请求解锁交易
void TradeManager::clientmsg_RequestUnlockTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestUnlockTrade*>(msgData);
	if(!rev)
		return;

	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;

	if(!tradePtr->isRoleInTheTrade(roleId))
		return;

	//检查交易状态
	TradeState state = tradePtr->getTradeState(roleId);
	if(state != TradeState::lock)
		return;

	tradePtr->requestUnlockTrade(roleId);
	return;
}

//请求取消交易
void TradeManager::clientmsg_RequestCancelTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestCancelTrade*>(msgData);
	if(!rev)
		return;

	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;

	if(!tradePtr->isRoleInTheTrade(roleId))
		return;

	tradePtr->requestCancelTrade(roleId);
	return;
}

//确认交易
void TradeManager::clientmsg_ConfirmTrade(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::ConfirmTrade*>(msgData);
	if(!rev)
		return;

	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;

	if(!tradePtr->isRoleInTheTrade(roleId))
		return;

	tradePtr->requestConfimTrade(roleId);
}

bool TradeManager::checkInSameScreen(RoleId sourceId, RoleId destId)
{
	Role::Ptr sourcePtr = RoleManager::me().getById(sourceId);
	Role::Ptr destPtr = RoleManager::me().getById(destId);
	if(sourcePtr == nullptr)
		return false;

	if(destPtr == nullptr)
	{
		sourcePtr->sendSysChat("对方已下线");
		return false;
	}

	if(destPtr->level() < 50)
	{
		sourcePtr->sendSysChat("对方等级不足, 不能交易");
		return false;
	}

	//黑名单
	if(destPtr->inBlackList(sourceId))
	{
		sourcePtr->sendSysChat("无法交易, 你在对方黑名单中");
		return false;
	}

	Scene::Ptr sourceScene = sourcePtr->scene();
	Scene::Ptr destScene = destPtr->scene();
	if(sourceScene == nullptr || destScene == nullptr || sourceScene != destScene)
	{
		sourcePtr->sendSysChat("距离太远不能交易");
		return false;
	}

	Coord2D sourcePos = sourcePtr->pos();
	Coord2D destPos = destPtr->pos();
	if(std::abs(sourcePos.x - destPos.x) <= 10 && std::abs(sourcePos.y - destPos.y) <= 10)
		return true;

	sourcePtr->sendSysChat("距离太远不能交易");  
	return false;	
}

bool TradeManager::checkTradeOverTime(RoleId askerId, RoleId acceptorId)
{
	Role::Ptr acceptor = RoleManager::me().getById(acceptorId);
	if(acceptor == nullptr)
		return false;

	auto pos = m_acceptorMap.find(acceptorId);
	if(pos == m_acceptorMap.end())
		return false;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		if(iter->askerId != askerId)
			continue;

		TimePoint overTime = iter->askTime + std::chrono::seconds {60 * 2};
		TimePoint now = Clock::now();
		if(now >= overTime)
		{
			pos->second.erase(iter);
			acceptor->sendSysChat("交易已过期");
			return false;
		}

		return true;
	}

	return false;
}

void TradeManager::eraseTradeAsk(RoleId askerId)
{
	auto pos = m_acceptorMap.find(askerId);
	if(pos == m_acceptorMap.end())
		return;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		if(iter->askerId != askerId)
			continue;

		pos->second.erase(iter);
		break;
	}

	return;
}

bool TradeManager::updateAcceptorAskList(RoleId askerId, RoleId acceptorId)
{
	Role::Ptr asker = RoleManager::me().getById(askerId);
	if(asker == nullptr)
		return false;

	auto pos = m_acceptorMap.find(acceptorId);
	if(pos != m_acceptorMap.end())
	{
		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			if(iter->askerId != askerId)
				continue;

			pos->second.erase(iter);
			break;
		}

		//交易请求最多为10条
		if(pos->second.size() >= 10)
		{
			pos->second.pop_front();
		}
	}

	AskerItem temp;
	temp.askerId = askerId;
	temp.askerName = asker->name();
	temp.level = asker->level();
	temp.askTime = Clock::now();

	m_acceptorMap[acceptorId].push_back(temp);
	return true;
}

Trade::Ptr TradeManager::getTradePtrByRoleId(RoleId roleId)
{
	if(m_tradeMap.find(roleId) == m_tradeMap.end())
		return nullptr;

	uint64_t tradeId = m_tradeMap[roleId];
	auto pos = m_tradePtrMap.find(tradeId);
	if(pos == m_tradePtrMap.end())
		return nullptr;

	return pos->second;
}

void TradeManager::sendTradeAskToAcceptor(RoleId askerId, RoleId acceptorId)
{
	Role::Ptr asker = RoleManager::me().getById(askerId);
	Role::Ptr acceptor = RoleManager::me().getById(acceptorId);
	if(asker == nullptr || acceptor == nullptr)
		return;

	PublicRaw::NotifyReceiveTradeAsk send;
	acceptor->sendToMe(RAWMSG_CODE_PUBLIC(NotifyReceiveTradeAsk), &send, sizeof(send));
}

void TradeManager::sendAskListToMe(RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	std::vector<uint8_t> buf;
	buf.reserve(512);
	buf.resize(sizeof(PublicRaw::RetTradeAskList));

	auto* msg = reinterpret_cast<PublicRaw::RetTradeAskList*>(buf.data());
	msg->size = 0;

	auto pos = m_acceptorMap.find(roleId);
	if(pos != m_acceptorMap.end())
	{
		for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
		{
			buf.resize(buf.size() + sizeof(PublicRaw::RetTradeAskList::AskerItem));
			auto* msg  = reinterpret_cast<PublicRaw::RetTradeAskList*>(buf.data());

			msg->data[msg->size].askerId = iter->askerId;
			std::memset(msg->data[msg->size].askerName, 0, NAME_BUFF_SZIE);
			iter->askerName.copy(msg->data[msg->size].askerName, NAME_BUFF_SZIE);
			msg->data[msg->size].level = iter->level;

			++msg->size;
		}
	}

	role->sendToMe(RAWMSG_CODE_PUBLIC(RetTradeAskList), buf.data(), buf.size());
	return;
}

void TradeManager::sendAgreeTradeToEachOther(RoleId askerId, RoleId acceptorId)
{
	Role::Ptr asker = RoleManager::me().getById(askerId);
	Role::Ptr acceptor = RoleManager::me().getById(acceptorId);
	if(asker == nullptr || acceptor == nullptr)
		return;

	PublicRaw::RetAgreeTradeToEachOther send;
	asker->sendToMe(RAWMSG_CODE_PUBLIC(RetAgreeTradeToEachOther), &send, sizeof(send));
	acceptor->sendToMe(RAWMSG_CODE_PUBLIC(RetAgreeTradeToEachOther), &send, sizeof(send));
	return;
}

void TradeManager::cancleTrade(RoleId roleId)
{
	Trade::Ptr tradePtr = getTradePtrByRoleId(roleId);
	if(tradePtr == nullptr)
		return;

	tradePtr->cancleTrade(roleId);
	return;
}

void TradeManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&TradeManager::timerLoop, this, StdInterval::sec_1, _1));
}

void TradeManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
	switch(interval)
	{
		case::StdInterval::sec_1:
			timer();
			break;
		default:
			break;
	}
	return;
}

void TradeManager::timer()
{
	std::vector<uint64_t> tradeIdVec;
	for(auto pos = m_tradePtrMap.begin(); pos != m_tradePtrMap.end();)
	{
		if(pos->second == nullptr)
		{
			tradeIdVec.push_back(pos->first);
			pos = m_tradePtrMap.erase(pos);
			continue;
		}

		if(pos->second->needErase())
		{
			tradeIdVec.push_back(pos->first);
			pos = m_tradePtrMap.erase(pos);
		}
		else
		{
			++pos;
		}
	}

	for(auto iter = tradeIdVec.begin(); iter != tradeIdVec.end(); ++iter)
	{
		for(auto pos = m_tradeMap.begin(); pos != m_tradeMap.end();)
		{
			if(*iter != pos->second)
			{
				++pos;
				continue;
			}

			pos = m_tradeMap.erase(pos);
		}
	}

	return;
}

}
