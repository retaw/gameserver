/*
 * Author: zhupengfei
 *
 * Created: 2015-07-13 17:15 +0800
 *
 * Modified: 2015-07-13 17:15 +0800
 *
 * Description: 处理查看相关逻辑
 */

#ifndef PROCESS_WORLD_WATCH_MANAGER_HPP
#define PROCESS_WORLD_WATCH_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>

namespace world{

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
	//请求查看角色
	void servermsg_RequestWatchRole(const uint8_t* msgData, uint32_t msgSize);

	//请求查看英雄
	void servermsg_RequestWatchHero(const uint8_t* msgData, uint32_t msgSize);

};

}

#endif
