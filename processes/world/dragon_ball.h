/*
 * Author: zhupengfei
 *
 * Created: 2015-08-25 15:45 +0800
 *
 * Modified: 2015-08-25 15:45 +0800
 *
 * Description: 龙珠升级
 */

#ifndef PROCESS_WORLD_DRAGON_BALL_HPP
#define PROCESS_WORLD_DRAGON_BALL_HPP

#include "attribute.h"
#include "water/common/roledef.h"

#include <map>

namespace world{

class Role;

class DragonBall : public Attribute
{
public:
	explicit DragonBall(Role& owner);
	~DragonBall() = default;

public:
	void loadFromDB(const std::vector<DragonBallInfo>& dragonVec);

	void sendDragonBallInfo();
	void requestLevelUpDragonBall(uint8_t type);

private:
	bool checkLevel(uint8_t type);
	bool reduceMaterial(uint8_t type);

	bool sendDragonBallExpToDB(uint8_t type, uint32_t exp);
	void sendDragonBallLevelUpResult(uint8_t type, uint8_t oldLevel, uint8_t newLevel, uint32_t exp);
	
	uint8_t getDragonBallLevel(uint8_t type) const;
	void calcAttribute();

private:
	Role& m_owner;
	std::map<uint8_t, uint32_t> m_dragonMap;	//<type, exp>
};


}

#endif
