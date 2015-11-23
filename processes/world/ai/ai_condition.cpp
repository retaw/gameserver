#include "ai_condition.h"

#include "ai_event.h"

#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"

#include "water/componet/random.h"

namespace world{
namespace ai{

void AICondition::setId(char id)
{
    m_id = id;
}

char AICondition::id() const
{
    return m_id;
}

/**********************************************/

bool TimerFilter::parseCfg(XmlParseNode& node)
{
    auto str = node.getAttr<std::string>("interval");
    if(str == "msec_100")
        m_interval = StdInterval::msec_100;
    else if(str == "msec_300")
        m_interval = StdInterval::msec_300;
    else if(str == "msec_500")
        m_interval = StdInterval::msec_500;
    else if(str == "sec_1")
        m_interval = StdInterval::sec_1;
    else if(str == "sec_3")
        m_interval = StdInterval::sec_3;
    else if(str == "sec_5")
        m_interval = StdInterval::sec_5;
    else if(str == "sec_15")
        m_interval = StdInterval::sec_15;
    else if(str == "sec_30")
        m_interval = StdInterval::sec_30;
    else if(str == "min_1")
        m_interval = StdInterval::min_1;
    else if(str == "min_10")
        m_interval = StdInterval::min_10;
    else if(str == "min_15")
        m_interval = StdInterval::min_15;

    return true;
}

bool TimerFilter::check(Npc* npc, const AIEvent* event)
{
    if(event->type != EventType::timerEmit)
        return false;

    auto e = reinterpret_cast<const TimerEmit*>(event);
//    LOG_DEBUG("timerfilter::check, e->interval={}", e->interval);
    if(m_interval != e->interval)
        return false;

    return true;
}

AICondition::Ptr TimerFilter::clone() const
{
    auto ret = std::make_shared<TimerFilter>();
    *ret = *this;
    return ret;
}

/******************/

bool Probability::parseCfg(XmlParseNode& node)
{
    m_prob = node.getAttr<int16_t>("prob");
    return true;
}

bool Probability::check(Npc* npc, const AIEvent* event)
{
    if(m_prob == 0)
        return true;
    
    static componet::Random<int16_t> rand(0, 99);
    auto r = rand.get();
    return r < m_prob;
}

AICondition::Ptr Probability::clone() const
{
    auto ret = std::make_shared<Probability>();
    *ret = *this;
    return ret;
}

}}

