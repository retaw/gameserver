/*
 * Author: zhupengfei
 *
 * Created: 2015-09-08 14:00 +0800
 *
 * Modified: 2015-09-08 14:00 +0800
 *
 * Description: 处理装备分解相关消息
 */

#ifndef PROCESS_WORLD_FEN_JIE_MANAGER_HPP
#define PROCESS_WORLD_FEN_JIE_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>

namespace world{

using water::process::ProcessIdentity;

class FenjieManager
{
public:
	~FenjieManager() = default;
    static FenjieManager& me();
private:
	static FenjieManager m_me;

public:
    void regMsgHandler();

private:
	void clientmsg_RequestFenjie(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

};


}

#endif
