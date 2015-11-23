/*
 * Author: zhupengfei
 *
 * Created: 2015-10-08 17:05 +0800
 *
 * Modified: 2015-10-08 17:05 +0800
 *
 * Description: 处理天下第一相关消息及逻辑
 */

#ifndef PROCESS_FUNC_FIRST_MANAGER_HPP
#define PROCESS_FUNC_FIRST_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"
#include "water/componet/datetime.h"

#include <cstdint>
#include <set>

namespace func{

using namespace water;
using namespace water::componet;
using water::process::ProcessIdentity;
using water::componet::TimePoint;

class FirstManager
{
public:
	FirstManager();
	~FirstManager() = default;
    static FirstManager& me();
private:
	static FirstManager m_me;

public:
    void regMsgHandler();
	
private:
	void clientmsg_RequestFirstApplyInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	void clientmsg_RequestApplyFirst(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	void clientmsg_RequestIntoFirstMap(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	void clientmsg_RequestFirstWinnerInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	void servermsg_UpdateFirstPlayerInfo(const uint8_t* msgData, uint32_t msgSize);

public:
    void regTimer();
private:
    void timerLoop(StdInterval interval, const componet::TimePoint& now);

	void checkSendNotifyApply(const TimePoint& now);
	void checkSendNotifyReady(const TimePoint& now);
	void clearApplySet();

	bool isActionTimeApply();
	bool isActionTimeReady();
	bool isActionBegin();

private:
	void sendApplyInfo(RoleId roleId); 

public:
	bool loadFromDB();	

private:
	bool insertToDB();
	bool deleteAllFromDB();
	
private:
	TimePoint m_notifyTimeApply;	//广播报名公告的时间
	TimePoint m_notifyTimeReady;	//广播准备公告的时间

private:
	RoleId m_winnerId;
	std::string m_winnerName;
	Job m_winnerJob;
	Sex m_winnerSex;

private:
	std::set<RoleId> m_applySet;
};


}

#endif
