#include "task_base.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace world{

TaskBase::TaskBase()
{
}

TaskBase& TaskBase::me()
{
    static TaskBase me;
    return me;
}

void TaskBase::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgdir + "/task.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        TaskTpl::Ptr taskTpl = TaskTpl::create();
        taskTpl->taskId = itemNode.getChildNodeText<TaskId>("taskid");
        taskTpl->type = static_cast<TaskType>(itemNode.getChildNodeText<uint8_t>("type"));
        taskTpl->preTaskId = itemNode.getChildNodeText<TaskId>("pre_taskid");
        taskTpl->autoAccept = itemNode.getChildNodeText<uint8_t>("auto_accept");
        taskTpl->minLevel = itemNode.getChildNodeText<uint32_t>("min_level");
        taskTpl->maxLevel = itemNode.getChildNodeText<uint32_t>("max_level");
        taskTpl->factionLevel = itemNode.getChildNodeText<uint16_t>("faction_level");
        taskTpl->acceptNpc = itemNode.getChildNodeText<TplId>("accept_npc");
        taskTpl->submitNpc = itemNode.getChildNodeText<TplId>("submit_npc");

        taskTpl->taskObj = itemNode.getChildNodeText<TplId>("task_obj");
        taskTpl->order = itemNode.getChildNodeText<uint8_t>("order");

        std::string str;
        std::vector<std::string> subvs;
        str = itemNode.getChildNodeText<std::string>("task_content");
        std::vector<std::string> vs = componet::splitString(str, ",");
        for(const auto& iter : vs)
        {
            subvs.clear();
            subvs = componet::splitString(iter, "-");
            if(subvs.size() < 4)
                continue;

            TaskContentCfg cfg;
            cfg.content = static_cast<TaskContent>(atoi(subvs[0].c_str()));
            cfg.param1 = componet::fromString<uint32_t>(subvs[1]);
            cfg.param2 = componet::fromString<uint32_t>(subvs[2]);
            cfg.param3 = componet::fromString<uint32_t>(subvs[3]);

            LOG_DEBUG("任务, taskId={}, 加载任务内容, TaskContentCfg:({}, {}, {}, {})", taskTpl->taskId, cfg.content, cfg.param1, cfg.param2, cfg.param3);
            taskTpl->taskContentCfgs.push_back(cfg);
        }

        str.clear();
        vs.clear();
        str = itemNode.getChildNodeText<std::string>("task_award");
        vs = componet::splitString(str, ",");
        for(const auto& iter : vs)
        {
            subvs.clear();
            subvs = componet::splitString(iter, "-");
            if(subvs.size() < 4)
                continue;

            TaskAward award;
            award.objId = componet::fromString<TplId>(subvs[0]);
            award.objNum = componet::fromString<uint32_t>(subvs[1]);
            award.bind = static_cast<Bind>(atoi(subvs[2].c_str()));
            award.job = static_cast<Job>(atoi(subvs[3].c_str()));

            taskTpl->taskObjAwards.push_back(award);
        }

        //帮派任务区别大，分到不同容器
        if(taskTpl->type == TaskType::faction)
        {
            m_factionTaskCfgs.emplace(taskTpl->taskId, taskTpl);
        }
        else
		{
            m_taskCfgs.insert({taskTpl->taskId, taskTpl});
    
		}
	}
}

TaskTpl::Ptr TaskBase::getTaskTpl(TaskId taskId)
{
    if(m_taskCfgs.find(taskId) == m_taskCfgs.end())
        return nullptr;

    return m_taskCfgs[taskId];
}

TaskTpl::Ptr TaskBase::getFactionTaskTpl(TaskId taskId)
{
    if(m_factionTaskCfgs.find(taskId) == m_factionTaskCfgs.end())
        return nullptr;
    return m_factionTaskCfgs[taskId];
}

void TaskBase::execTaskTpls(std::function<bool (TaskTpl::Ptr)> exec)
{
    for(const auto& iter : m_taskCfgs)
    {
        if(exec(iter.second))
            break;
    }
}

const std::unordered_map<TaskId, TaskTpl::Ptr>& TaskBase::getTaskCfg() const
{
	return m_taskCfgs;
}

}
