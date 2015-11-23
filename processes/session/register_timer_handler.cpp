/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-07 15:44 +0800
 *
 * Description:  统一注册注册定时器事件
 */

#include "session.h"

#include "water/componet/logger.h"

#include "scene_dispenser.h"

namespace session{


void Session::registerTimerHandler()
{
    using namespace std::placeholders;
    SceneDispenser::me().regTimer();
}


}

