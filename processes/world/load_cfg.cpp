#include "world.h"

#include "ai/ai_manager.h"
#include "scene_manager.h"
#include "config_table_manager.h"
#include "npc_manager.h"
#include "pk_cfg.h"
#include "exp_config.h"
#include "relive_config.h"
#include "object_config.h"
#include "nonsuch_config.h"
#include "suit_config.h"
#include "hero_config.h"
#include "store_manager.h"
#include "package_config.h"
#include "map_base.h"
#include "massive_config.h"
#include "stone_config.h"
#include "strong_config.h"
#include "guanzhi_config.h"
#include "title_config.h"
#include "horse_base.h"
#include "lucky_config.h"
#include "wash_config.h"
#include "ronghe_config.h"
#include "merge_config.h"
#include "equip_quality_config.h"
#include "wing_config.h"
#include "pet_base.h"
#include "dragon_ball_config.h"
#include "reward_fixed_config.h"
#include "reward_random_config.h"
#include "zhuansheng_config.h"
#include "fenjie_config.h"
#include "task_base.h"
#include "dragon_heart_cfg.h"
#include "expbead_manager.h"
#include "exp_area_config.h"
#include "bubble_point_config.h"
#include "trigger_cfg.h"
#include "world_boss.h"
#include "first_config.h"
#include "bonfire_config.h"
#include "factionactive_manager.h"
#include "field_boss_manager.h"
#include "private_boss_base.h"
#include "boss_home_manager.h"
#include "daily_task_config.h"
#include "shabake.h"

namespace world{

void World::loadConfig()
{
    MapBase::me().loadConfig(m_cfgDir);
    ai::AIManager::me().loadConfig(m_cfgDir);
    NpcManager::me().loadConfig(m_cfgDir);
    ConfigTableMgr::me().loadAllCfgTable(m_cfgDir);
    PKCfg::me().loadConfig(m_cfgDir);
	ExpConfig::me().loadConfig(m_cfgDir);
	ReliveConfig::me().loadConfig(m_cfgDir);
	ObjectConfig::me().loadConfig(m_cfgDir);
    SceneManager::me().loadConfig(m_cfgDir);
	NonsuchConfig::me().loadConfig(m_cfgDir);
	SuitConfig::me().loadConfig(m_cfgDir);
	HeroConfig::me().loadConfig(m_cfgDir);
    StoreMgr::me().loadConfig(m_cfgDir);
	PackageConfig::me().loadConfig(m_cfgDir);
    Massive::me().loadConfig(m_cfgDir);
	StoneConfig::me().loadConfig(m_cfgDir);
	StrongConfig::me().loadConfig(m_cfgDir);
	GuanzhiConfig::me().loadConfig(m_cfgDir);
	TitleConfig::me().loadConfig(m_cfgDir);
    HorseBase::me().loadConfig(m_cfgDir);
	LuckyConfig::me().loadConfig(m_cfgDir);
	WashConfig::me().loadConfig(m_cfgDir);
	RongheConfig::me().loadConfig(m_cfgDir);
	MergeConfig::me().loadConfig(m_cfgDir);
	EquipQualityConfig::me().loadConfig(m_cfgDir);
	WingConfig::me().loadConfig(m_cfgDir);
    PetBase::me().loadConfig(m_cfgDir);
	DragonBallConfig::me().loadConfig(m_cfgDir);
	RewardFixedConfig::me().loadConfig(m_cfgDir);
	RewardRandomConfig::me().loadConfig(m_cfgDir);
    TaskBase::me().loadConfig(m_cfgDir);
	ZhuanshengConfig::me().loadConfig(m_cfgDir);
	FenjieConfig::me().loadConfig(m_cfgDir);
    DragonHeartBase::me().loadConfig(m_cfgDir);
    ExpBeadManager::me().loadConfig(m_cfgDir);
	ExpAreaConfig::me().loadConfig(m_cfgDir);
	BubblePointConfig::me().loadConfig(m_cfgDir);
    TriggerCfg::me().loadConfig(m_cfgDir);
    WorldBoss::me().loadConfig(m_cfgDir);
	FirstConfig::me().loadConfig(m_cfgDir);
	BonfireConfig::me().loadConfig(m_cfgDir);
    FactionActiveManager::me().loadConfig(m_cfgDir);
    FieldBossManager::me().loadConfig(m_cfgDir);
    PrivateBossBase::me().loadConfig(m_cfgDir);
    BossHomeManager::me().loadConfig(m_cfgDir);
	DailyTaskConfig::me().loadConfig(m_cfgDir);
    ShaBaKe::me().loadConfig(m_cfgDir);
}

}
