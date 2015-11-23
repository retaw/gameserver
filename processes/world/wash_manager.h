/*
 * Author: zhupengfei
 *
 * Created: 2015-08-06 11:18 +0800
 *
 * Modified: 2015-08-06 11:18 +0800
 *
 * Description: 处理洗练相关消息
 */

#ifndef PROCESS_WORLD_WASH_MANAGER_HPP
#define PROCESS_WORLD_WASH_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class WashManager
{
public:
	~WashManager() = default;
    static WashManager& me();
private:
	static WashManager m_me;

public:
    void regMsgHandler();

private:
	//请求属性列表
	void clientmsg_RequestPropList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求锁定或解锁属性
	void clientmsg_RequestLockOrUnlockProp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求洗练
	void clientmsg_RequestWash(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求替换属性
	void clientmsg_RequestReplaceCurProp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

};


}

#endif
