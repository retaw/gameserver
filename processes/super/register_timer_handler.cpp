/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-07 15:44 +0800
 *
 * Description:  统一注册注册定时器事件
 */

#include "super.h"

#include "water/componet/logger.h"
#include "zone_manager.h"

#include "test.h"

namespace super{


void Super::registerTimerHandler()
{
//    m_timer.regEventHandler(std::chrono::seconds(10), std::bind(testMsgSend));

    using namespace std::placeholders;
    m_timer.regEventHandler(std::chrono::milliseconds(100), std::bind(&ZoneManager::timerExec, &ZoneManager::me(), _1));

}


}
