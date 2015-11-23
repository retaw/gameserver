#include "watch_manager.h"
#include "role_manager.h"
#include "role.h"

#include "protocol/rawmsg/public/watch.h"
#include "protocol/rawmsg/public/watch.codedef.public.h"

#include "protocol/rawmsg/private/watch.h"
#include "protocol/rawmsg/private/watch.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace func{

using namespace std::placeholders;

WatchManager WatchManager::m_me;

WatchManager& WatchManager::me()
{
	return m_me;
}


void WatchManager::regMsgHandler()
{
	//c->s 
	REG_RAWMSG_PUBLIC(RequestWatchRole, std::bind(&WatchManager::clientmsg_RequestWatchRole, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestWatchHero, std::bind(&WatchManager::clientmsg_RequestWatchHero, this, _1, _2, _3));


	//world->func
	REG_RAWMSG_PRIVATE(RetWatchRoleToFunc, std::bind(&WatchManager::servermsg_RetWatchRole, this, _1, _2));

	REG_RAWMSG_PRIVATE(RetWatchHeroToFunc, std::bind(&WatchManager::servermsg_RetWatchHero, this, _1, _2));
}

//请求查看角色
void WatchManager::clientmsg_RequestWatchRole(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr watcher = RoleManager::me().getById(roleId);
	if(watcher == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestWatchRole*>(msgData);
	if(!rev)
		return;

	Role::Ptr role = RoleManager::me().getById(rev->roleId);
	if(role == nullptr)
	{
		watcher->sendSysChat("对方不在线");
		return;
	}

	PrivateRaw::RequestWatchRoleToWorld send;
	send.watcherId = watcher->id();
	send.roleId = role->id();

	role->sendToWorld(RAWMSG_CODE_PRIVATE(RequestWatchRoleToWorld), &send, sizeof(send));
	return;
}

//请求查看英雄
void WatchManager::clientmsg_RequestWatchHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr watcher = RoleManager::me().getById(roleId);
	if(watcher == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestWatchHero*>(msgData);
	if(!rev)
		return;

	Role::Ptr role = RoleManager::me().getById(rev->roleId);
	if(role == nullptr)
	{
		watcher->sendSysChat("对方不在线");
		return;
	}

	PrivateRaw::RequestWatchHeroToWorld send;
	send.watcherId = watcher->id();
	send.roleId = role->id();

	role->sendToWorld(RAWMSG_CODE_PRIVATE(RequestWatchHeroToWorld), &send, sizeof(send));
	return;
}

//返回角色主数据及装备信息 world->func 
void WatchManager::servermsg_RetWatchRole(const uint8_t* msgData, uint32_t msgSize)
{
	auto rev = reinterpret_cast<const PrivateRaw::RetWatchRoleToFunc*>(msgData);
	if(!rev)
		return;

	Role::Ptr watcher = RoleManager::me().getById(rev->watcherId);
	if(watcher == nullptr)
		return;

	std::vector<uint8_t> buf;   
	buf.resize(sizeof(PublicRaw::RetWatchRole) - 2 * sizeof(RetPackageObjList) + rev->equipBufSize + rev->stoneBufSize);

	auto* msg  = reinterpret_cast<PublicRaw::RetWatchRole*>(buf.data());   
	std::memcpy(&msg->info, &rev->info, sizeof(rev->info));  
	std::memcpy(&msg->equipList, &rev->equipList, rev->equipBufSize);
	RetPackageObjList* sourceStoneData = (RetPackageObjList*)(rev->equipList.data + rev->equipList.objListSize); 
	
	void* destStoneData = reinterpret_cast<char*>(&msg->equipList) + rev->equipBufSize;
	std::memcpy(destStoneData, sourceStoneData, rev->stoneBufSize);
	watcher->sendToMe(RAWMSG_CODE_PUBLIC(RetWatchRole), buf.data(), buf.size());
	return;
}

//返回英雄主数据及装备信息　world->func
void WatchManager::servermsg_RetWatchHero(const uint8_t* msgData, uint32_t msgSize)
{
	auto rev = reinterpret_cast<const PrivateRaw::RetWatchHeroToFunc*>(msgData);
	if(!rev)
		return;

	Role::Ptr watcher = RoleManager::me().getById(rev->watcherId);
	if(watcher == nullptr)
		return;

	if(!rev->summon)
	{
		watcher->sendSysChat("对方没有默认英雄");
		return;
	}

	std::vector<uint8_t> buf;   
	buf.resize(sizeof(PublicRaw::RetWatchHero) - 2 * sizeof(RetPackageObjList) + rev->equipBufSize + rev->stoneBufSize);

	auto* msg  = reinterpret_cast<PublicRaw::RetWatchHero*>(buf.data());   
	std::memcpy(&msg->info, &rev->info, sizeof(rev->info));  
	std::memcpy(&msg->equipList, &rev->equipList, rev->equipBufSize);
	RetPackageObjList* sourceStoneData = (RetPackageObjList*)(rev->equipList.data + rev->equipList.objListSize); 
	
	void* destStoneData = reinterpret_cast<char*>(&msg->equipList) + rev->equipBufSize;
	std::memcpy(destStoneData, sourceStoneData, rev->stoneBufSize);  
	watcher->sendToMe(RAWMSG_CODE_PUBLIC(RetWatchHero), buf.data(), buf.size());
	return;
}


}
