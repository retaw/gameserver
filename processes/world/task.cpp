#include "task.h"
#include "role.h"
#include "world.h"
#include "npc_manager.h"
#include "factiontask.h"

#include "water/componet/logger.h"
#include "water/componet/serialize.h"

#include "protocol/rawmsg/private/task.h"
#include "protocol/rawmsg/private/task.codedef.private.h"
#include "protocol/rawmsg/public/task.h"
#include "protocol/rawmsg/public/task.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

RoleTask::RoleTask(Role& me)
: m_owner(me)
, m_nextUnacceptableMainTask(0)
{
}

void RoleTask::acceptTask(TaskId taskId, NpcId npcId, bool autoAccept/*=false*/)
{
    if(m_owner.isDead())
        return;
    TaskTpl::Ptr taskPtr = TaskBase::me().getTaskTpl(taskId);
    if(nullptr == taskPtr)
    {
        LOG_DEBUG("任务, taskId={} 配置异常", taskId);
        return;
    }

    if(!autoAccept)
    {
        if(m_acceptableTasks.find(taskId) == m_acceptableTasks.end())
        {
            LOG_DEBUG("任务, taskId={} 不可接取", taskId);
            return;
        }

        //传情
        if(taskPtr->taskObj)
        {
            if(!m_owner.checkPutObj(taskPtr->taskObj, 1, Bind::yes, PackageType::role))
            {
                m_owner.sendSysChat("包裹空间不足,无法接取任务");
                return;
            }
            m_owner.putObj(taskPtr->taskObj, 1, Bind::yes, PackageType::role);
        }
    }

    changeTaskState(taskId, TaskState::accepted);
}

void RoleTask::arriveTargetTaskNpc(PKId npcId)
{
    TaskParam param;
    param.element.npcId = npcId;
    dispatch(TaskContent::visit_npc, param);
}

void RoleTask::submitTask(TaskId taskId, NpcId npcId)
{
    if(m_owner.isDead())
        return;
    if(m_finishedTasks.find(taskId) == m_finishedTasks.end())
    {
        LOG_DEBUG("任务, 提交任务taskId={}, 发现未完成", taskId);
        return;
    }

    TaskTpl::Ptr taskPtr = TaskBase::me().getTaskTpl(taskId);
    if(nullptr == taskPtr)
    {
        LOG_DEBUG("任务, taskId={} 配置异常", taskId);
        return;
    }

    Npc::Ptr npc = NpcManager::me().getById(npcId);
    if(nullptr == npc || npc->tplId() != taskPtr->submitNpc)
        return;

    if(std::abs(npc->pos().x - m_owner.pos().x) > 6
        || std::abs(npc->pos().y - m_owner.pos().y) > 6)
    {
        LOG_DEBUG("任务, 距离交任务npc过远");
        return;
    }

    std::vector<ObjItem> objs;
    for(const auto& it : taskPtr->taskObjAwards)
    {
        if(Job::none != it.job && m_owner.job() != it.job)
            continue;
        ObjItem obj;
        obj.tplId = it.objId;
        obj.num = it.objNum;
        obj.bind = it.bind;
        objs.push_back(obj);
    }

    if(!m_owner.checkPutObj(objs))
    {
        m_owner.sendSysChat("背包空间不足, 不能完成任务");
        return;
    }

    changeTaskState(taskId, TaskState::over);
    giveTaskAward(taskPtr);

    checkAndUnlockTask();
}

void RoleTask::dispatch(TaskContent content, TaskParam param)
{
    if(m_owner.isDead())
        return;
    switch(content)
    {
    case TaskContent::visit_npc:
    case TaskContent::talk_npc:
    case TaskContent::transfer_obj:
        onVisitNpc(param);
        break;
    case TaskContent::kill_npc:
        onKillNpc(param);
        break;
    case TaskContent::pass_copy:
        onPassCopy(param);
        break;
    case TaskContent::collection:
        onCollection(param);
        break;
    default:
        break;
    }
}

void RoleTask::changeTaskState(TaskId taskId, TaskState state)
{
    TaskTpl::Ptr taskPtr = TaskBase::me().getTaskTpl(taskId);
    if(nullptr == taskPtr)
    {
        taskPtr = TaskBase::me().getFactionTaskTpl(taskId);
        if(taskPtr == nullptr)
            return;
    }

	TaskInfo info;
	info.taskId = taskId;
	info.type = taskPtr->type;
	info.state = TaskState::accepted;
	info.time = taskPtr->type == TaskType::daily ? m_owner.m_dailyTask.getDailyTaskTime(taskId) : 0;	
	info.star = taskPtr->type == TaskType::daily ? m_owner.m_dailyTask.getDailyTaskStar(taskId) : 0;
	for(const auto& iter : taskPtr->taskContentCfgs)
	{
		TaskStep step;
		step.content = iter.content;
		step.state = 0;
		step.count = 0;
		step.reserve = 0;
		info.steps.push_back(step);
	}

    switch(state)
    {
	case TaskState::acceptable:
		{
			if(taskPtr->type != TaskType::daily)
				break;

			info.state = TaskState::acceptable;
			m_acceptableDailyTasks.insert(std::make_pair(taskId, info));
		}
		break;
    case TaskState::accepted:
        {
			info.state = TaskState::accepted;
            m_acceptedTasks.insert({taskId, info});
            refreshAcceptedTaskDetails(taskId);

			if(taskPtr->type == TaskType::daily)
			{
				m_acceptableDailyTasks.erase(taskId);
			}
		}
        break;
    case TaskState::finished:
        {
            m_acceptedTasks.erase(taskId);
			info.state = TaskState::finished;
            m_finishedTasks.insert(std::make_pair(taskId, info));

            refreshTaskState(taskId, state);
            //通知帮派任务模块，任务完成，让帮派模块变为可完成状态,等提交之后帮派模块会主动删除m_finishedTasks的该任务
            if(taskPtr->type == TaskType::faction)
            {
                m_owner.m_roleFactionTask.m_taskInfo.state = TaskState::finished;
            }
			else if(taskPtr->type == TaskType::daily)
			{
				m_owner.m_dailyTask.finishTask(taskId);
			}
        }
        break;
    case TaskState::over:
        {
            m_finishedTasks.erase(taskId);
            //如果是帮派任务，不需要插入m_overTasks，帮派任务自己处理
            if(taskPtr->type != TaskType::faction)
                m_overTasks.insert(taskId);
            refreshTaskState(taskId, state);
        }
        break;
    case TaskState::quit://放弃任务
        {
            m_acceptedTasks.erase(taskId);
            m_acceptableDailyTasks.erase(taskId);
			refreshTaskState(taskId, state);
        }
        break;
    default:
        return;
    }

	updateAllTaskInfoToDB();
	LOG_DEBUG("任务, role=({},{}), taskId={} 状态改变, state={}", m_owner.name(), m_owner.id(), taskId, state);
}

void RoleTask::checkAndUnlockTask(bool refresh/*=true*/)
{
    m_nextUnacceptableMainTask = 0;
    m_acceptableTasks.clear();
    std::vector<TaskId> autoAcceptTasks;
    uint32_t ownerLevel = m_owner.level();
    auto unlockExec = [&](TaskTpl::Ptr taskPtr) -> bool
    {
        if(nullptr == taskPtr)
            return false;
        if(taskPtr->type != TaskType::main && taskPtr->type != TaskType::branch)
            return false;

        //具备解锁的任务必须满足:
        //1, 等级需求
        //2, 前置任务完成
        //3, 没有做过的任务
        if((taskPtr->preTaskId > 0 ? m_overTasks.find(taskPtr->preTaskId) != m_overTasks.end() : true)
           && m_acceptedTasks.find(taskPtr->taskId) == m_acceptedTasks.end()
           && m_finishedTasks.find(taskPtr->taskId) == m_finishedTasks.end()
           && m_overTasks.find(taskPtr->taskId) == m_overTasks.end())
        {
            if(ownerLevel <= taskPtr->maxLevel && ownerLevel >= taskPtr->minLevel)
            {
                if(taskPtr->autoAccept)
                    autoAcceptTasks.push_back(taskPtr->taskId);
                else
                {
                    LOG_DEBUG("任务, role=({},{}), 解锁任务", m_owner.name(), m_owner.id());
                    m_acceptableTasks.insert(taskPtr->taskId);
                }
            }
            else if(taskPtr->type == TaskType::main 
                    && ownerLevel < taskPtr->minLevel)
                m_nextUnacceptableMainTask = taskPtr->taskId;
        }
        return false;
    };

    TaskBase::me().execTaskTpls(unlockExec);

    for(const auto& taskId : autoAcceptTasks)
        acceptTask(taskId, 0, true);

    if(refresh)
    {
        for(const auto& taskId : m_acceptableTasks)
            refreshTaskState(taskId, TaskState::acceptable);

        notifyNextUnacceptableMainTask();
    }
}

void RoleTask::notifyNextUnacceptableMainTask()
{
    PublicRaw::NotifyNextUnacceptableMainTask notify;
    notify.taskId = m_nextUnacceptableMainTask;
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(NotifyNextUnacceptableMainTask), &notify, sizeof(notify));
}

void RoleTask::refreshTaskState(TaskId taskId, TaskState state)
{
	if(0 == taskId)
		return;

    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::RefreshTaskState) + sizeof(PublicRaw::RefreshTaskState::TaskStateEle));
    auto msg = reinterpret_cast<PublicRaw::RefreshTaskState*>(buf.data());
    msg->data[0].taskId = taskId;
    msg->data[0].state = state;
    msg->size = 1;
    msg->modify = true;
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshTaskState), buf.data(), buf.size());
}

void RoleTask::refreshAcceptedTaskDetails(TaskId taskId, bool modify/*=true*/)
{
    if(m_acceptedTasks.find(taskId) == m_acceptedTasks.end())
        return;
    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::RefreshAcceptedTaskDetails));
    auto msg = reinterpret_cast<PublicRaw::RefreshAcceptedTaskDetails*>(buf.data());
    msg->taskId = taskId;
    msg->modify = modify;

    const TaskInfo& info = m_acceptedTasks[taskId];
    for(const auto& step : info.steps)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RefreshAcceptedTaskDetails::TaskSteps));
        auto msg = reinterpret_cast<PublicRaw::RefreshAcceptedTaskDetails*>(buf.data());
        msg->data[msg->size].content = step.content;
        msg->data[msg->size].state = step.state;
        msg->data[msg->size].npcCount = step.count;
        msg->data[msg->size].reserve = step.reserve;
        ++msg->size;
    }

    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshAcceptedTaskDetails), buf.data(), buf.size());
}

void RoleTask::onVisitNpc(TaskParam param)
{
    for(auto& iter : m_acceptedTasks)
    {
        TaskTpl::Ptr taskPtr = TaskBase::me().getTaskTpl(iter.first);
        if(nullptr == taskPtr)
        {
            taskPtr = TaskBase::me().getFactionTaskTpl(iter.first);
            if(nullptr == taskPtr)
                continue;
        }

        bool allstepFinished = true;
        auto& steps = iter.second.steps;
        uint32_t stepSize = steps.size();
        for(uint32_t i = 0; i < stepSize; ++i)
        {
            if(steps[i].state == 1) //已经完成
                continue;
            if(steps[i].content != TaskContent::visit_npc 
               && steps[i].content != TaskContent::talk_npc
               && steps[i].content != TaskContent::transfer_obj)
            {
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }

            //范围验证
            Npc::Ptr npc = NpcManager::me().getById(param.element.npcId);
            if(nullptr == npc || npc->tplId() != taskPtr->taskContentCfgs[i].param1)
            {
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }
            if(std::abs(npc->pos().x - m_owner.pos().x) > 6
               || std::abs(npc->pos().y - m_owner.pos().y) > 6)
            {
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }

            if(taskPtr->taskObj && !m_owner.m_packageSet.eraseObj(taskPtr->taskObj, 1, Bind::yes, PackageType::role, ""))
            {
                m_owner.sendSysChat("任务道具丢失!!!");
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }

            steps[i].state = 1;
        }

        if(allstepFinished)
        {
            changeTaskState(iter.first, TaskState::finished);
            break;
        }
    }
}

void RoleTask::onKillNpc(TaskParam param)
{
    std::vector<TaskId> finishedTasks;
    for(auto& iter : m_acceptedTasks)
    {
        TaskTpl::Ptr taskPtr = TaskBase::me().getTaskTpl(iter.first);
        if(nullptr == taskPtr)
        {
            taskPtr = TaskBase::me().getFactionTaskTpl(iter.first);
            if(nullptr == taskPtr)
                continue;
        }

        bool allstepFinished = true;
        bool refreshAcceptedTaskInfo = false;
        auto& steps = iter.second.steps;
        uint32_t stepSize = steps.size();
        for(uint32_t i = 0; i < stepSize; ++i)
        {
            if(steps[i].state == 1) //已经完成
                continue;
            if(steps[i].content != TaskContent::kill_npc)
            {
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }

            if(taskPtr->taskContentCfgs[i].param3 > 0) //说明为击杀等级段npc任务
            {
                if(param.element.npclevel >= taskPtr->taskContentCfgs[i].param1
                   && param.element.npclevel <= taskPtr->taskContentCfgs[i].param2)
                {
                    steps[i].count += 1;
                    if(steps[i].count >= taskPtr->taskContentCfgs[i].param3)
                        steps[i].state = 1;
                }
                else
                {
                    allstepFinished = false;
                    if(taskPtr->order)
                        break;
                    continue;
                }
            }
            else
            {
                if(param.element.npcTplId == taskPtr->taskContentCfgs[i].param1)
                {
                    steps[i].count += 1;
                    if(steps[i].count >= taskPtr->taskContentCfgs[i].param2)
                        steps[i].state = 1;
                }
                else
                {
                    allstepFinished = false;
                    if(taskPtr->order)
                        break;
                    continue;
                }
            }

            refreshAcceptedTaskInfo = true;
            if(steps[i].state == 0)
                allstepFinished = false;
        }

        if(allstepFinished)
            finishedTasks.push_back(iter.first);
        else if(refreshAcceptedTaskInfo)
            refreshAcceptedTaskDetails(iter.first);
    }

    for(const auto& taskId : finishedTasks)
    {
        changeTaskState(taskId, TaskState::finished);
    }
}

void RoleTask::onPassCopy(TaskParam param)
{
    for(auto& iter : m_acceptedTasks)
    {
        TaskTpl::Ptr taskPtr = TaskBase::me().getTaskTpl(iter.first);
        if(nullptr == taskPtr)
        {
            taskPtr = TaskBase::me().getFactionTaskTpl(iter.first);
            if(nullptr == taskPtr)
                continue;
        }

        bool allstepFinished = true;
        auto& steps = iter.second.steps;
        uint32_t stepSize = steps.size();
        for(uint32_t i = 0; i < stepSize; ++i)
        {
            if(steps[i].state == 1) //已经完成
                continue;
            if(steps[i].content != TaskContent::pass_copy)
            {
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }

            if(param.element.mapId != taskPtr->taskContentCfgs[i].param1)
            {
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }

            steps[i].state = 1;
        }

        if(allstepFinished)
        {
            changeTaskState(iter.first, TaskState::finished);
            break;
        }
    }
}

void RoleTask::onCollection(TaskParam param)
{
    for(auto& iter : m_acceptedTasks)
    {
        TaskTpl::Ptr taskPtr = TaskBase::me().getTaskTpl(iter.first);
        if(nullptr == taskPtr)
        {
            taskPtr = TaskBase::me().getFactionTaskTpl(iter.first);
            if(nullptr == taskPtr)
                continue;
        }

        bool allstepFinished = true;
        bool refreshAcceptedTaskInfo = false;
        auto& steps = iter.second.steps;
        uint32_t stepSize = steps.size();
        for(uint32_t i = 0; i < stepSize; ++i)
        {
            if(steps[i].state == 1) //已经完成
                continue;
            if(steps[i].content != TaskContent::collection)
            {
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }

            Npc::Ptr npc = NpcManager::me().getById(param.element.npcId);
            if(nullptr == npc || npc->tplId() != taskPtr->taskContentCfgs[i].param1)
            {
                allstepFinished = false;
                if(taskPtr->order)
                    break;
                continue;
            }

            steps[i].count += 1;
            if(steps[i].count >= taskPtr->taskContentCfgs[i].param2)
                steps[i].state = 1;

            refreshAcceptedTaskInfo = true;
            if(steps[i].state == 0)
                allstepFinished = false;
        }

        if(allstepFinished)
        {
            changeTaskState(iter.first, TaskState::finished);
            break;
        }

        if(refreshAcceptedTaskInfo)
            refreshAcceptedTaskDetails(iter.first);
    }
}

TaskState RoleTask::getFactionTaskState(TaskId taskId) const
{
    auto accepted = m_acceptedTasks.find(taskId);
    if(accepted != m_acceptedTasks.end())
        return TaskState::accepted;

    auto finish = m_finishedTasks.find(taskId);
    if(finish != m_finishedTasks.end())
        return TaskState::finished;

    return TaskState::no;
}

void RoleTask::redoTask(TaskId taskId)
{
    m_acceptedTasks.erase(taskId);
    m_finishedTasks.erase(taskId);
    m_overTasks.erase(taskId);

    changeTaskState(taskId, TaskState::accepted);
}

void RoleTask::giveTaskAward(TaskTpl::Ptr taskPtr)
{
    for(const auto& obj : taskPtr->taskObjAwards)
    {
        if(Job::none != obj.job && m_owner.job() != obj.job)
            continue;
        m_owner.putObj(obj.objId, obj.objNum, obj.bind, PackageType::role);
    }
}

void RoleTask::loadFromDB(const std::string& taskStr)
{
	std::vector<TaskInfo> dailyTaskVec;

	Deserialize<std::string> ds(&taskStr);
	uint32_t acceptedTaskSize = 0;
	ds >> acceptedTaskSize;
	for(uint32_t i = 0; i < acceptedTaskSize; ++i)
	{
		TaskInfo info;
		ds >> info.taskId;
		ds >> info.type;
		ds >> info.state;
		ds >> info.time;
		ds >> info.star;
		ds >> info.steps;
		m_acceptedTasks.insert(std::make_pair(info.taskId, info));

		if(info.type == TaskType::daily)
		{
			dailyTaskVec.push_back(info);
		}
	}
	
	uint32_t finishedTaskSize = 0;
	ds >> finishedTaskSize;
	for(uint32_t i = 0; i < finishedTaskSize; ++i)
	{
		TaskInfo info;
		ds >> info.taskId;
		ds >> info.type;
		ds >> info.state;
		ds >> info.time;
		ds >> info.star;
		ds >> info.steps;
		m_finishedTasks.insert(std::make_pair(info.taskId, info));

		if(info.type == TaskType::daily)
		{
			dailyTaskVec.push_back(info);
		}
	}
	ds >> m_overTasks;

	uint32_t acceptableDailyTaskSize = 0;
	ds >> acceptableDailyTaskSize;
	for(uint32_t i = 0; i < acceptableDailyTaskSize; ++i)
	{
		TaskInfo info;
		ds >> info.taskId;
		ds >> info.type;
		ds >> info.state;
		ds >> info.time;
		ds >> info.star;
		ds >> info.steps;
		m_acceptableDailyTasks.insert(std::make_pair(info.taskId, info));
		dailyTaskVec.push_back(info);
	}
	ds >> m_dailyTaskTopStarReward;

	m_owner.m_dailyTask.loadFromDB(dailyTaskVec);
	return;
}

void RoleTask::updateAllTaskInfoToDB() const
{
    Serialize<std::string> ss;
    ss.reset();
    ss << static_cast<uint32_t>(m_acceptedTasks.size());
    for(const auto& iter : m_acceptedTasks)
    {
        ss << iter.second.taskId;
        ss << iter.second.type;
        ss << iter.second.state;
		ss << iter.second.time;
		ss << iter.second.star;
        ss << iter.second.steps;
    }
    
	ss << static_cast<uint32_t>(m_finishedTasks.size());
    for(const auto& iter : m_finishedTasks)
    {
        ss << iter.second.taskId;
        ss << iter.second.type;
        ss << iter.second.state;
		ss << iter.second.time;
		ss << iter.second.star;
        ss << iter.second.steps;
    }
	ss << m_overTasks;
	
	ss << static_cast<uint32_t>(m_acceptableDailyTasks.size());
	for(const auto& iter : m_acceptableDailyTasks)
	{
        ss << iter.second.taskId;
        ss << iter.second.type;
        ss << iter.second.state;
		ss << iter.second.time;
		ss << iter.second.star;
        ss << iter.second.steps;
	}
	ss << m_dailyTaskTopStarReward;

    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PrivateRaw::UpdateAllTaskInfoToDB)+ss.tellp()); 
    auto msg = reinterpret_cast<PrivateRaw::UpdateAllTaskInfoToDB*>(buf.data());
    msg->roleId = m_owner.id();
    msg->size = ss.tellp();
    std::memcpy(msg->buf,ss.buffer()->data(),ss.tellp());

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateAllTaskInfoToDB), buf.data(), buf.size());
}

void RoleTask::afterEnterScene()
{
    notifyNextUnacceptableMainTask();

    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::RefreshTaskState));
    auto msg = reinterpret_cast<PublicRaw::RefreshTaskState*>(buf.data());
    msg->modify = false;
    for(const auto& iter : m_acceptableTasks)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RefreshTaskState::TaskStateEle));
        auto msg = reinterpret_cast<PublicRaw::RefreshTaskState*>(buf.data());
        msg->data[msg->size].taskId = iter;
        msg->data[msg->size].state = TaskState::acceptable;
        ++msg->size;
    }
    for(const auto& iter : m_finishedTasks)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RefreshTaskState::TaskStateEle));
        auto msg = reinterpret_cast<PublicRaw::RefreshTaskState*>(buf.data());
        msg->data[msg->size].taskId = iter.second.taskId;
        msg->data[msg->size].state = TaskState::finished;
        ++msg->size;
    }
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshTaskState), buf.data(), buf.size());

    for(const auto& iter : m_acceptedTasks)
    {
        refreshAcceptedTaskDetails(iter.first, false);
    }
}

void RoleTask::beforeLeaveScene() const
{
	updateAllTaskInfoToDB();
}

void RoleTask::setDailyTaskStar(TaskId taskId, uint8_t star)
{
	auto pos = m_acceptableDailyTasks.find(taskId);
	if(pos == m_acceptedTasks.end())
		return;

	pos->second.star = star;
	updateAllTaskInfoToDB();
	return;
}

void RoleTask::clearDailyTaskTopStarReward()
{
	m_dailyTaskTopStarReward.clear();
	return;
}

void RoleTask::setDailyTaskTopStarRewardState(uint32_t num, Reward rewardState)
{
	m_dailyTaskTopStarReward[num] = rewardState;
	return;
}

const std::unordered_map<uint32_t, Reward>& RoleTask::getDailyTaskTopStarReward() const
{
	return m_dailyTaskTopStarReward;
}


}
