/*
 * Author: zhupengfei
 *
 * Created: 2015-09-22 20:30 +0800
 *
 * Modified: 2015-09-22 20:30 +0800
 *
 * Description: 处理激情泡点相关消息及逻辑
 */

#ifndef PROCESS_WORLD_BUBBLE_POINT_MANAGER_HPP
#define PROCESS_WORLD_BUBBLE_POINT_MANAGER_HPP

#include "role.h"
#include "scene.h"

#include "water/process/process_id.h"
#include "water/common/roledef.h"
#include "water/common/actiondef.h"
#include "water/componet/datetime.h"
#include "water/componet/event.h"

#include <cstdint>
#include <map>

namespace world{

using namespace water;
using namespace water::componet;
using water::process::ProcessIdentity;
using water::componet::TimePoint;
using water::componet::Event;

class BubblePointManager
{
public:
	BubblePointManager();
	~BubblePointManager() = default;
    static BubblePointManager& me();
private:
	static BubblePointManager m_me;

public:
	void requestIntoScene(Role::Ptr Role);


public:
    void regTimer();
private:
    void timerLoop(StdInterval interval, const componet::TimePoint& now);
	
	void checkAndSetActionState(const TimePoint& now);
	void checkSendNotify(const TimePoint& now);

	bool inActionNotifyTime(const TimePoint& now) const;
	bool isActionBegin() const;
	void calcActionEndTimePoint(const TimePoint& now);

	bool isActionWorld(); 
	bool isActionMap(Scene::Ptr scene);

public:
	void afterRoleEnterScene(Role::Ptr role);
	void beforeRoleLeaveScene(Role::Ptr role);

	void roleBeKilled(Role::Ptr role);

private:
	void dealRoleChangePos(Role::Ptr role, Coord2D oldPos, Coord2D newPos);

	void giveOutExpReward();
	void giveOutSpecialReward(const TimePoint& now);

	uint32_t getBaseRewardExp(uint32_t level);
	uint32_t getExpPercent(Coord2D pos);
	uint32_t getSpecialRewardId(Coord2D pos);
	std::string getPointName(Coord2D pos);

	void sendSpecialPointInfoToAll();
	void sendSpecialPointInfoToMe(Role::Ptr role);
	void fillSpecialPointMsg(std::vector<uint8_t>* buf);

	void sendGetSpecialRewardNeedSecToAll();
	void sendGetSpecialRewardNeedSecToMe(Role::Ptr role);

	void kickoutActionMap(const TimePoint& now);
	void clearBuff(Role::Ptr role);

public:
	uint32_t getSpanSecOfActionEnd() const;

private:
	TimePoint m_notifyTime;					//广播公告的时间
	TimePoint m_actionEndTime;				//本次活动将要结束的时间点
	TimePoint m_nextSendSpecialRewardTime;	//下次发放特殊点奖励的时间
	bool m_setActionBegin;					//是否设置过活动开始
	bool m_setActionEnd;					//是否设置过活动结束
	
private:
	std::map<RoleId, Role::PosChangeEvent::RegID> m_roleChangePosEventRegId;
	std::map<Coord2D, Role::Ptr> m_specialPointMap;
};


}

#endif
