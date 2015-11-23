/*
 * Author: zhupengfei
 *
 * Created: 2015-07-14 14:50 +0800
 *
 * Modified: 2015-07-14 14:50 +0800
 *
 * Description: 处理查看相关逻辑
 */

#ifndef PROCESS_FUNC_WATCH_MANAGER_HPP
#define PROCESS_FUNC_WATCH_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>

namespace func{

using water::process::ProcessIdentity;

class WatchManager
{
public:
	~WatchManager() = default;
    static WatchManager& me();
private:
	static WatchManager m_me;

public:
    void regMsgHandler();

private:
	//请求查看角色 c->s
	void clientmsg_RequestWatchRole(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求查看英雄 c->s
	void clientmsg_RequestWatchHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	//返回角色主数据及装备信息 world->func
	void servermsg_RetWatchRole(const uint8_t* msgData, uint32_t msgSize);

	//返回英雄主数据及装备信息　world->func
	void servermsg_RetWatchHero(const uint8_t* msgData, uint32_t msgSize);
};

}

#endif
