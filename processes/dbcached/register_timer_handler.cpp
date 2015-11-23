/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-14 10:52 +0800
 *
 * Description:  统一注册注册定时器事件
 */

#include "dbcached.h"

#include "water/componet/logger.h"
#include "test.h"
#include "role_manager.h"

namespace dbcached{



void DbCached::registerTimerHandler()
{
    using namespace std::placeholders;
//    m_timer.regEventHandler(std::chrono::seconds(2), test2SecsTimerHandler);
    //m_timer.regEventHandler(std::chrono::seconds(3),std::bind(&RoleManager::test, &RoleManager::me()));
    //m_timer.regEventHandler(std::chrono::hours(12),std::bind(&FriendManager::timerExec, &FriendManager::me()));
}


}
