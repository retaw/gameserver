/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-21 20:58 +0800
 *
 * Modified: 2015-04-21 20:58 +0800
 *
 * Description: 处理角色在场景上pk相关的消息逻辑
 */

#ifndef PROCESS_WORLD_ROLE_PK_H
#define PROCESS_WORLD_ROLE_PK_H

#include "water/process/process_id.h"
#include "water/common/roledef.h"
#include "water/componet/datetime.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;
using water::componet::TimePoint;

class RolePk
{
public:
    void regMsgHandler();

private:
    void clientmsg_RoleRequestUpgradeSkill(const uint8_t* msgData, uint32_t msgSize, RoleId rid);
    void clientmsg_HeroRequestUpgradeSkill(const uint8_t* msgData, uint32_t msgSize, RoleId rid);
    void clientmsg_RoleRequestStrengthenSkill(const uint8_t* msgData, uint32_t msgSize, RoleId rid);
    void clientmsg_HeroRequestStrengthenSkill(const uint8_t* msgData, uint32_t msgSize, RoleId rid);
    void clientmsg_RequestAttack(const uint8_t* msgData, uint32_t msgSize, RoleId rid, const TimePoint& now);

	//请求角色复活
	void clientmsg_RequestReliveRole(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	void clientmsg_RequestChangeAttackMode(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

public:
    static RolePk& me();
};

}

#endif
