/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-14 10:52 +0800
 *
 * Description:  统一注册注册消息处理
 */

#include "dbcached.h"

#include "water/componet/logger.h"
#include "role_manager.h"
#include "object_manager.h"
#include "randname_manager.h"
#include "skill_manager.h"
#include "buff_manager.h"
#include "counter_manager.h"
#include "hero_manager.h"
#include "mail.h"
#include "horse.h"
#include "title_manager.h"
#include "wash_manager.h"
#include "dragon_ball_manager.h"
#include "task_manager.h"
#include "exp_area_manager.h"
#include "sundry_manager.h"
#include "stall_sell_log.h"
#include "faction_role_manager.h"

namespace dbcached{


void DbCached::registerTcpMsgHandler()
{
    using namespace std::placeholders;
    RoleManager::me().regMsgHandler();
    
	ObjectManager::me().regMsgHandler();
    RandNameManager::me().regMsgHandler();
    SkillManager::me().regMsgHandler();
    BuffManager::me().regMsgHandler();
    CounterManager::me().regMsgHandler();
    HeroManager::me().regMsgHandler();
    Mail::me().regMsgHandler();
    Horse::me().regMsgHandler();
	TitleManager::me().regMsgHandler();
	WashManager::me().regMsgHandler();
	DragonBallManager::me().regMsgHandler();
    TaskManager::me().regMsgHandler();
	ExpAreaManager::me().regMsgHandler();
    SundryManager::me().regMsgHandler();
    StallLogMgr::me().regMsgHandler();
    FactionRoleManager::me().regMsgHandler();
}



}
