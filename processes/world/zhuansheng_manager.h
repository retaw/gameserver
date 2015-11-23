/*
 * Author: zhupengfei
 *
 * Created: 2015-09-06 14:00 +0800
 *
 * Modified: 2015-09-06 14:00 +0800
 *
 * Description: 处理角色、英雄转生相关消息
 */

#ifndef PROCESS_WORLD_ZHUAN_SHENG_MANAGER_HPP
#define PROCESS_WORLD_ZHUAN_SHENG_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>

namespace world{

using water::process::ProcessIdentity;

class ZhuanshengManager
{
public:
	~ZhuanshengManager() = default;
    static ZhuanshengManager& me();
private:
	static ZhuanshengManager m_me;

public:
    void regMsgHandler();

private:
	//请求转生
	void clientmsg_RequestZhuansheng(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

};


}

#endif
