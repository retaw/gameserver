/*
 * Author: zhupengfei
 *
 * Created: 2015-07-22 16:09 +0800
 *
 * Modified: 2015-07-22 16:09 +0800
 *
 * Description: 官职
 */

#ifndef PROCESS_WORLD_GUANZHI_HPP
#define PROCESS_WORLD_GUANZHI_HPP

#include "attribute.h"

#include "water/common/roledef.h"
#include <memory>

namespace world{

class Role;

/*********************************************/
class Guanzhi : public Attribute 
{
public:
	Guanzhi(Role& owner, uint8_t level);
	~Guanzhi() = default;

public:
	void sendRewardState();
	
	void requestGetDailyReward();
	void requestLevelUp();

public:
	void dealNotSameDay();

private:
	void setDailyRewardState(Reward reward);
	Reward getDailyRewardState();

	bool updateGuanzhiLevelToDB();

public:
	uint8_t level() const;
private:
	void setLevel(uint8_t level);
	void calcAttribute();
	
	void sendLevelUpSucess();
	
private:
	Role& m_owner;
	uint8_t m_level;

private:
	Reward m_dailyReward;
};


}

#endif
