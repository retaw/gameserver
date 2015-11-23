#include "use_object.h"
#include "role.h"
#include "hero.h"
#include "scene.h"
#include "relive_config.h"
#include "roles_and_scenes.h"
#include "water/componet/logger.h"

#include "protocol/rawmsg/public/use_object.h"
#include "protocol/rawmsg/public/use_object.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

UseObject::UseObject(Role& owner)
: m_owner(owner)
{
}

void UseObject::requestUseObj(SceneItemType sceneItem, uint16_t cell, uint16_t num)
{
	if(sceneItem != SceneItemType::role && sceneItem != SceneItemType::hero)
		return;

	const uint16_t objNum = m_owner.getObjNumByCell(cell);
	if( 0 == num || num > objNum)
		return;

	Object::Ptr obj = m_owner.getObjByCell(cell);
	if(obj == nullptr)
		return;

	if(1 != num && !obj->bBatUse())
		return;

	if(m_owner.level() < obj->level())
	{
		m_owner.sendSysChat("等级不足, 不可使用");
		return;
	}

	if(!useObj(sceneItem, num, obj))
		return;

	//扣除道具
	if(!m_owner.eraseObjByCell(cell, num, PackageType::role, "使用道具"))
	{
		LOG_ERROR("使用道具, 使用成功, 扣除道具失败! name={}, id=(), sceneItem={}, cell={}, num={}",
				  m_owner.name(), m_owner.id(), cell, num);
		return;
	}

	return;
}

bool UseObject::useObj(SceneItemType sceneItem, uint16_t num, Object::Ptr obj)
{
	if(nullptr == obj)
		return false;

	const uint32_t spe1 = obj->spe1();
	const uint32_t spe2 = obj->spe2();
	const uint32_t totalNum = num * spe1; 

	switch(obj->childType())
	{
	case ObjChildType::drug_addBuff:
		return addBuff(sceneItem, spe1);
	case ObjChildType::drug_addRoleExp:
		return addRoleExp(totalNum);
	case ObjChildType::drug_addHeroExp:
		return addHeroExp(totalNum);
	case ObjChildType::use_backCity:
		return goBackCity();
	case ObjChildType::use_randomPos:
		return gotoRandomPos();
	case ObjChildType::use_money_1:
		return m_owner.addMoney(MoneyType::money_1, totalNum, "使用绑定金币券");
	case ObjChildType::use_money_2:
		return m_owner.addMoney(MoneyType::money_2, totalNum, "使用非绑金币券");
	case ObjChildType::use_money_3:
		return m_owner.addMoney(MoneyType::money_3, totalNum, "使用绑定元宝券");
	case ObjChildType::use_money_4:
		return m_owner.addMoney(MoneyType::money_4, totalNum, "使用元宝券");
	case ObjChildType::use_expSec:
		return addAutoExpSec(static_cast<ExpSecType>(spe2), num, totalNum);
	case ObjChildType::use_bonfire:
		return summonBonfire(static_cast<BonfireType>(spe2), num, spe1);
	case ObjChildType::use_wine:
		return drinkWine(static_cast<WineType>(spe2));
	default:
		break;		
	}

	return false;
}

bool UseObject::addBuff(SceneItemType sceneItem, uint32_t buffId)
{
	if(sceneItem == SceneItemType::role)
	{
		return m_owner.m_buffM.showBuff(buffId);
	}
	else if(sceneItem == SceneItemType::hero)
	{
		Hero::Ptr hero = m_owner.m_heroManager.getDefaultHero();
		if(hero == nullptr)
			return false;

		return hero->m_buffM.showBuff(buffId);
	}

	return false;
}

bool UseObject::addRoleExp(uint32_t exp)
{
	m_owner.addExp(exp);		
	return true;
}

bool UseObject::addHeroExp(uint32_t exp)
{
	Hero::Ptr hero = m_owner.m_heroManager.getDefaultHero();
	if(hero == nullptr)
		return false;

	hero->addExp(exp);	
	return true;
}

bool UseObject::goBackCity()
{
	Scene::Ptr oldScene = m_owner.scene(); 
	if(oldScene == nullptr)  
		return false;

    if(oldScene->copyType() == CopyMap::shabake)
    {
        m_owner.sendSysChat("沙巴克地图中不能使用回城转轴");
        return false;
    }

	//主城为静态地图，静态地图sceneId = mapId 
	SceneId newSceneId = ReliveConfig::me().cityId();  
	if(oldScene->id() == newSceneId)  
	{
		return m_owner.changePos(ReliveConfig::me().cityRelivePos(), m_owner.dir(), MoveType::blink);   
	}
	else
	{
		return RolesAndScenes::me().gotoOtherScene(m_owner.id(), newSceneId, ReliveConfig::me().cityRelivePos()); 
	}

	return false;
}

bool UseObject::gotoRandomPos()
{
	Scene::Ptr s = m_owner.scene();
	if(nullptr == s)
		return false;
    if(s->copyType() == CopyMap::shabake)
    {
        m_owner.sendSysChat("沙巴克地图中不能使用随机转轴");
        return false;
    }

	Coord2D destPos;
	destPos = s->randAvailablePos(SceneItemType::role);
	return m_owner.changePos(destPos, m_owner.dir(), MoveType::blink);	
}

bool UseObject::addAutoExpSec(ExpSecType type, uint16_t num, uint32_t totalNum)
{
	const uint32_t sec = m_owner.m_expArea.getSecByType(type);
	if(sec >= 720 * 60)
	{
		m_owner.sendSysChat("时间已达上限, 物品无法使用");
		return false;
	}

	uint32_t count = m_owner.m_roleCounter.get(CounterType::autoAddExpSec);
	uint32_t limitCount = m_owner.m_expArea.getLimitCount();
	if(count == (uint32_t)-1 || limitCount == (uint32_t)-1)
		return false;

	if(count + num > limitCount)
	{
		m_owner.sendSysChat("每日最多可使用物品{}次", limitCount);
		return false;
	}

	if(sec + totalNum >= 720 * 60)
		totalNum = 720 * 60;

	m_owner.m_roleCounter.add(CounterType::autoAddExpSec, num);
	m_owner.m_expArea.addSec(type, totalNum);
	return true;
}

bool UseObject::summonBonfire(BonfireType type, uint16_t num, uint32_t tplId)
{
	if(1 != num)
		return false;

	uint32_t count = m_owner.m_roleCounter.get(CounterType::bonfire);
	uint32_t limitCount = BonfireManager::me().getLimitCount(m_owner.id());	
	if(count == (uint32_t)-1 || limitCount == (uint32_t)-1)
		return false;

	if(count + num > limitCount)
	{
		m_owner.sendSysChat("每日最多可使用物品{}次", limitCount);
		return false;
	}

	//召唤篝火
	if(!BonfireManager::me().summonBonfire(m_owner.id(), type, tplId))
		return false;

	m_owner.m_roleCounter.add(CounterType::bonfire, num);
	return true;
}

bool UseObject::drinkWine(WineType type)
{
	if(type == WineType::none)
		return false;

	return BonfireManager::me().drinkWine(m_owner.id(), type);
}

}
