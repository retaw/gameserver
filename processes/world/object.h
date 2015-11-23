/*
 * Author: zhupengfei 
 *
 * Created: 2015-04-20 +0800 
 *
 * Modified: 2015-04-20 +0800
 *
 * Description: 物品
 */

#ifndef PROCESS_WORLD_OBJECT_HPP
#define PROCESS_WORLD_OBJECT_HPP

#include "water/common/objdef.h"
#include "water/common/roledef.h"
#include "water/componet/class_helper.h"

#include <string>

namespace world{

class Object 	
{            	
public:
	TYPEDEF_PTR(Object);
	
	Object(ObjBasicData info);
	~Object() = default;

public:
	std::string name();

	ObjectId objId() const;

	ObjParentType parentType() const;

	ObjChildType childType() const;

	uint16_t maxStackNum() const;

	TplId tplId() const;

	uint8_t quality() const;

	uint8_t level() const;

	Job job() const;

	Sex sex() const;

	TurnLife turnLife() const;

	MoneyType needMoneyType() const;

	uint32_t needMoneyNum() const;

	uint32_t nonsuchId() const;

	uint32_t suitId() const; 

	uint32_t skillId() const;

	bool bBatUse() const;

	bool bDiscard() const;

    MoneyType moneyType() const;

    uint32_t price() const;

    uint32_t prob() const;

	uint32_t objLevel() const;

	uint32_t spe1() const;
	uint32_t spe2() const;
	uint32_t spe3() const;

	const std::vector<FenjieRewardItem>& fenjieRewardVec() const;

    BroadCast broadCast() const;

public:
	void setStrongLevel(uint8_t level);
	uint8_t strongLevel() const;

	void setLuckyLevel(uint8_t level);
	uint8_t luckyLevel() const;

public:
	bool isEquipment() const;

	const ObjBasicData& getObjBasicData() const;

private:
	ObjBasicData m_data;


};


}


#endif
