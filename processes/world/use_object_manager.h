/*
 * Author: zhupengfei
 *
 * Created: 2015-08-26 17:07 +0800
 *
 * Modified: 2015-08-26 17:07 +0800
 *
 * Description: 处理使用道具相关消息及逻辑
 */

#ifndef PROCESS_WORLD_USE_OBJECT_MANAGER_HPP
#define PROCESS_WORLD_USE_OBJECT_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class UseObjectManager
{
public:
	~UseObjectManager() = default;
    static UseObjectManager& me();
private:
	static UseObjectManager m_me;

public:
    void regMsgHandler();

private:
	//请求使用道具
	void clientmsg_RequestUseObject(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

};


}

#endif
