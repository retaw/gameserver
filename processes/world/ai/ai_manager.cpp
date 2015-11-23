#include "ai_manager.h"

#include "ai_action.h"
#include "ai_condition.h"
#include "ai_factory.h"

#include "npc_manager.h"
#include "npc.h"

#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"

namespace world{
namespace ai{

/************************************** AIManager **************************************/
AIManager& AIManager::me()
{
    static AIManager me;
	return me;
}

AIManager::AIManager()
{
	// 初始化AIAction 和 AICondition的工厂
	registerAction();
	registerCondition();
}

AIManager::~AIManager()
{
	clear();
}

bool AIManager::loadConfig(const std::string& cfgDir)
{
    clear();

    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgDir + "/ai.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    // AI节点
    for(XmlParseNode aiNode = root.getChild("ai"); aiNode; ++aiNode)
    {
        AI::Ptr ai = AI::create(aiNode.getAttr<uint32_t>("id"));
        if(0 == ai->tplId())
            continue;

//        ai->disableDurationAfterBorn = aiNode.getAttr<uint32_t>("delay_ms");

        // AI::condition 事件过滤的原子条件
        for(XmlParseNode conditionNode = aiNode.getChild("condition"); conditionNode; ++conditionNode)
        {
            char conditionID = conditionNode.getAttr<std::string>("id").c_str()[0];	// 通过c_str取，空串时不越界
            if('\0' == conditionID)
                break;

            const std::string conditionName = conditionNode.getAttr<std::string>("name");
            AICondition::Ptr condition(conditionFactory.produce(conditionName));
            if(condition == nullptr)
            {
                LOG_TRACE("[AI:%u], condition %c, %s 不存在",
                                     ai->tplId(), conditionID, conditionName.c_str());
                continue;
            }
            condition->setId(conditionID);
            condition->parseCfg(conditionNode);
            ai->m_conditions.insert(std::make_pair(conditionID, condition)).second;
        }

        // AI::AIItem
        for(XmlParseNode itemNode = aiNode.getChild("item"); itemNode; ++itemNode)
        {
            uint16_t actionID = 0;
            AI::AIItem item;
            // AIItem::trigger
            XmlParseNode triggerNode = itemNode.getChild("trigger");
            if(triggerNode)
                item.trigger = AITrigger::create(ai->m_conditions, triggerNode.getAttr<std::string>("expression"));
            // AIItem::action
            for(XmlParseNode actionNode = itemNode.getChild("action"); actionNode; ++actionNode)
            {
                std::string actionName = actionNode.getAttr<std::string>("name");
                AIAction::Ptr action(actionFactory.produce(actionName));
                if(action == nullptr)
                {
                    LOG_ERROR("[AI%u], action %s 不存在", ai->tplId(), actionName.c_str());
                    continue;
                }
                action->setId(++actionID);
                action->parseCfg(actionNode);
                item.actions.push_back(action);
            }
            item.actions.shrink_to_fit();

            // 存储本AIItem节点
            ai->m_items.push_back(item);
        }
        // 存储本AI节点
        m_ais.insert(std::make_pair(ai->tplId(), ai));
    }
    return true;
}

void AIManager::clear()
{
	m_ais.clear();	
}

AI::Ptr AIManager::create(uint32_t tplId)
{
	auto iter = m_ais.find(tplId);
	if(iter == m_ais.end())
		return nullptr;

	return iter->second->clone();
}

/*
uint64_t AIManager::activeDelay(uint32_t tplId)
{
	AI* ai = get(tplId);
	if(ai == nullptr)
		return 0;

	return ai->m_activeDelay;
}

*/

}}

