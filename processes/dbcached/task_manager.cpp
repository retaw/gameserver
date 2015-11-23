#include "task_manager.h"
#include "role_manager.h"
#include "task_table_structure.h"

#include "water/componet/serialize.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

#include "protocol/rawmsg/private/task.h"
#include "protocol/rawmsg/private/task.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"


namespace dbcached{

using namespace water::componet;

TaskManager& TaskManager::me()
{
    static TaskManager me;
    return me;
}

void TaskManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(UpdateAllTaskInfoToDB, std::bind(&TaskManager::servermsg_UpdateAllTaskInfoToDB, this, _1, _2));
    REG_RAWMSG_PRIVATE(UpdateFactionTaskInfo, std::bind(&TaskManager::servermsg_UpdateFactionTaskInfo, this, _1, _2));
}

void TaskManager::servermsg_UpdateFactionTaskInfo(const uint8_t* msgData, uint32_t msgSize)
{
    auto msg = reinterpret_cast<const PrivateRaw::UpdateFactionTaskInfo*>(msgData);
    std::string data;
    data.append(msg->data, msg->size);

    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfFactionTask taskRow(msg->roleId, data);
        query.replace(taskRow);
        query.execute();
    }
    catch(const mysqlpp::Exception& err)
    {
        LOG_ERROR("帮派任务, 入库失败");
        return;
    }
    RoleManager::me().m_contrRoles.updateFactionTaskState(msg->roleId, data);
}

void TaskManager::servermsg_UpdateAllTaskInfoToDB(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateAllTaskInfoToDB*>(msgData);
	if(!rev)
		return;

    std::string blobStr("");
    blobStr.append((char*)rev->buf, rev->size);
    try
    {
		mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
		RowOfTask taskRow(rev->roleId, blobStr);
		query.replace(taskRow);
		query.execute();
    }
    catch(const mysqlpp::Exception& err)
    {
        LOG_DEBUG("任务, modifyTaskInfo失败, err={}", err.what());
        return;
    }

    RoleManager::me().m_contrRoles.updateAllTaskInfo(rev->roleId, blobStr);
}


std::string TaskManager::getTaskInfoByRoleId(RoleId roleId)
{
    std::string taskStr("");
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select * from task where roleId = ";
        query << sql << roleId;
        std::vector<RowOfTask> res;
        query.storein(res);
        if(res.empty())
        {
			LOG_DEBUG("任务, load()空, 请求roleId={}",roleId); 
        }
        else
		{
			taskStr = res[0].blobStr;
        }
    }
	return taskStr;
}

std::string TaskManager::getFactionTaskInfo(RoleId roleId)
{
    std::string ret;
    ret.clear();
    std::vector<RowOfFactionTask> vec;
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from factionTask where roleId = " << roleId;
        query.storein(vec);
    }
    if(vec.empty())
        return ret;
    return vec[0].data;
}

}
