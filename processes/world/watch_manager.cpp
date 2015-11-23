#include "watch_manager.h"
#include "role_manager.h"
#include "package.h"
#include "world.h"

#include "protocol/rawmsg/private/watch.h"
#include "protocol/rawmsg/private/watch.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

WatchManager WatchManager::m_me;

WatchManager& WatchManager::me()
{
	return m_me;
}


void WatchManager::regMsgHandler()
{
	REG_RAWMSG_PRIVATE(RequestWatchRoleToWorld, std::bind(&WatchManager::servermsg_RequestWatchRole, this, _1, _2));

	REG_RAWMSG_PRIVATE(RequestWatchHeroToWorld, std::bind(&WatchManager::servermsg_RequestWatchHero, this, _1, _2));
}

//请求查看角色
void WatchManager::servermsg_RequestWatchRole(const uint8_t* msgData, uint32_t msgSize)
{
	auto rev = reinterpret_cast<const PrivateRaw::RequestWatchRoleToWorld*>(msgData);
	if(!rev)
		return;

	Role::Ptr role = RoleManager::me().getById(rev->roleId);
	if(role == nullptr)
		return;

	Package::Ptr equipPackagePtr = role->m_packageSet.getPackageByPackageType(PackageType::equipOfRole);
	Package::Ptr stonePackagePtr = role->m_packageSet.getPackageByPackageType(PackageType::stoneOfRole);
	if(equipPackagePtr == nullptr || stonePackagePtr == nullptr)
		return;

	//获取装备信息
	std::vector<uint8_t> equipBuf;
	equipPackagePtr->fillObjList(&equipBuf);

	//获取宝石信息
	std::vector<uint8_t> stoneBuf;
	stonePackagePtr->fillObjList(&stoneBuf);

	//获取角色主数据
	PrivateRaw::RetWatchRoleToFunc roleData;
	role->fillMainData(&roleData.info);

	//返回角色装备信息及人物主数据
	std::vector<uint8_t> buf;
	buf.resize(sizeof(PrivateRaw::RetWatchRoleToFunc) - 2 * sizeof(RetPackageObjList) + equipBuf.size() + stoneBuf.size()); 

	auto* msg  = reinterpret_cast<PrivateRaw::RetWatchRoleToFunc*>(buf.data());
	msg->watcherId = rev->watcherId;
	msg->equipBufSize = equipBuf.size();
	msg->stoneBufSize = stoneBuf.size();
	std::memcpy(&msg->info, &roleData.info, sizeof(roleData.info));
	std::memcpy(&msg->equipList, equipBuf.data(), equipBuf.size());
	void* stoneData = reinterpret_cast<char*>(&msg->equipList) + equipBuf.size();
	std::memcpy(stoneData, stoneBuf.data(), stoneBuf.size());

	ProcessIdentity funcId("func", 1);
	World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(RetWatchRoleToFunc), buf.data(), buf.size());
	return;
}

//请求查看英雄
void WatchManager::servermsg_RequestWatchHero(const uint8_t* msgData, uint32_t msgSize)
{
	auto rev = reinterpret_cast<const PrivateRaw::RequestWatchHeroToWorld*>(msgData);
	if(!rev)
		return;

	Role::Ptr role = RoleManager::me().getById(rev->roleId);
	if(role == nullptr)
		return;

	Hero::Ptr hero = role->m_heroManager.getDefaultHero();
	if(hero == nullptr)
	{
		std::vector<uint8_t> buf;
		buf.reserve(2048);
		buf.resize(sizeof(PrivateRaw::RetWatchRoleToFunc));
	
		auto* msg  = reinterpret_cast<PrivateRaw::RetWatchHeroToFunc*>(buf.data());    
		msg->watcherId = rev->watcherId;
		msg->summon = false;
		ProcessIdentity funcId("func", 1);
		World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(RetWatchHeroToFunc), buf.data(), buf.size());
		return;
	}

	Package::Ptr equipPackagePtr = hero->m_packageSet.getPackageByPackageType(hero->getEquipPackageType());
	Package::Ptr stonePackagePtr = hero->m_packageSet.getPackageByPackageType(hero->getStonePackageType());
	if(equipPackagePtr == nullptr || stonePackagePtr == nullptr)
		return;
	
	//获取装备信息
	std::vector<uint8_t> equipBuf;
	equipPackagePtr->fillObjList(&equipBuf);

	//获取宝石信息
	std::vector<uint8_t> stoneBuf;
	stonePackagePtr->fillObjList(&stoneBuf);

	//获取英雄主数据
	PrivateRaw::RetWatchHeroToFunc heroData;
	hero->fillMainData(&heroData.info);

	//返回英雄装备信息及英雄主数据
	std::vector<uint8_t> buf;
	buf.resize(sizeof(PrivateRaw::RetWatchHeroToFunc) - 2 * sizeof(RetPackageObjList) + equipBuf.size() + stoneBuf.size()); 

	auto* msg  = reinterpret_cast<PrivateRaw::RetWatchHeroToFunc*>(buf.data());
	msg->watcherId = rev->watcherId;
	msg->summon = true;
	msg->equipBufSize = equipBuf.size();
	msg->stoneBufSize = stoneBuf.size();
	std::memcpy(&msg->info, &heroData.info, sizeof(heroData.info));
	std::memcpy(&msg->equipList, equipBuf.data(), equipBuf.size());
	void* stoneData = reinterpret_cast<char*>(&msg->equipList) + equipBuf.size();
	std::memcpy(stoneData, stoneBuf.data(), stoneBuf.size());
	
	ProcessIdentity funcId("func", 1);
	World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(RetWatchHeroToFunc), buf.data(), buf.size());
	return;
}


}
