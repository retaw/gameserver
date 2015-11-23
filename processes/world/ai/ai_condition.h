/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-19 14:57 +0800
 *
 * Modified: 2015-05-19 14:57 +0800
 *
 * Description: ai 触发条件的定义
 */


#ifndef PROCESS_WORLD_AI_CONDITION_H
#define PROCESS_WORLD_AI_CONDITION_H

#include "water/common/commdef.h"
#include "water/componet/class_helper.h"


namespace water{ namespace componet{ class XmlParseNode; }}

namespace world{
class Npc;

namespace ai{
class AIEvent;
using namespace water;
using componet::XmlParseNode;

class AICondition
{
public:
    TYPEDEF_PTR(AICondition)

    virtual ~AICondition() = default;

    void setId(char id);
    char id() const;

    virtual bool parseCfg(XmlParseNode& node) = 0;
    virtual bool check(Npc* npc, const AIEvent* event) = 0;
    virtual AICondition::Ptr clone() const = 0;

private:
    char m_id;
};


class TimerFilter : public AICondition
{
public:
    bool parseCfg(XmlParseNode& node) override;
    bool check(Npc* npc, const AIEvent* event) override;
    AICondition::Ptr clone() const override;

private:
    StdInterval m_interval;
};

class Probability : public AICondition
{
    bool parseCfg(XmlParseNode& node) override;
    bool check(Npc* npc, const AIEvent* event) override;
    AICondition::Ptr clone() const override;

private:
    int16_t m_prob;
};

}}

#endif

