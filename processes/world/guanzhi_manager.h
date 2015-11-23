/*
 * Author: zhupengfei
 *
 * Created: 2015-07-22 15:49 +0800
 *
 * Modified: 2015-07-2 15:49 +0800
 *
 * Description: 处理官职相关消息
 */

#ifndef PROCESS_WORLD_GUANZHI_MANAGER_HPP
#define PROCESS_WORLD_GUANZHI_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class GuanzhiManager
{
public:
	~GuanzhiManager() = default;
    static GuanzhiManager& me();
private:
	static GuanzhiManager m_me;

public:
    void regMsgHandler();

private:
	//请求官职奖励状态
	void clientmsg_RequestGuanzhiRewardState(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求领取奖励
	void clinentmsg_RequestGetGuanzhiReward(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求晋升官职
	void clientmsg_RequestGuanzhiLevelUp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

};

}

#endif
