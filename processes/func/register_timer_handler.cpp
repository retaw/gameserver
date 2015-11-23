/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-14 10:52 +0800
 *
 * Description:  统一注册注册定时器事件
 */

#include "func.h"
#include "role_manager.h"
#include "global_sundry.h"
#include "world_boss.h"
#include "first_manager.h"
#include "shabake.h"

#include "water/componet/logger.h"

namespace func{

void Func::registerTimerHandler()
{
    using namespace std::placeholders;
	m_timer.regEventHandler(std::chrono::seconds(5), std::bind(&RoleManager::timerExec, &RoleManager::me(), _1));
    RoleManager::me().regTimer();
    GlobalSundry::me().regTimer();
    WorldBoss::me().regTimer();
	FirstManager::me().regTimer();
    ShaBaKe::me().regTimer();
}


}
