/*
 * Author: zhupengfei
 *
 * Created: 2015-09-15 15:50 +0800
 *
 * Modified: 2015-09-15 15:50 +0800
 *
 * Description: 处理经验区相关消息
 */

#ifndef PROCESS_WORLD_BONFIRE_MANAGER_HPP
#define PROCESS_WORLD_BONFIRE_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"
#include "water/componet/coord.h"
#include "water/componet/datetime.h"

#include <cstdint>
#include <unordered_map>

namespace world{

using namespace water;
using namespace water::componet;
using water::process::ProcessIdentity;
using water::componet::TimePoint;

enum class BonfireType : uint8_t
{
	none	= 0,
	normal	= 1, 
	medium	= 2, 
	high	= 3,
};

enum class WineType : uint8_t
{
	none		= 0,
	ten			= 1, //十年女儿红
	hundred		= 2, //百年女儿红
	thousand	= 3, //千年女儿红
};


class BonfireManager
{
public:
	BonfireManager();
	~BonfireManager() = default;
    static BonfireManager& me();
private:
	static BonfireManager m_me;

public:
    void regMsgHandler();

private:
	//请求篝火队伍信息
	void clientmsg_RequestBonfireTeamInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求篝火主人名字
	void clientmsg_RequestBonfireOwnerName(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	//请求加入篝火
	void clientmsg_RequestJoinBonfire(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);


public:
	bool summonBonfire(RoleId roleId, BonfireType type, TplId tplId);
	bool drinkWine(RoleId roleId, WineType type);
	
	void bonfireLeaveScene(TriggerId triggerId);
	uint32_t getLimitCount(RoleId) const;

public:
	void regTimer();
private:
    void timerLoop(StdInterval interval, const componet::TimePoint& now);

	void giveoutReward();
	uint32_t getBaseRewardExp(TriggerId triggerId, uint32_t level);
	uint32_t getTeamPercent(uint8_t num);
	uint32_t getDrinkPercent(RoleId roleId);

	bool checkInBonfireRange(TriggerId triggerId, const Coord2D& pos);

private:
	void sendTeamInfo(RoleId roleId, TriggerId triggerId);
	std::string getOwnerName(TriggerId triggerId);
	WineType getDrinkWineType(RoleId roleId);

private:
	struct TeamItem
	{
		RoleId roleId;
		Job job;
		Sex sex;
		std::string name;
		bool isOwner;
	};

	std::unordered_map<TriggerId, std::vector<TeamItem> > m_bonfireMap;
	std::unordered_map<RoleId, WineType> m_drinkMap;		//是否喝酒

};


}

#endif
