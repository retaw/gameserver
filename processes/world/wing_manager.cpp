#include "wing_manager.h"
#include "wing_config.h"
#include "role_manager.h"
#include "pkdef.h"
#include "hero.h"

#include "protocol/rawmsg/public/wing.h"
#include "protocol/rawmsg/public/wing.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

WingManager WingManager::m_me;

WingManager& WingManager::me()
{
	return m_me;
}


void WingManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestWingLevelUp, std::bind(&WingManager::clientmsg_RequestWingLevelUp, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestWingZhuling, std::bind(&WingManager::clientmsg_RequestWingZhuling, this, _1, _2, _3));

}

//请求翅膀晋阶
void WingManager::clientmsg_RequestWingLevelUp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestWingLevelUp*>(msgData);
	if(!rev)
		return;

	if(rev->packageType == PackageType::equipOfRole)
	{
		role->m_wing.requestWingLevelUp(rev->cell, rev->packageType, rev->useYuanbao);
	}
	else if(isHeroEquipPackage(rev->packageType))
	{
		Job job = role->getHeroJobByPackageType(rev->packageType);
		if(job == Job::none)
			return;

		Hero::Ptr hero = role->getHeroByJob(job);
		if(hero == nullptr)
			return;

		hero->m_wing.requestWingLevelUp(rev->cell, rev->packageType, rev->useYuanbao);
	}

	return;
}

//请求翅膀注灵
void WingManager::clientmsg_RequestWingZhuling(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestWingZhuling*>(msgData);
	if(!rev)
		return;

	if(rev->packageType == PackageType::equipOfRole)
	{
		role->m_wing.requestWingZhuling(rev->type, rev->cell, rev->packageType);
	}
	else if(isHeroEquipPackage(rev->packageType))
	{
		Job job = role->getHeroJobByPackageType(rev->packageType);
		if(job == Job::none)
			return;

		Hero::Ptr hero = role->getHeroByJob(job);
		if(hero == nullptr)
			return;

		hero->m_wing.requestWingZhuling(rev->type, rev->cell, rev->packageType);
	}

	return;
}

bool WingManager::isHeroEquipPackage(PackageType packageType)
{
	if(packageType == PackageType::equipOfWarrior 
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
		return true;

	return false;
}


}
