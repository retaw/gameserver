/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-14 10:52 +0800
 *
 * Description:  统一注册注册定时器事件
 */

#include "world.h"

#include "water/componet/logger.h"

#include "role_manager.h"
#include "npc_manager.h"
#include "scene_object_manager.h"
#include "fire.h"
#include "pet_manager.h"
#include "hero_ids.h"
#include "exp_area_manager.h"
#include "trigger_manager.h"
#include "bubble_point_manager.h"
#include "world_boss.h"
#include "first_manager.h"
#include "bonfire_manager.h"
#include "trade_manager.h"
#include "scene_manager.h"
#include "shabake.h"

namespace world{


void World::registerTimerHandler()
{
    RoleManager::me().regTimer();
    NpcManager::me().regTimer();
	SceneObjectManager::me().regTimer();
    FireManager::me().regTimer();
    PetManager::me().regTimer();
    HeroIDs::me().regTimer();
	ExpAreaManager::me().regTimer();
    TriggerManager::me().regTimer();
	BubblePointManager::me().regTimer();
    WorldBoss::me().regTimer();
	FirstManager::me().regTimer();
	BonfireManager::me().regTimer();
	TradeManager::me().regTimer();
    SceneManager::me().regTimer();
    ShaBaKe::me().regTimer();
}


}
