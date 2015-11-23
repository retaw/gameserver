#include "dragon_ball_manager.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/dragon_ball.h"
#include "protocol/rawmsg/public/dragon_ball.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

DragonBallManager DragonBallManager::m_me;

DragonBallManager& DragonBallManager::me()
{
	return m_me;
}


void DragonBallManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestDragonBallInfo, std::bind(&DragonBallManager::clientmsg_RequestDragonBallInfo, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestLevelUpDragonBall, std::bind(&DragonBallManager::clientmsg_RequestLevelUpDragonBall, this, _1, _2, _3));

}

//请求龙珠信息
void DragonBallManager::clientmsg_RequestDragonBallInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestDragonBallInfo*>(msgData);
	if(!rev)
		return;

	role->m_dragonBall.sendDragonBallInfo();
	return;
}

//请求升级龙珠
void DragonBallManager::clientmsg_RequestLevelUpDragonBall(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestLevelUpDragonBall*>(msgData);
	if(!rev)
		return;

	role->m_dragonBall.requestLevelUpDragonBall(rev->type);
	return;
}


}
