/*
 * Author: zhupengfei
 *
 * Created: 2015-08-06 11:45 +0800
 *
 * Modified: 2015-08-06 11:45 +0800
 *
 * Description: 洗练
 */

#ifndef PROCESS_WORLD_WASH_HPP
#define PROCESS_WORLD_WASH_HPP

#include "attribute.h"

#include "water/common/roledef.h"
#include "water/componet/datetime.h"

#include <map>
#include <vector>
#include <memory>

namespace world{

using namespace water;

class PK;
class Role;

class Wash : public Attribute 
{
public:
	Wash(SceneItemType sceneItem, RoleId roleId, PK& owner);
	~Wash() = default;

public:
	void loadFromDB(const std::vector<WashPropInfo>& propVec);	

	void sendCurPropList(uint8_t type);
	void requestLockOrUnlockProp(uint8_t type, uint8_t group, bool lock);
	void requestWash(uint8_t type, uint8_t washWay);
	void requestReplaceCurProp(uint8_t type);

private:
	void sendWashPropResult(uint8_t type);
	void updateWashPropToDB(uint8_t type);

private:
	bool checkLevel(uint8_t type);
	std::shared_ptr<Role> getRole();

	bool checkAndReduceMoney(uint8_t type, uint8_t washWay);
	uint8_t getCurLockPropNum(uint8_t type);
	
	uint8_t randomQuality(uint8_t type, uint8_t washWay);
	PropertyType randomPropType(uint8_t type);
	uint32_t randmonPropValue(uint8_t type, PropertyType propType, uint8_t quality);

	bool isPropTypeExit(uint8_t type, PropertyType propType);
	bool isSpecialPropType(PropertyType propType);
	PropertyType getMinPropTypeByPropType(PropertyType propType);

	void calcAttribute();

private:
	const SceneItemType m_sceneItem; 
	const RoleId m_roleId;
	PK& m_owner;

private:
	struct PropItem
	{
		PropertyType propType;
		uint32_t prop;
		uint8_t quality;
		bool lock;
	};

	struct LockInfo
	{
		uint8_t washWay;
		uint8_t lockNum;
	};

private:
	//<washType, <group, PropItem> >
	std::map<uint8_t, std::map<uint8_t, std::vector<PropItem>> > m_curWashMap;	
	std::map<uint8_t, std::map<uint8_t, std::vector<PropItem>> > m_washResultMap;
};


}

#endif
