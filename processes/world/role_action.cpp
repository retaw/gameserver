#include "role_action.h"
#include "role_manager.h"

#include "water/componet/logger.h"

#include "protocol/rawmsg/public/role_action.h"
#include "protocol/rawmsg/public/role_action.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include <functional>

namespace world{

RoleAction& RoleAction::me()
{
    static RoleAction me;
    return me;
}


void RoleAction::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(RoleMoveToPos, std::bind(&RoleAction::clientmsg_RoleMoveToPos, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestSetRoleBufData, std::bind(&RoleAction::clientmsg_RequestSetRoleBufData, this, _1, _2, _3));
}


void RoleAction::clientmsg_RoleMoveToPos(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    if(role->isDead())
        return;

    auto rev = reinterpret_cast<const PublicRaw::RoleMoveToPos*>(msgData);
    if(!rev)
		return;

	role->changePos(Coord2D(rev->posX, rev->posY), static_cast<componet::Direction>(rev->dir), rev->type);
}

void RoleAction::clientmsg_RequestSetRoleBufData(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	auto role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestSetRoleBufData*>(msgData);
	if(!rev)
		return;

	if(rev->size + sizeof(*rev) > msgSize)
	{
		LOG_ERROR("角色, 缓存, 收到的消息长度非法, revSize={}, needSize={}", msgSize, rev->size + sizeof(*rev));
		return;
	}

	std::vector<uint16_t> bufferVec;
	for(ArraySize i = 0; i < rev->size; i++)
	{
		bufferVec.push_back(rev->buf[i]);
	}
	
	role->setBufferData(bufferVec);
	return;
}

}
