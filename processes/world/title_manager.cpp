#include "title_manager.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/title.h"
#include "protocol/rawmsg/public/title.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

TitleManager TitleManager::m_me;

TitleManager& TitleManager::me()
{
	return m_me;
}


void TitleManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestTitleList, std::bind(&TitleManager::clientmsg_RequestTitleList, this, _1, _2, _3));

	REG_RAWMSG_PUBLIC(RequestOperateNormalTitle, std::bind(&TitleManager::clientmsg_RequestOperateNormalTitle, this, _1, _2, _3));

}

//请求称号列表
void TitleManager::clientmsg_RequestTitleList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestTitleList*>(msgData);
	if(!rev)
		return;

	role->m_title.sendTitleList();	
	return;
}

//请求佩戴或取下普通称号
void TitleManager::clientmsg_RequestOperateNormalTitle(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestOperateNormalTitle*>(msgData);
	if(!rev)
		return;

	if(1 == rev->type)
	{
		role->m_title.requestUseNormalTitle(rev->titleId);
	}
	else if(2 == rev->type)
	{
		role->m_title.requestTakeOffNormalTitle(rev->titleId);
	}
	return;
}


}
