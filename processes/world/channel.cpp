#include "channel.h"
#include "world.h"

#include "water/componet/logger.h"
#include "water/componet/scope_guard.h"

#include "protocol/rawmsg/private/channel_info.h"
#include "protocol/rawmsg/private/channel_info.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

Channel Channel::m_me;

Channel& Channel::me()
{
	return m_me;
}


void Channel::sendSysNotifyToGlobalPrivate(ChannelType type, const std::string& text)
{
	if(type != ChannelType::screen_top && type != ChannelType::screen_middle && type != ChannelType::global)
		return;

	if(text.empty())
		return;

	const ArraySize textSize = text.size() + 1;
	const uint32_t bufSize = sizeof(const PrivateRaw::SendSysNotifyToGlobal) + textSize;
	auto buf = new uint8_t[bufSize];
	ON_EXIT_SCOPE_DO(delete[] buf);

	auto send = new(buf) PrivateRaw::SendSysNotifyToGlobal();
	send->type = type;
	send->textSize = textSize;
	memcpy(send->text, text.c_str(), textSize);

	ProcessIdentity funcId("func", 1);
	const bool ret = World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(SendSysNotifyToGlobal), buf, bufSize);
	LOG_TRACE("频道, 发送全服广播, to {} {}, type={}, textSize={}, text={}",
			  funcId, ret ? "ok" : "falied",
			  send->type, send->textSize, send->text);

	return;
}


}
