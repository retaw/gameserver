/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-06 21:05 +0800
 *
 * Modified: 2015-05-06 21:05 +0800
 *
 * Description: 频道
 */


#ifndef PROCESSES_FUNC_CHANNEL_H
#define PROCESSES_FUNC_CHANNEL_H


#include "water/common/roledef.h"
#include "water/common/channeldef.h"
#include "water/componet/format.h"

#include "protocol/rawmsg/public/channel_info.h"
#include "protocol/rawmsg/public/channel_info.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace func{

typedef uint64_t GangId;
using namespace water;

class Role;

class Channel
{
public:
    ~Channel() = default;

    static Channel& me();

private:
	Channel() = default;

public:
    void regMsgHandler();

    //给帮派发送帮派的提示信息
    void sendFactionMsgToGang(RoleId roleId, std::string text);    
    void sendFactionMsgToGlobal(RoleId roleId, std::string text);    

private:
    void clientmsg_NormalChannelMsgFromClient(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	void clientmsg_GmMsgFromClient(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	void servermsg_SendSysNotifyToGlobal(const uint8_t* msgData, uint32_t msgSize);


private:
	//非系统消息
	void sendMsgToPrivateChat(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev);
	void sendMsgToGlobal(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev);
	void sendMsgToScene(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev);
	void sendMsgToGang(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev);
	void sendMsgToteam(RoleId roleId, const PublicRaw::NormalChannelMsgFromClient* rev);

	void fillMsgData(std::shared_ptr<Role> role, const PublicRaw::NormalChannelMsgFromClient* rev, std::vector<uint8_t>* buf) const;
	
public:
	template<typename... Args>    
	void sendSysNotifyToGlobal(ChannelType type, const std::string& formatStr, const Args&... args)
	{
		const std::string text = componet::format(formatStr, args...); 
		sendSysNotifyToGlobal(type, text);
		return;
	}

	//系统消息
	void sendSysNotifyToGlobal(ChannelType type, const std::string& text);
};


}

#endif
