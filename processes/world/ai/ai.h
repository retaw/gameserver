/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-18 16:51 +0800
 *
 * Modified: 2015-05-18 16:51 +0800
 *
 * Description: ai的实现
 */


#ifndef PROCESS_WORLD_AI_DEF_H
#define PROCESS_WORLD_AI_DEF_H

#include "ai_action.h"
#include "ai_condition.h"

#include "water/componet/class_helper.h"

#include <map>
#include <vector>

namespace world{
class Npc;

namespace ai{
class AIEvent;

class AITrigger
{
public:
    TYPEDEF_PTR(AITrigger)
    CREATE_FUN_MAKE(AITrigger)


	AITrigger(std::map<char, AICondition::Ptr>& conditions, const std::string& expression);
	~AITrigger() = default;

    const std::string& expression() const;
	bool check(Npc* npc, const AIEvent* event);

private:
	const std::map<char, AICondition::Ptr>& m_conditions;	//全体condition的集合以及其对应的编号，expression的组成元素
	const std::string m_expression;	// 表达式

	// bool文法分析器，执行expression内容的脚本解释器
	// 语法作了限制，仅允许单个字符作为token，这样就省去了词法分析; 文法分析使用递归下降分析法
private:
	std::map<char, bool> m_conditionValues; // 缓存计算得到的condition的结果，避免在一个表达式中重复计算一个condition
	/*	消除左递归后的不二表达式文法，终结符c表示一个condition
	 *	E = F | E
	 *	E = F
	 *	F = T * F	// &是xml关键字，不便在xml中使用，用*替代，本来与运算符也叫逻辑乘
	 *	F = T
	 *	T = !T
	 *	T = (E)
	 *	T = c
	 *	T = Eof
	 */
	enum {Eof = '\0'};	//代码结束标志

	void read();	// 辅助函数，读一两个tokens, 第一个填入cur, 第二个填入next
	bool isCondition(); //判断是否为合法的token

	bool E();		// 非终结符E
	bool F();		// 非终结符F
	bool T();		// 非终结符T

private:
	uint32_t m_pos;       //当前分析到的位置
	char m_cur;           //当前token
	char m_next;          //下一个token
	bool m_grammarError;  //是否碰到文法错误

    Npc* m_npc;       //check执行时，暂存npc指针，非必须，暂存可以使解释器部分更清晰
    const AIEvent* m_event;
};


class AI
{
    friend class AIManager;
public:
    TYPEDEF_PTR(AI)
    CREATE_FUN_MAKE(AI)

	struct AIItem
	{
		AIItem() : trigger(nullptr)
		{
		}
        AITrigger::Ptr trigger;
		std::vector<AIAction::Ptr> actions;
	};
public:
	AI(uint32_t tplId);
	~AI() = default;

    uint32_t tplId() const;

	void handleEvent(Npc* npc, const AIEvent* event);             // 对某个npc, 循环执行每个AIItem
    AI::Ptr clone() const;


private:
	int32_t m_tplId;
	std::map<char, AICondition::Ptr> m_conditions;	// 具体condition的集合以及其对应的编号，每次tigger执行时临时保存的condition集
	std::vector<AIItem> m_items;					// 具体ai逻辑的原子单位，包括触发条件和动作 
};

}}

#endif
