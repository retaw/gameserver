#include "guanzhi_manager.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/guanzhi.h"
#include "protocol/rawmsg/public/guanzhi.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

GuanzhiManager GuanzhiManager::m_me;

GuanzhiManager& GuanzhiManager::me()
{
	return m_me;
}


void GuanzhiManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestGuanzhiRewardState, std::bind(&GuanzhiManager::clientmsg_RequestGuanzhiRewardState, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestGetGuanzhiReward, std::bind(&GuanzhiManager::clinentmsg_RequestGetGuanzhiReward, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestGuanzhiLevelUp, std::bind(&GuanzhiManager::clientmsg_RequestGuanzhiLevelUp, this, _1, _2, _3));
}

//请求官职奖励状态
void GuanzhiManager::clientmsg_RequestGuanzhiRewardState(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestGuanzhiRewardState*>(msgData);
	if(!rev)
		return;

	role->m_guanzhi.sendRewardState();
	return;
}

//请求领取奖励
void GuanzhiManager::clinentmsg_RequestGetGuanzhiReward(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestGetGuanzhiReward*>(msgData);
	if(!rev)
		return;


	role->m_guanzhi.requestGetDailyReward();
	return;
}

//请求晋升官职
void GuanzhiManager::clientmsg_RequestGuanzhiLevelUp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestGuanzhiLevelUp*>(msgData);
	if(!rev)
		return;

	role->m_guanzhi.requestLevelUp();
	return;
}


}
