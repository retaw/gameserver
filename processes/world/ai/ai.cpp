#include "ai.h"
#include "ai_factory.h"

#include "npc.h"
#include "npc_manager.h"

namespace world{
namespace ai{

/************************************** AITrigger **************************************/
AITrigger::AITrigger(std::map<char, AICondition::Ptr>& conditions, const std::string& expression)
: m_conditions(conditions)
, m_expression(expression)
{
}


const std::string& AITrigger::expression() const
{
    return m_expression;
}

bool AITrigger::check(Npc* npc, const AIEvent* event)
{
	if(!npc)
		return false;

	// 每个表达式计算前，先清空缓存的结果
	m_conditionValues.clear();

	// 初始化解析器
	m_npc = npc;
    m_event = event;

	m_pos = 0;
	m_cur = Eof;
	m_next = Eof;
	m_grammarError = false;

	// 解释执行表达式
	bool ret = E();
	if(m_grammarError)
	{
		LOG_TRACE("ai解析出错，pos={}, expression='{}'", m_pos - 1, m_expression.c_str());
		ret = false;
	}
	return ret;
}

void AITrigger::read()
{
	if(m_expression.length() <= m_pos)	// 没得读了
	{
		m_cur = Eof;
		m_next = Eof;
	}
	else
	{
		m_cur = m_expression[m_pos];
		m_next = (m_pos + 1 == m_expression.length()) ? Eof : m_expression[m_pos + 1];
		++m_pos;
	}
}

bool AITrigger::isCondition()
{
	return std::isalnum(m_cur);
}


bool AITrigger::E()
{
	if(m_grammarError)	// 已经出错了，不用再分析了
		return false;
	
	bool ret = F();
	
	if(m_next == '|')	// E = F | E
	{
		read();
		ret = ret || E();
	}
	else	// E= F
	{
	}

	return ret;
}

bool AITrigger::F()
{
	if(m_grammarError)
		return false;

	bool ret = T();
	if(m_next == '*')
	{
		read();
		ret = ret && E();
	}
	else   // F = T
	{
	}

	return ret;
}

bool AITrigger::T()
{
	if(m_grammarError)
		return false;

	bool ret = false;
	read();
	if(m_cur == '!')	// T = !T
	{
		ret = !T();
	}
	else if(m_cur == '(')	// T = (E)
	{
		ret = E();
		read();
		if(m_cur != ')')
		{
			m_grammarError = true;
			return false;
		}
	}
	else if(isCondition())	// T = c
    {
        std::map<char, bool>::iterator v = m_conditionValues.find(m_cur);
        if(v != m_conditionValues.end())	// 此condition已经在本表达式中求值过，使用缓存的结果，不再重复计算
        {
            ret = v->second;
        }
        else	// 计算condition
        {
            auto iter = m_conditions.find(m_cur);
            if(iter != m_conditions.end())
            {
                ret = iter->second->check(m_npc, m_event);	//计算这个condition
                m_conditionValues.insert(std::make_pair(m_cur, ret));	// 缓存结果
            }
            else
            {
                if(m_npc != nullptr)
                {
                    LOG_TRACE("[AI:{}], trigger执行时，conditionId={} 不存在", m_npc->aiTplId(), m_cur);
                }
            }
        }
    }
	else if(m_cur == Eof)	// T = Eof
	{
		ret = false; // 空表达式按false处理
	}
    else //非预期的token, 语法错误
	{
		m_grammarError = true;
	}

	return ret;
}

/************************************** AI **************************************/
AI::AI(uint32_t tplId)
: m_tplId(tplId)
{

}

uint32_t AI::tplId() const
{
    return m_tplId;
}

void AI::handleEvent(Npc* npc, const AIEvent* event)
{
	if(npc == nullptr)
		return;
	
	for(const AIItem& item : m_items)
	{
		if(!item.trigger->check(npc, event))	
			continue;

		for(const AIAction::Ptr action : item.actions)
            action->exec(npc, event);
	}
}

AI::Ptr AI::clone() const
{
    AI::Ptr ret = create(m_tplId);

    for(auto it = m_conditions.begin(); it != m_conditions.end(); ++it)
        ret->m_conditions.insert({it->first, it->second->clone()});

    for(const auto& item : m_items)
    {
        ret->m_items.resize(ret->m_items.size() + 1);
        AIItem& newItem = ret->m_items.back();

        newItem.trigger = AITrigger::create(ret->m_conditions, item.trigger->expression());
        newItem.actions = item.actions;

        for(auto& action : newItem.actions)
            action = action->clone();
    }

    return ret;
}


}}

