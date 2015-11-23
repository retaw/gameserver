/*
 * Author: zhupengfei
 *
 * Created: 2015-08-25 15:20 +0800
 *
 * Modified: 2015-08-25 15:20 +0800
 *
 * Description: 处理龙珠升级相关消息
 */

#ifndef PROCESS_WORLD_DRAGON_BALL_MANAGER_HPP
#define PROCESS_WORLD_DRAGON_BALL_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class DragonBallManager
{
public:
	~DragonBallManager() = default;
    static DragonBallManager& me();
private:
	static DragonBallManager m_me;

public:
    void regMsgHandler();

private:
	//请求龙珠信息
	void clientmsg_RequestDragonBallInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求升级龙珠
	void clientmsg_RequestLevelUpDragonBall(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

};


}

#endif
