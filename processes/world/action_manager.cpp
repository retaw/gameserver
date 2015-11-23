#include "action_manager.h"
#include "role.h"
#include "world.h"
#include "channel.h"
#include "role_manager.h"
#include "exp_area_manager.h"
#include "bubble_point_manager.h"
#include "world_boss.h"
#include "first_manager.h"
#include "shabake.h"

#include "protocol/rawmsg/public/action.h"
#include "protocol/rawmsg/public/action.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

ActionManager ActionManager::m_me;

ActionManager& ActionManager::me()
{
	return m_me;
}

ActionManager::ActionManager()
{
	init();
}


void ActionManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestSpanSecOfActionEnd, std::bind(&ActionManager::clientmsg_RequestSpanSecOfActionEnd, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestJoinAction, std::bind(&ActionManager::clientmsg_RequestJoinAction, this, _1, _2, _3));
}

void ActionManager::clientmsg_RequestSpanSecOfActionEnd(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestSpanSecOfActionEnd*>(msgData);
	if(!rev)
		return;

	PublicRaw::RetSpanSecOfActionEnd send;
	send.type = rev->type;
	send.sec = getSpanSecOfActionEnd(rev->type);
	
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetSpanSecOfActionEnd), &send, sizeof(send));
	return;
}

void ActionManager::clientmsg_RequestJoinAction(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

    auto rev = reinterpret_cast<const PublicRaw::RequestJoinAction*>(msgData);
    switch(rev->type)
    {
    case ActionType::bubble_point:
        BubblePointManager::me().requestIntoScene(role);
        break;
    case ActionType::world_boss:
        WorldBoss::me().requestIntoWorldBoss(role);
        break;
    case ActionType::shabake:
        ShaBaKe::me().requestIntoShabake(role);
        break;
    default:
        break;
    }
}

void ActionManager::init()
{
	LOG_TRACE("活动, ActionManager, 获取活动状态!!!!!!!!");
	for(uint8_t i = 1; i <= TOTAL_ACTION_NUM; i++)
	{
		ActionType type = static_cast<ActionType>(i);
		m_actionMap[type] = ActionState::end;

		LOG_TRACE("活动, ActionManager, 获取活动状态, type={}, state={}", 
				  type, m_actionMap[type]);
	}
}

uint32_t ActionManager::getSpanSecOfActionEnd(ActionType type)
{
	switch(type)
	{
	case ActionType::exp_area:
		return ExpAreaManager::me().getSpanSecOfActionEnd();
	case ActionType::bubble_point:
		return BubblePointManager::me().getSpanSecOfActionEnd();
    case ActionType::world_boss:
        return WorldBoss::me().leftTime();
	case ActionType::first_apply:
		return FirstManager::me().getSpanSecOfApplyEnd();
	case ActionType::first_ready:
		return FirstManager::me().getSpanSecOfReadyEnd();
    case ActionType::shabake:
        return ShaBaKe::me().lefttime();
	default:
		break;
	}

	return 0;
}

void ActionManager::setActionState(ActionType type, ActionState state)
{
	if(type == ActionType::none)
		return;

	LOG_TRACE("活动, ActionManager, 设置活动状态, type={}, state={}",  type, state);
	m_actionMap[type] = state;
	boardcastAllActionState();

	return;
}

void ActionManager::boardcastAllActionState()
{
	if(m_actionMap.empty())
		return;

	std::vector<uint8_t> buf;
	buf.reserve(256);
	buf.resize(sizeof(PublicRaw::RetAllActionState));

	auto* msg = reinterpret_cast<PublicRaw::RetAllActionState*>(buf.data());
	msg->size = 0;

	for(auto pos = m_actionMap.begin(); pos != m_actionMap.end(); ++pos)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetAllActionState::ActionItem));
		auto* msg  = reinterpret_cast<PublicRaw::RetAllActionState*>(buf.data());

		msg->data[msg->size].type = pos->first;
		msg->data[msg->size].state = pos->second;

		LOG_TRACE("活动, 返回活动状态, 广播, type={}, state={}", pos->first, pos->second);
		++msg->size;
	}

	for(Role::Ptr role : RoleManager::me())
	{
		if(role == nullptr)
			continue;

		role->sendToMe(RAWMSG_CODE_PUBLIC(RetAllActionState), buf.data(), buf.size());
	}

	return;
}

void ActionManager::sendAllActionStateToMe(Role::Ptr role)
{
	if(role == nullptr)
		return;

	std::vector<uint8_t> buf;
	buf.reserve(256);
	buf.resize(sizeof(PublicRaw::RetAllActionState));

	auto* msg = reinterpret_cast<PublicRaw::RetAllActionState*>(buf.data());
	msg->size = 0;

	for(auto pos = m_actionMap.begin(); pos != m_actionMap.end(); ++pos)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetAllActionState::ActionItem));
		auto* msg  = reinterpret_cast<PublicRaw::RetAllActionState*>(buf.data());

		msg->data[msg->size].type = pos->first;
		msg->data[msg->size].state = pos->second;

		LOG_TRACE("活动, 返回活动状态, 个人, name={}, id={}, type={}, state={}",
				  role->name(), role->id(), pos->first, pos->second);
		++msg->size;
	}

	role->sendToMe(RAWMSG_CODE_PUBLIC(RetAllActionState), buf.data(), buf.size());
	return;
}

}
