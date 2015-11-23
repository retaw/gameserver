/*
 * Author: zhupengfei
 *
 * Created: 2015-09-25 10:30 +0800
 *
 * Modified: 2015-09-25 10:30 +0800
 *
 * Description: 通知活动状态
 */

#ifndef PROCESS_WORLD_ACTION_MANAGER_HPP
#define PROCESS_WORLD_ACTION_MANAGER_HPP

#include "role.h"

#include "water/common/roledef.h"
#include "water/common/actiondef.h"

#include <cstdint>

namespace world{

using namespace water;
using water::process::ProcessIdentity;
using water::componet::TimePoint;

class Role;

class ActionManager
{
public:
	ActionManager();
	~ActionManager() = default;
    static ActionManager& me();
private:
	static ActionManager m_me;

public:
    void regMsgHandler();
private:
	void clientmsg_RequestSpanSecOfActionEnd(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
	void clientmsg_RequestJoinAction(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);


private:
	void init();

	uint32_t getSpanSecOfActionEnd(ActionType type);
	
	void boardcastAllActionState();

public:
	void setActionState(ActionType type, ActionState state);
	void sendAllActionStateToMe(Role::Ptr role); 

private:
	std::map<ActionType, ActionState> m_actionMap;

};


}

#endif
