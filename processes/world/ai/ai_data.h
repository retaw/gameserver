/*
 * Author: LiZhaojia
 *
 * Created: 2015-06-01 09:58 +0800
 *
 * Modified: 2015-06-01 09:58 +0800
 *
 * Description: 
 */

#ifndef PROCESSES_AI_DATA_H
#define PROCESSES_AI_DATA_H


#include "pk.h"
#include "water/componet/datetime.h"

namespace world{

struct AIData
{
    struct
    {
        PK::WPtr enemy;
        SceneItemType type = SceneItemType::none;
        TimePoint lastHarmTime = componet::EPOCH;
    } target;
    bool backToHome = false;
};

}


#endif

