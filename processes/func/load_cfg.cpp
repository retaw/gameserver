#include "func.h"
#include "store_consume_record.h"
#include "global_sundry.h"
#include "world_boss.h"
#include "first_config.h"
#include "first_manager.h"
#include "field_boss_manager.h"
#include "shabake.h"
#include "boss_home_manager.h"
#include "all_role_info_manager.h"
#include "faction_manager.h"
#include "team_manager.h"
#include "friend_manager.h"

namespace func{

void Func::loadConfigAndDB()
{
    GlobalSundry::me().loadDB();
	FirstManager::me().loadFromDB();

    StoreConsumeRecord::me().loadConfig(m_cfgDir);
    WorldBoss::me().loadConfig(m_cfgDir);
	FirstConfig::me().loadConfig(m_cfgDir);
    FieldBossManager::me().loadConfig(m_cfgDir);
    ShaBaKe::me().loadConfig(m_cfgDir);
    BossHomeManager::me().loadConfig(m_cfgDir);
    RoleInfoManager::me().init();
    FactionManager::me().loadConfig(m_cfgDir);
    FactionManager::me().loadFaction();
    FactionManager::me().loadRoleInFaction();
    TeamManager::me().loadConfig(m_cfgDir);
    FriendManager::me().loadConfig(m_cfgDir);
}

}
