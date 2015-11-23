/*
 * Author: zhupengfei
 *
 * Created: 2015-06-12 16:23 +0800
 *
 * Modified: 2015-06-12 16:23 +0800
 *
 * Description: 频道
 */

#ifndef PROCESS_WORLD_CHANNEL_HPP
#define PROCESS_WORLD_CHANNEL_HPP

#include "water/common/channeldef.h"
#include "water/componet/format.h"
#include "water/process/process_id.h"


namespace world{

using namespace water;
using water::process::ProcessIdentity;

class Role;

class Channel
{
public:
	~Channel() = default;
    static Channel& me();
private:
	static Channel m_me;

public:
	template<typename... Args>    
	void sendSysNotifyToGlobal(ChannelType type, const std::string& formatStr, const Args&... args)
	{
		const std::string text = componet::format(formatStr, args...); 
		sendSysNotifyToGlobalPrivate(type, text);
		return;
	}

private:
	void sendSysNotifyToGlobalPrivate(ChannelType type, const std::string& text);

};



}




#endif
