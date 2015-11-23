/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-23 10:11 +0800
 *
 * Modified: 2015-05-23 10:11 +0800
 *
 * Description: ai事件
 */

#ifndef PROCESSES_AI_EVENT_H
#define PROCESSES_AI_EVENT_H

#include "water/componet/datetime.h"
#include "water/common/commdef.h"

namespace world{
namespace ai{

using water::componet::TimePoint;


enum class EventType
{
    none         = 0,
    timerEmit    = 1,
};


struct AIEvent
{
    AIEvent(EventType type_)
    : type(type_)
    {
    }

    const EventType type;
};

/***************************/

//定时器事件
struct TimerEmit : public AIEvent
{
    TimerEmit() : AIEvent(EventType::timerEmit)
    {
    }

    StdInterval interval;
    TimePoint now;
};

//找到了攻击目标
struct EnemyInTheView : public AIEvent
{
};



}}


#endif

