#include "wash_manager.h"
#include "role_manager.h"
#include "pkdef.h"
#include "hero.h"

#include "protocol/rawmsg/public/wash.h"
#include "protocol/rawmsg/public/wash.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

WashManager WashManager::m_me;

WashManager& WashManager::me()
{
	return m_me;
}


void WashManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestPropList, std::bind(&WashManager::clientmsg_RequestPropList, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestLockOrUnlockProp, std::bind(&WashManager::clientmsg_RequestLockOrUnlockProp, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestWash, std::bind(&WashManager::clientmsg_RequestWash, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestReplaceCurProp, std::bind(&WashManager::clientmsg_RequestReplaceCurProp, this, _1, _2, _3));

}

//请求属性列表
void WashManager::clientmsg_RequestPropList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestPropList*>(msgData);
	if(!rev)
		return;

	if(rev->sceneItem == SceneItemType::role)
	{
		role->m_wash.sendCurPropList(rev->type);
	}
	else if(rev->sceneItem == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getDefaultHero();
		if(hero == nullptr)
			return;

		hero->m_wash.sendCurPropList(rev->type);
	}

	return;
}

//请求锁定或解锁属性
void WashManager::clientmsg_RequestLockOrUnlockProp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestLockOrUnlockProp*>(msgData);
	if(!rev)
		return;

	if(rev->sceneItem == SceneItemType::role)
	{
		role->m_wash.requestLockOrUnlockProp(rev->type, rev->group, rev->lock);
	}
	else if(rev->sceneItem == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getDefaultHero();
		if(hero == nullptr)
			return;

		hero->m_wash.requestLockOrUnlockProp(rev->type, rev->group, rev->lock);
	}

	return;
}

//请求洗练
void WashManager::clientmsg_RequestWash(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestWash*>(msgData);
	if(!rev)
		return;

	if(rev->sceneItem == SceneItemType::role)
	{
		role->m_wash.requestWash(rev->type, rev->washWay);
	}
	else if(rev->sceneItem == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getDefaultHero();
		if(hero == nullptr)
			return;
		
		hero->m_wash.requestWash(rev->type, rev->washWay);
	}

	return;
}

//请求替换属性
void WashManager::clientmsg_RequestReplaceCurProp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestReplaceCurProp*>(msgData);
	if(!rev)
		return;

	if(rev->sceneItem == SceneItemType::role)
	{
		role->m_wash.requestReplaceCurProp(rev->type);
	}
	else if(rev->sceneItem == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getDefaultHero();
		if(hero == nullptr)
			return;
		
		hero->m_wash.requestReplaceCurProp(rev->type);
	}
	
	return;
}


}
