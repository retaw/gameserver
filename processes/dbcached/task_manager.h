#ifndef PROCESS_DBCACHED_TASK_MANAGER_HPP
#define PROCESS_DBCACHED_TASK_MANAGER_HPP


#include "water/common/roledef.h"

namespace dbcached{

class TaskManager
{
    friend class RoleManager; 
public:
    ~TaskManager() = default;
    static TaskManager& me();

    void regMsgHandler();

private:
    TaskManager() = default;
    void servermsg_UpdateAllTaskInfoToDB(const uint8_t* msgData, uint32_t msgSize);

    //帮派任务
    void servermsg_UpdateFactionTaskInfo(const uint8_t* msgData, uint32_t msgSize);

public:
	std::string getTaskInfoByRoleId(RoleId roleId);
    std::string getFactionTaskInfo(RoleId roleId);
};

}
#endif
