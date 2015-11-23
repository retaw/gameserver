/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-14 10:52 +0800
 *
 * Description:  统一注册注册消息处理
 */

#include "func.h"

#include "water/componet/logger.h"

#include "role_manager.h"
#include "channel.h"
#include "friend_manager.h"
#include "store_consume_record.h"
#include "team_manager.h"
#include "watch_manager.h"
#include "faction_manager.h"
#include "relay_msg.h"
#include "world_boss.h"
#include "first_manager.h"
#include "field_boss_manager.h"
#include "boss_home_manager.h"
#include "shabake.h"

namespace func{


void Func::registerTcpMsgHandler()
{
    RoleManager::me().regMsgHandler();
    Channel::me().regMsgHandler();
    FriendManager::me().regMsgHandler();
    StoreConsumeRecord::me().regMsgHandler();
    TeamManager::me().regMsgHandler();
	WatchManager::me().regMsgHandler();
	FactionManager::me().regMsgHandler();
    RelayMsg::me().regMsgHandler();
    WorldBoss::me().regMsgHandler();
	FirstManager::me().regMsgHandler();
    FieldBossManager::me().regMsgHandler();
    BossHomeManager::me().regMsgHandler();
    ShaBaKe::me().regMsgHandler();
}


}
