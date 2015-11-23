/*
 * Author: zhupengfei
 *
 * Created: 2015-09-15 17:35 +0800
 *
 * Modified: 2015-09-15 17:35 +0800
 *
 * Description: 经验区
 */

#ifndef PROCESS_WORLD_EXP_AREA_HPP
#define PROCESS_WORLD_EXP_AREA_HPP

#include "water/common/roledef.h"
#include "water/componet/datetime.h"

#include <vector>
#include <map>

namespace world{

using namespace water;
using water::componet::TimePoint;

class Role;

enum class ExpSecType : uint8_t
{
	none	= 0,
	one		= 1, 
	two		= 2, 
	three	= 3,
	four	= 4,
};

class ExpArea
{
public:
	explicit ExpArea(Role& owner);
	~ExpArea() = default;

public:
	void loadFromDB(const std::vector<std::pair<uint8_t, uint32_t> >& expSecVec);

	void sendAutoAddExpList();
	void requestOpenAutoAddExp(uint8_t type);
	void requestCloseAutoAddExp();

	void timerLoop(StdInterval interval, const componet::TimePoint& now);
	void dealNotSameDay();

private:
	void autoAddExp(const componet::TimePoint& now);
	void breakAutoAddExp();

	bool checkLevel();
	uint32_t getBaseRewardExp();
	uint32_t getBasePercent();
	uint32_t getVipPercent();

	void sendOpenAutoAddExpSucess(uint8_t type);
	void sendAutoAddExpResult(uint64_t exp);
	void sendBreakAutoAddExp();
	bool sendSecToDB(uint8_t type, uint32_t sec);

public:
	void addSec(ExpSecType type, uint32_t sec);
	uint32_t getSecByType(ExpSecType type) const;
	uint32_t getLimitCount() const;
	bool isAutoAddExp() const;

private:
	Role& m_owner;
	ExpSecType m_openExpType;	//开启的自动加经验类型
	TimePoint m_openExpTime;	//开启泡点时间点
	bool bRefreshSceneData;		//是否刷新过人物九屏数据标记

private:
	std::map<uint8_t, uint32_t> m_autoExpMap;	//<type, sec>
};


}

#endif
