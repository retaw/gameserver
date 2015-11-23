#ifndef PROCESS_WORLD_FACTIONTASK_H
#define PROCESS_WORLD_FACTIONTASK_H


#include "task_base.h"
#include "water/common/roledef.h"
#include <unordered_map>
#include <unordered_set>

namespace world{

struct FactionTaskInfo
{
    TaskId taskId = 0;
    TaskState state;
    uint32_t time = 0;
    FactionTaskReward reward;
    Bind bind;
    uint16_t taskRecord = 0;
};

class Role;

class RoleFactionTask
{
public:
    RoleFactionTask(Role& me);
    ~RoleFactionTask() = default;

    void afterEnterScene();
    void beforeLeaveScene();

    void acceptTask();
    bool taskOutTime();
    void refreshTask(bool needJudgeTime = true);
    void finishTask();
    void quitTask();
    void buyVipGift(uint32_t rewardId);
    void sendRetFactionTask();
    void clear();

public:
    void initInfo(std::string& taskInfos);

private:
    void savetaskInfo();
    //bool confChange();
    void giveReward();
    void sendFinishedTaskNum();

private:
    Role& role;
public:
    FactionTaskInfo m_taskInfo;
};


}

#endif
