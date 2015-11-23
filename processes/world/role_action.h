/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-21 20:54 +0800
 *
 * Modified: 2015-04-21 20:54 +0800
 *
 * Description:  角色的基本行为处理
 */


#ifndef PROCESS_WORLD_ROLE_ACTION_H
#define PROCESS_WORLD_ROLE_ACTION_H

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>
#include <vector>

namespace world{

using water::process::ProcessIdentity;

class RoleAction
{
public:
    void regMsgHandler();
private:
    void clientmsg_RoleMoveToPos(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	void clientmsg_RequestSetRoleBufData(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	
public:
    static RoleAction& me();
};

}

#endif
