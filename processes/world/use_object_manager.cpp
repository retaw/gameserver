#include "use_object_manager.h"
#include "role_manager.h"
#include "object.h"
#include "reward_manager.h"

#include "protocol/rawmsg/public/use_object.h"
#include "protocol/rawmsg/public/use_object.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

UseObjectManager UseObjectManager::m_me;

UseObjectManager& UseObjectManager::me()
{
	return m_me;
}


void UseObjectManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestUseObject, std::bind(&UseObjectManager::clientmsg_RequestUseObject, this, _1, _2, _3));

}

//请求使用道具
void UseObjectManager::clientmsg_RequestUseObject(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestUseObject*>(msgData);
	if(!rev)
		return;

	Object::Ptr obj = role->getObjByCell(rev->cell);
	if(obj == nullptr)
		return;

	const ObjChildType childType = obj->childType();   
	if(childType == ObjChildType::use_gift_fixed || childType == ObjChildType::use_gift_random)
	{
		RewardManager::me().requestOpenGift(rev->cell, rev->num, roleId);
	}
	else
	{
		role->m_useObject.requestUseObj(rev->sceneItem, rev->cell, rev->num);
	}

	return;
}


}
