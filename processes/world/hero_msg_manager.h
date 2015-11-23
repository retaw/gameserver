/*
 * Author: zhupengfei
 *
 * Created: 2015-06-17 17:15 +0800
 *
 * Modified: 2015-06-17 17:15 +0800
 *
 * Description: 处理英雄相关逻辑
 */

#ifndef PROCESS_WORLD_HERO_MSG_MANAGER_HPP
#define PROCESS_WORLD_HERO_MSG_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/herodef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class HeroMsgManager
{
public:
	~HeroMsgManager() = default;
    static HeroMsgManager& me();
private:
	static HeroMsgManager m_me;

public:
    void regMsgHandler();

private:
	//请求已创建英雄列表
	void clientmsg_RequestCreatedHeroList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求创建英雄
	void clientmsg_RequestCreateHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求召唤英雄
	void clientmsg_RequestSummonHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求召回英雄
	void clientmsg_RequestRecallHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求移动英雄
	void clientmsg_RequestHeroChangePos(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求设置默认召唤英雄
	void clientmsg_RequestSetDefaultCallHero(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    //英雄ai相关的几个消息
    void clientmsg_HeroAIMode(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_HeroLockOnTarget(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RoleLockOnTarget(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_HeroDisableSkillList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    //========================= servermsg ===============================
    //db返回英雄序列化data
    void servermsg_RetHeroSerializeData(const uint8_t* msgData, uint32_t msgSize);

};

}

#endif
