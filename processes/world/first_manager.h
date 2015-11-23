/*
 * Author: zhupengfei
 *
 * Created: 2015-10-08 17:05 +0800
 *
 * Modified: 2015-10-08 17:05 +0800
 *
 * Description: 处理天下第一相关消息及逻辑
 */

#ifndef PROCESS_WORLD_FIRST_MANAGER_HPP
#define PROCESS_WORLD_FIRST_MANAGER_HPP

#include "role.h"
#include "scene.h"

#include "water/process/process_id.h"
#include "water/common/roledef.h"
#include "water/common/actiondef.h"
#include "water/componet/datetime.h"

#include <cstdint>
#include <map>

namespace world{

using namespace water;
using namespace water::componet;
using water::process::ProcessIdentity;
using water::componet::TimePoint;

enum class GameState : uint8_t
{
	none		= 0,
	apply		= 1,
	ready		= 2,
	start		= 3,
	over		= 4,
	timeout		= 5,
};

class FirstManager
{
public:
	FirstManager();
	~FirstManager() = default;
    static FirstManager& me();
private:
	static FirstManager m_me;

public:
	uint32_t getSpanSecOfApplyEnd() const;
	uint32_t getSpanSecOfReadyEnd() const;
	uint8_t getDuanweiType(RoleId roleId) const;

	void afterRoleEnterScene(Role::Ptr role);
	void beforeRoleLeaveScene(Role::Ptr role);
	
	void roleBeKilled(Role::Ptr role, Role::Ptr attacker);

public:
    void regTimer();
private:
    void timerLoop(StdInterval interval, const componet::TimePoint& now);
	void timer(const TimePoint& now);
	
	void checkAndSetState();
	void calcEndTime();

	void ready();
	void start();
	void over();

	void setModeOfAll(attack_mode mode);
	uint32_t getAlivePlayerNum();
	uint32_t getPoint(RoleId roleId) const;

	bool isActionTimeApply();
	bool isActionBegin();

public:
	bool isActionTimeReady();
	bool isActionWorld(); 
	bool isActionMap(Scene::Ptr scene) const;

private:
	void giveOutRandomReward();
	
	void sendGetPointNum(Role::Ptr role, uint32_t point);
	void sendBattleResult(Role::Ptr role, uint32_t killNum, uint32_t rewardId);

	void sendTopListToAll();
	uint32_t getRank(RoleId roleId);
	uint8_t getDuanweiTypeByPoint(uint32_t point);
	void judgeWinner();
	
	void sendSpanSecOfPkEndToAll();
	void kickoutActionMap(const TimePoint& now);
	void reset();

private:
	TimePoint m_applyEndTime;		//本次活动的结束报名时间
	TimePoint m_readyEndTime;		//本次活动的准备结束时间
	TimePoint m_endTime;			//本次活动结束时间
	GameState m_state;
	attack_mode m_mode;
	bool execGameOverFlag;			//战斗结算标记
	
private:
	std::string m_winnerName;

private:
	struct PlayerItem
	{
		PlayerItem()
		{
			memset(this, 0, sizeof(*this));
		}

		RoleId roleId;
		uint32_t point;
		uint32_t killNum;
		TimePoint killTime;
		Reward rewardState;

		bool operator < (const PlayerItem& source) const
		{
			if(this->killNum > source.killNum)
				return true;
			else if(this->killNum == source.killNum)
				return this->killTime < source.killTime;
			return false;			
		}
	};

	std::set<PlayerItem> m_playerSet;

private:
	bool getPlayerItem(PlayerItem& playerItem, RoleId roleId);
};


}

#endif
