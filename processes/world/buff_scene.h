/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-21 20:58 +0800
 *
 * Modified: 2015-04-21 20:58 +0800
 *
 * Description: 处理角色在场景上pk相关的消息逻辑
 */

#ifndef PROCESS_WORLD_BUFF_SCENE_H
#define PROCESS_WORLD_BUFF_SCENE_H

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class BuffScene
{
public:
    void regMsgHandler();

private:
    void clientmsg_RoleRequestSelectedBuff(const uint8_t* msgData, uint32_t msgSize, RoleId rid);
    void clientmsg_RoleRequestSelectedBuffTips(const uint8_t* msgData, uint32_t msgSize, RoleId rid);

public:
    static BuffScene& me();
};

}

#endif
