#include "channel.h"
#include "func.h"
#include "role_manager.h"

#include "water/componet/logger.h"
#include "water/componet/scope_guard.h"

#include "protocol/rawmsg/private/channel_info.h"
#include "protocol/rawmsg/private/channel_info.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "friend_manager.h"
#include "faction_manager.h"
#include "team_manager.h"


namespace func{

Channel& Channel::me()
{
    static Channel me;
    return me;
}

void Channel::sendFactionMsgToGang(RoleId roleId, std::string text)
{
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PublicRaw::NormalChannelMsgToClient) + text.size());
    auto rev = reinterpret_cast<PublicRaw::NormalChannelMsgToClient*>(buf.data());

    rev->type = ChannelType::gang;
    rev->talkerId = 0;
    std::memset(rev->talkerName, 0, NAME_BUFF_SZIE);

    rev->textSize = text.size();
    text.copy(rev->text, text.size());

    auto members = FactionManager::me().getFactionMembers(roleId);
    for(auto& id : members)
	{
        auto role = RoleManager::me().getById(id);
		if(role == nullptr)
			continue;
    
		role->sendToMe(RAWMSG_CODE_PUBLIC(NormalChannelMsgToClient), buf.data(), buf.size());
	}
}

void Channel::sendFactionMsgToGlobal(RoleId roleId, std::string text)
{
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PublicRaw::NormalChannelMsgToClient) + text.size());
    auto rev = reinterpret_cast<PublicRaw::NormalChannelMsgToClient*>(buf.data());

    rev->type = ChannelType::gang;
    rev->talkerId = 0;
    std::memset(rev->talkerName, 0, NAME_BUFF_SZIE);

    rev->textSize = text.size();
    text.copy(rev->text, text.size());

    for(Role::Ptr role : RoleManager::me())
	{
		if(role == nullptr)
			continue;
    
		if(FriendManager::me().isExistBlackList(role->id(), roleId))
            continue;
        
		role->sendToMe(RAWMSG_CODE_PUBLIC(NormalChannelMsgToClient), buf.data(), buf.size());
	}

}

void Channel::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(NormalChannelMsgFromClient, std::bind(&Channel::clientmsg_NormalChannelMsgFromClient, this, _1, _2, _3));
	
	REG_RAWMSG_PUBLIC(GmMsgFromClient, std::bind(&Channel::clientmsg_GmMsgFromClient, this, _1, _2, _3));
	
	//内部消息 world->func
	REG_RAWMSG_PRIVATE(SendSysNotifyToGlobal, std::bind(&Channel::servermsg_SendSysNotifyToGlobal, this, _1, _2));
}

void Channel::clientmsg_NormalChannelMsgFromClient(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto rev = reinterpret_cast<const PublicRaw::NormalChannelMsgFromClient*>(msgData);
    if(rev->textSize + sizeof(*rev) > msgSize)
    {
        LOG_ERROR("频道, 收到的消息长度非法, revSize={}, needSize={}", msgSize, rev->textSize + sizeof(*rev));
        return;
    }

	switch(rev->type)
	{
	case ChannelType::private_chat:
		sendMsgToPrivateChat(roleId, rev);
		break;
	case ChannelType::global:
	case ChannelType::trumpet:
		sendMsgToGlobal(roleId, rev);
		break;
	case ChannelType::scene:
		sendMsgToScene(roleId, rev);
		break;
	case ChannelType::gang:
		sendMsgToGang(roleId, rev);
		break;
	case ChannelType::team:
		sendMsgToteam(roleId, rev);
		break;
	default:
		break;
	}

	return;
}


void Channel::sendMsgToPrivateChat(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	std::vector<uint8_t> buf;
	fillMsgData(role, rev, &buf);
	
	role->sendToMe(RAWMSG_CODE_PUBLIC(NormalChannelMsgToClient), buf.data(), buf.size());
	
	Role::Ptr listener = RoleManager::me().getById(rev->listenerId);
	if(listener == nullptr)
		return;
   
	if(FriendManager::me().isExistBlackList(rev->listenerId, roleId))
        return;

	listener->sendToMe(RAWMSG_CODE_PUBLIC(NormalChannelMsgToClient), buf.data(), buf.size());
}

void Channel::sendMsgToGlobal(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

	std::vector<uint8_t> buf;
	fillMsgData(role, rev, &buf);

    for(Role::Ptr role : RoleManager::me())
	{
		if(role == nullptr)
			continue;
    
		if(FriendManager::me().isExistBlackList(role->id(), roleId))
            continue;
        
		role->sendToMe(RAWMSG_CODE_PUBLIC(NormalChannelMsgToClient), buf.data(), buf.size());
	}

	return;
}

void Channel::sendMsgToScene(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

	std::vector<uint8_t> buf;
	fillMsgData(role, rev, &buf);
		
	SceneId  sceneId= role->sceneId(); 
    for(Role::Ptr role : RoleManager::me())
	{
		if(role == nullptr)
			continue;

		if(role->sceneId() != sceneId)
			continue;

        if(FriendManager::me().isExistBlackList(role->id(), roleId))
            continue;
    
		role->sendToMe(RAWMSG_CODE_PUBLIC(NormalChannelMsgToClient), buf.data(), buf.size());
	}

	return;
}

void Channel::sendMsgToGang(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

	std::vector<uint8_t> buf;
	fillMsgData(role, rev, &buf);
    
	auto members = FactionManager::me().getFactionMembers(roleId);
    for(auto& id : members)
	{
        auto role = RoleManager::me().getById(id);
		if(role == nullptr)
			continue;
    
		if(FriendManager::me().isExistBlackList(role->id(), roleId))
			continue;
        
		role->sendToMe(RAWMSG_CODE_PUBLIC(NormalChannelMsgToClient), buf.data(), buf.size());
	}

	return;
}

void Channel::sendMsgToteam(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev)
{
    Role::Ptr role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

	std::vector<uint8_t> buf;
	fillMsgData(role, rev, &buf);
    
	auto members = TeamManager::me().getTeamMembers(roleId);
    for(Role::Ptr role : members)
	{
		if(role == nullptr)
			continue;
    
		if(FriendManager::me().isExistBlackList(role->id(), roleId))
			continue;
        
		role->sendToMe(RAWMSG_CODE_PUBLIC(NormalChannelMsgToClient), buf.data(), buf.size());
	}

	return;

}

void Channel::fillMsgData(Role::Ptr role, const PublicRaw::NormalChannelMsgFromClient* rev, std::vector<uint8_t>* buf) const
{
	if(role == nullptr)
		return;

	buf->clear();
	buf->reserve(512);
	buf->resize(sizeof(PublicRaw::NormalChannelMsgToClient) + rev->textSize);
	
	auto* msg = reinterpret_cast<PublicRaw::NormalChannelMsgToClient*>(buf->data());
	msg->type = rev->type;
	msg->talkerId = role->id();
	role->name().copy(msg->talkerName, NAME_BUFF_SZIE);
	msg->textSize = rev->textSize;
	msg->propSize = rev->propSize;
	std::strncpy(msg->text, rev->text, rev->textSize); 
	for(ArraySize i = 0; i < rev->propSize; ++i)
	{
		buf->resize(buf->size() + sizeof(PublicRaw::NormalChannelMsgToClient::ToPropList));
		msg = reinterpret_cast<PublicRaw::NormalChannelMsgToClient*>(buf->data());
		PublicRaw::NormalChannelMsgFromClient::FromPropList* propList = (PublicRaw::NormalChannelMsgFromClient::FromPropList*)(rev->text + rev->textSize);

		auto data = (PublicRaw::NormalChannelMsgFromClient::FromPropList*)(msg->text + msg->textSize);

		data[i].tplId = propList[i].tplId;
		data[i].nonsuchProp = propList[i].nonsuchProp;
		data[i].luckyLevel = propList[i].luckyLevel;
		data[i].strongLevel = propList[i].strongLevel;
	}

	return;
}

void Channel::servermsg_SendSysNotifyToGlobal(const uint8_t* msgData, uint32_t msgSize)
{
	auto rev = reinterpret_cast<const PrivateRaw::SendSysNotifyToGlobal*>(msgData);
	if(!rev)
		return;

	if(rev->textSize + sizeof(*rev) > msgSize)
	{
        LOG_ERROR("频道, 系统公告, 收到的消息长度非法, revSize={}, needSize={}", msgSize, rev->textSize + sizeof(*rev));
        return;
	}

	sendSysNotifyToGlobal(rev->type, rev->text);
	return;
}

void Channel::sendSysNotifyToGlobal(ChannelType type, const std::string& text)
{
	if(type != ChannelType::screen_top && type != ChannelType::screen_middle)
		return;

	if(text.empty())
		return;

	const ArraySize textSize = text.size() + 1;
	const uint32_t bufSize = sizeof(const PublicRaw::SystemChannelMsgToClient) + textSize;
	auto buf = new uint8_t[bufSize];
	ON_EXIT_SCOPE_DO(delete[] buf);

	auto send = new(buf) PublicRaw::SystemChannelMsgToClient();
	send->type = type;
	send->textSize = textSize;
	memcpy(send->text, text.c_str(), textSize); 

    for(Role::Ptr role : RoleManager::me())
	{
		if(role == nullptr)
			continue;

        role->sendToMe(RAWMSG_CODE_PUBLIC(SystemChannelMsgToClient), buf, bufSize);
	}

	return;
}

void Channel::clientmsg_GmMsgFromClient(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	auto rev = reinterpret_cast<const PublicRaw::GmMsgFromClient*>(msgData);
	if(!rev)
		return;

	if(rev->textSize + sizeof(*rev) > msgSize)
	{
        LOG_ERROR("GM, 收到的消息长度非法, revSize={}, needSize={}", msgSize, rev->textSize + sizeof(*rev));
        return;
	}

	const uint32_t bufSize = sizeof(const PrivateRaw::BroadcastGmMsgToGlobal) + rev->textSize;
	auto buf = new uint8_t[bufSize];
	ON_EXIT_SCOPE_DO(delete[] buf);

	auto send = new(buf) PrivateRaw::BroadcastGmMsgToGlobal();
	send->roleId = roleId;
	send->textSize = rev->textSize;
	memcpy(send->text, rev->text, rev->textSize); 

	Func::me().broadcastToWorlds(RAWMSG_CODE_PRIVATE(BroadcastGmMsgToGlobal), buf, bufSize);
	return;
}

}

