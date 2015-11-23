#include "zhuansheng_manager.h"
#include "role_manager.h"
#include "pkdef.h"
#include "hero.h"

#include "protocol/rawmsg/public/zhuansheng.h"
#include "protocol/rawmsg/public/zhuansheng.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

ZhuanshengManager ZhuanshengManager::m_me;

ZhuanshengManager& ZhuanshengManager::me()
{
	return m_me;
}


void ZhuanshengManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestZhuansheng, std::bind(&ZhuanshengManager::clientmsg_RequestZhuansheng, this, _1, _2, _3));

}

//请求转生
void ZhuanshengManager::clientmsg_RequestZhuansheng(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestZhuansheng*>(msgData);
	if(!rev)
		return;

	if(rev->sceneItem == SceneItemType::role)
	{
		role->m_zhuansheng.requestZhuansheng();
	}
	else if(rev->sceneItem == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getDefaultHero();
		if(hero == nullptr)
			return;

		hero->m_zhuansheng.requestZhuansheng();
	}

	return;
}


}
