/*
 * Author: zhupengfei
 *
 * Created: 2015-08-26 17:21:00 +0800
 *
 * Modified: 2015-08-26 17:21:00 +0800  
 *
 * Description: 使用道具
 */

#ifndef PROCESS_WORLD_USE_OBJECT_HPP
#define PROCESS_WORLD_USE_OBJECT_HPP

#include "object.h"
#include "exp_area.h"
#include "bonfire_manager.h"

#include "water/common/roledef.h"

namespace world{

class Role;

class UseObject
{
public:
	explicit UseObject(Role& owner);
	~UseObject() = default;

public:
	void requestUseObj(SceneItemType sceneItem, uint16_t cell, uint16_t num);

private:
	bool useObj(SceneItemType sceneItem, uint16_t num, Object::Ptr obj);
	
	bool addBuff(SceneItemType sceneItem, uint32_t buffId);
	bool addRoleExp(uint32_t exp);
	bool addHeroExp(uint32_t exp);

	bool goBackCity();
	bool gotoRandomPos();

	bool addAutoExpSec(ExpSecType type, uint16_t num, uint32_t totalNum);
	bool summonBonfire(BonfireType type, uint16_t num, uint32_t tplId);
	bool drinkWine(WineType type);

private:
	Role& m_owner;
};



}

#endif
