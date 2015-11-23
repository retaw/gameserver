/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-19 15:38 +0800
 *
 * Modified: 2015-05-19 15:38 +0800
 *
 * Description: ai 行为
 */


#ifndef PROCESS_WORLD_AI_ACTION_H
#define PROCESS_WORLD_AI_ACTION_H


#include "water/componet/class_helper.h"

namespace water{ namespace componet{ class XmlParseNode; }}

namespace world{
class Npc;
using water::componet::XmlParseNode;

namespace ai{
class AIEvent;

class AIAction
{
public:
    TYPEDEF_PTR(AIAction)

    virtual ~AIAction() = default;

    void setId(char id);
    char id() const;

    virtual bool parseCfg(XmlParseNode& node) {return true;}
    virtual void exec(Npc* npc, const AIEvent* event) = 0;
    virtual AIAction::Ptr clone() const = 0;

private:
    char m_id;
};

class IdelMove : public AIAction
{
public:
    void exec(Npc* npc, const AIEvent* event) override;
    virtual AIAction::Ptr clone() const override;
private:
};


class MobAction : public AIAction
{
public:
    bool parseCfg(XmlParseNode& node) override;
    void exec(Npc* npc, const AIEvent* event) override;
    AIAction::Ptr clone() const override;

private:
    int16_t m_idelViewRange;        //待机视野
    int16_t m_combatViewRange;      //战斗视野
    int16_t m_attRange;             //射程
    bool    m_homeless;             //是否有家
};

class Tower : public AIAction
{
public:
    bool parseCfg(XmlParseNode& node) override;
    void exec(Npc* npc, const AIEvent* event) override;
    AIAction::Ptr clone() const override;

private:
    int16_t m_attRange;
};


}}

#endif
