/*
 * Author: zhupengfei
 *
 * Created: 2015-09-15 15:50 +0800
 *
 * Modified: 2015-09-15 15:50 +0800
 *
 * Description: 处理经验区相关消息
 */

#ifndef PROCESS_WORLD_EXP_AREA_MANAGER_HPP
#define PROCESS_WORLD_EXP_AREA_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"
#include "water/componet/datetime.h"

#include <cstdint>


namespace world{

using namespace water;
using water::process::ProcessIdentity;
using water::componet::TimePoint;

class ExpAreaManager
{
public:
	ExpAreaManager();
	~ExpAreaManager() = default;
    static ExpAreaManager& me();
private:
	static ExpAreaManager m_me;

public:
    void regMsgHandler();

private:
	//请求自动加经验列表
	void clientmsg_RequestAutoAddExpList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求开启自动加经验
	void clientmsg_RequestOpenAutoAddExp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求关闭自动加经验
	void clientmsg_RequestCloseAutoAddExp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);


public:
    void regTimer();

private:
    void timerLoop(StdInterval interval, const componet::TimePoint& now);
	
	void checkSendNotify(const TimePoint& now);
	void checkAndSetActionState(const TimePoint& now);

public:
	bool inActionNotifyTime(const TimePoint& now) const;
	bool isActionBegin(const TimePoint& now) const;
	void calcActionEndTime(const TimePoint& now);

	uint32_t getSpanSecOfActionEnd() const;

private:
	TimePoint m_notifyTime;		//广播公告的时间
	TimePoint m_actionEndTime;  //本次活动将要结束的时间点
	bool m_setActionBegin;      //是否设置过活动开始 
	bool m_setActionEnd;        //是否设置过活动结束 
};


}

#endif
