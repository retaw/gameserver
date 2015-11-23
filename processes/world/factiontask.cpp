#include "factiontask.h"
#include "role.h"
#include "world.h"
#include "factionactive_manager.h"
#include "reward_manager.h"
#include "mail_manager.h"

#include "protocol/rawmsg/private/task.h"
#include "protocol/rawmsg/private/task.codedef.private.h"

#include "protocol/rawmsg/public/task.h"
#include "protocol/rawmsg/public/task.codedef.public.h"
#include "protocol/rawmsg/public/faction_active.h"
#include "protocol/rawmsg/public/faction_active.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

#include "water/componet/logger.h"
#include "water/componet/serialize.h"
#include "water/componet/datetime.h"
#include "water/componet/random.h"


namespace world{

RoleFactionTask::RoleFactionTask(Role& me)
: role(me)
{}

void RoleFactionTask::initInfo(std::string& taskInfos)
{
    if(!taskInfos.empty())
    {
        Deserialize<std::string> ds(&taskInfos);
        ds >> m_taskInfo.taskId;
        ds >> m_taskInfo.time;
        ds >> m_taskInfo.taskRecord;
    }

    if(taskOutTime())
        m_taskInfo.taskRecord = 0;

    if(m_taskInfo.taskRecord != 0)
    {
        auto taskPtr = TaskBase::me().getFactionTaskTpl(m_taskInfo.taskRecord);//如果此帮派任务在配置中不尊在则要去掉，因为任务机制中是去掉的，需要保持同步
        if(nullptr == taskPtr)
            m_taskInfo.taskId = 0;
    }

    //从task中得到任务状态(只有接受的或者完成的任务会会再task中，而其它装填返回none)
    m_taskInfo.state = role.m_roleTask.getFactionTaskState(m_taskInfo.taskId);
    if(m_taskInfo.state == TaskState::no)
    {
        m_taskInfo.state = TaskState::acceptable;
        if(m_taskInfo.taskRecord >= FactionActiveManager::me().m_taskNum)
            m_taskInfo.state = TaskState::over;
    }

}

void RoleFactionTask::savetaskInfo()
{
    Serialize<std::string> ss;
    ss.reset();
    ss << m_taskInfo.taskId;
    ss << m_taskInfo.time;
    ss << m_taskInfo.taskRecord;

    std::vector<uint8_t> buf;
    buf.reserve(512);
    buf.resize(sizeof(PrivateRaw::UpdateFactionTaskInfo) + ss.tellp());
    auto msg = reinterpret_cast<PrivateRaw::UpdateFactionTaskInfo*>(buf.data());
    msg->roleId = role.id();
    msg->size = ss.tellp();
    std::memcpy(msg->data, ss.buffer()->data(),ss.tellp());
    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateFactionTaskInfo), buf.data(), buf.size());
}

//void RoleFactionTask::sendTaskState(bool refresh/*true*/)
/*{
    if(m_taskInfo.taskId == 0)
        return;
    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::RefreshTaskState) + sizeof(PublicRaw::RefreshTaskState::TaskStateEle));
    auto msg = reinterpret_cast<PublicRaw::RefreshTaskState*>(buf.data());
    msg->data[0].taskId = m_taskInfo.taskId;
    msg->data[0].state = m_taskInfo.state;
    msg->size = 1;
    msg->modify = refresh;
    role.sendToMe(RAWMSG_CODE_PUBLIC(RefreshTaskState), buf.data(), buf.size());
}

void RoleFactionTask::sendTaskDetails(bool refresh)
{
    if(m_taskInfo.taskId == 0)
        return;
    if(m_taskInfo.state != TaskState::accepted)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::RefreshAcceptedTaskDetails));
    auto msg = reinterpret_cast<PublicRaw::RefreshAcceptedTaskDetails*>(buf.data());
    msg->taskId = m_taskInfo.taskId;
    msg->modify = refresh;
    
    for(auto& step : m_taskInfo.steps)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RefreshAcceptedTaskDetails::TaskSteps));
        auto msg = reinterpret_cast<PublicRaw::RefreshAcceptedTaskDetails*>(buf.data());
        msg->data[msg->size].content = step.content;
        msg->data[msg->size].state = step.state;
        msg->data[msg->size].npcCount = step.count;
        msg->data[msg->size].reserve = step.reserve;
        ++msg->size;
    }

    role.sendToMe(RAWMSG_CODE_PUBLIC(RefreshAcceptedTaskDetails), buf.data(), buf.size());
}
*/

void RoleFactionTask::beforeLeaveScene()
{
    savetaskInfo();
}

void RoleFactionTask::acceptTask()
{
    if(m_taskInfo.taskId == 0)
    {
        role.sendSysChat("今日帮派任务已经完成");
        return;
    }
    
    if(m_taskInfo.state != TaskState::acceptable)
    {
        role.sendSysChat("任务不是可接受状态");
        return;
    }

    m_taskInfo.state = TaskState::accepted;
    
    //加入task的已接取任务容器中,要保证task与factiontask同步
    role.m_roleTask.changeTaskState(m_taskInfo.taskId, m_taskInfo.state);

    //sendTaskDetails();
}

void RoleFactionTask::clear()
{
    m_taskInfo.state = TaskState::quit;
    role.m_roleTask.changeTaskState(m_taskInfo.taskId, m_taskInfo.state); 
    uint16_t taskRecord = m_taskInfo.taskRecord;
    std::memset(&m_taskInfo, 0, sizeof(m_taskInfo));
    m_taskInfo.taskRecord = taskRecord;
}

bool RoleFactionTask::taskOutTime()
{
    if(m_taskInfo.time == 0)
        return false;
    return !inSameDay(water::componet::Clock::from_time_t(m_taskInfo.time), water::componet::Clock::now());
}

void RoleFactionTask::afterEnterScene()
{}

//needJudgeTime=false时必须自己判断是否是同一天，并给time赋值
void RoleFactionTask::refreshTask(bool needJudgeTime/*=true*/)
{
    //刷新新任务时判断时间是否为同一天
    if(taskOutTime() && needJudgeTime)
    {
        m_taskInfo.taskRecord = 0;
        m_taskInfo.time = time(NULL);    
    }

    //是否达到今日任务上限
    if(m_taskInfo.taskRecord > FactionActiveManager::me().m_taskNum)
    {
        role.sendSysChat("今日帮派任务次数已达上限");
        return;
    }

    std::vector<TaskTpl::Ptr> taskIdVec;
    for(auto& it : TaskBase::me().m_factionTaskCfgs)
    {
        if(role.level() <= it.second->maxLevel && role.level() >= it.second->minLevel && role.factionLevel() >= it.second->factionLevel)
            taskIdVec.insert(taskIdVec.end(), it.second);
    }
    if(taskIdVec.empty())
        return;
    water::componet::Random<uint32_t> rand(1, taskIdVec.size());
    auto& taskPtr = taskIdVec[rand.get() - 1];
    if(taskPtr == nullptr)
        return;
    m_taskInfo.taskId = taskPtr->taskId;
    m_taskInfo.state = TaskState::acceptable;
    FactionActiveManager::me().fillFactionTaskReward(role);
}

void RoleFactionTask::quitTask()
{
    if(m_taskInfo.state != TaskState::accepted)
        return;

    m_taskInfo.state = TaskState::quit;
    m_taskInfo.taskRecord++;
    sendFinishedTaskNum();
    role.m_roleTask.changeTaskState(m_taskInfo.taskId, m_taskInfo.state);
    if(m_taskInfo.taskRecord >= FactionActiveManager::me().m_taskNum)
    {
        m_taskInfo.taskId = 0;
        return;
    }
    refreshTask();
    sendRetFactionTask();
    savetaskInfo();
}

void RoleFactionTask::finishTask()//应是提交任务overTask
{
   if(m_taskInfo.state != TaskState::finished) 
       return;

   m_taskInfo.state = TaskState::over;
   //更新任务状态
   role.m_roleTask.changeTaskState(m_taskInfo.taskId, m_taskInfo.state);
   //发奖励
   giveReward();
   //更新taskRecord；
   m_taskInfo.taskRecord++;
   //发送任务完成数量
   sendFinishedTaskNum();
   if(m_taskInfo.taskRecord >= FactionActiveManager::me().m_taskNum)
   {
       m_taskInfo.taskId = 0;
       return;
   }
   //刷新任务
   refreshTask();
   //发送新的任务
   sendRetFactionTask();
   savetaskInfo();
}

void RoleFactionTask::buyVipGift(uint32_t rewardId)
{
    //if(role->vip < 1)
      //  return;

    //验证并花钱
    if(!role.checkMoney(FactionActiveManager::me().m_vipBuyMoneytype, FactionActiveManager::me().m_vipBuyPriceNum))
    {
        return;
    }
    role.reduceMoney(FactionActiveManager::me().m_vipBuyMoneytype, FactionActiveManager::me().m_vipBuyPriceNum, "帮派任务礼包领取");
    
    //发奖励
	std::vector<ObjItem> objVec;
	if(!RewardManager::me().getFixReward(rewardId, 1, role.level(), role.job(), objVec))
	{
		LOG_ERROR("帮派任务, 获取固定奖励失败, 领取帮派任务奖励失败, name={}, roleId={}, level={}, job={}, rewardId={}",
				  role.name(), role.id(), role.level(), role.job(), rewardId);
		return;
	}
	
	if(!objVec.empty() && !role.checkPutObj(objVec))
	{
		std::string text = "由于背包空间不足, 通过邮件发放帮派任务奖励, 请注意查收";
		MailManager::me().send(role.id(), "帮派任务奖励", text, objVec);
	}
	else
	{
		role.putObj(objVec);
	}

    bool timeOut = taskOutTime();//先判断是否已经不是同一天
    if(timeOut)
        m_taskInfo.taskRecord = 0;

    //已经做完今日任务
    if(m_taskInfo.taskRecord >= FactionActiveManager::me().m_taskNum)
    {
        return;
    }

    //如果当前任务是接受或者完成状态，要更改task内的任务状态为over
    if(m_taskInfo.state == TaskState::finished || m_taskInfo.state == TaskState::accepted)
    {
        role.m_roleTask.changeTaskState(m_taskInfo.taskId, TaskState::over);
    }

    //完成当前任务
    FactionActiveManager::me().fillFactionTaskReward(role);
    giveReward();//给奖励
    m_taskInfo.taskRecord++;

    //完成剩余任务
    uint16_t needToFinishNum = FactionActiveManager::me().m_taskNum - m_taskInfo.taskRecord;
    uint32_t num = 0;
    while(num  < needToFinishNum)
    {
        refreshTask();  //如果中间出现跨天,那record会清零,循环后m_taskInfo.taskRecord将不是最大值
        giveReward();
        m_taskInfo.taskRecord++;
        num++;
    }
    //如果当前record不等最大任务量，那就是循环的过程发生了跨天处理，record要清零,发送新的任务信息给客户端
    if(FactionActiveManager::me().m_taskNum != m_taskInfo.taskRecord)
    {
        refreshTask();
        m_taskInfo.taskRecord = 0;
        role.sendSysChat("前日帮派任务全部完成, 请继续完成今日任务");
        return;
    }
    //今日完成所有任务后的状态
    m_taskInfo.state = TaskState::over;
    m_taskInfo.taskId = 0;

    sendFinishedTaskNum();
    sendRetFactionTask();
    savetaskInfo();
    role.sendSysChat("帮派任务全部完成");
}

void RoleFactionTask::giveReward()
{
    std::string text = "获得帮派任务奖励:";
    if(m_taskInfo.reward.roleExp != 0)
    {
        text = text + water::componet::format("角色经验{} ", m_taskInfo.reward.roleExp);
        role.addExp(m_taskInfo.reward.roleExp);
    }

    if(m_taskInfo.reward.heroExp != 0)
    {
        auto hero = role.m_heroManager.getDefaultHero();
        if(hero != nullptr)
        {
            text = text + water::componet::format("英雄经验{} ", m_taskInfo.reward.heroExp);
            hero->addExp(m_taskInfo.reward.heroExp);
        }
    }

    if(m_taskInfo.reward.factionExp != 0 || m_taskInfo.reward.factionResource != 0 || m_taskInfo.reward.banggong != 0)
    {
        text = text + water::componet::format("帮派经验{} ", m_taskInfo.reward.factionExp);
        role.addFaction(m_taskInfo.reward.factionExp, m_taskInfo.reward.factionResource, m_taskInfo.reward.banggong);
    }

    bool objMail = false;
    if(m_taskInfo.reward.objNum != 0)
    {
        if(role.checkPutObj(m_taskInfo.reward.objTplId, m_taskInfo.reward.objNum, m_taskInfo.bind))
        {
            text = text + water::componet::format("英雄道具");
            role.putObj(m_taskInfo.reward.objTplId, m_taskInfo.reward.objNum, m_taskInfo.bind, PackageType::role);
        }
        else
        {
            objMail = true;
            std::vector<ObjItem> objVec;
            ObjItem temp;
            temp.tplId = m_taskInfo.reward.objTplId;
            temp.num = m_taskInfo.reward.objNum;
            temp.bind = m_taskInfo.bind;
            objVec.push_back(std::move(temp));
            std::string text = "您顺利完成帮派任务, 获得帮派任务奖励";
            MailManager::me().send(role.id(), "帮派任务奖励", text, objVec);
        }
    }
    role.sendSysChat(text);
    if(objMail)
    {
        role.sendSysChat("背包空间不足, 奖励通过邮件发放");
    }

}

void RoleFactionTask::sendRetFactionTask()
{
    //未初始或者已经做完任务
    if(m_taskInfo.taskId == 0)
    {
        if(m_taskInfo.state == TaskState::over)
        {
            role.sendSysChat("今日帮派任务已经做完");
            return;
        }
        else
        {
            refreshTask();
        }
    }

    PublicRaw::RetFactionTask send;
    send.taskId = role.m_roleFactionTask.m_taskInfo.taskId;
    send.state = role.m_roleFactionTask.m_taskInfo.state;
    send.reward = role.m_roleFactionTask.m_taskInfo.reward;
    role.sendToMe(RAWMSG_CODE_PUBLIC(RetFactionTask), &send, sizeof(send));
}

void RoleFactionTask::sendFinishedTaskNum()
{
    PublicRaw::FinishedTaskNum send;
    send.num = m_taskInfo.taskRecord;
    role.sendToMe(RAWMSG_CODE_PUBLIC(FinishedTaskNum), &send, sizeof(send));
}


}
