/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-14 10:52 +0800
 *
 * Description:  统一注册注册消息处理
 */

#include "world.h"

#include "water/componet/logger.h"

#include "factionactive_manager.h"
#include "roles_and_scenes.h"
#include "role_action.h"
#include "role_pk.h"
#include "role_package.h"
#include "gm.h"
#include "buff_scene.h"
#include "hero_msg_manager.h"
#include "store_manager.h"
#include "relations_manager.h"
#include "watch_manager.h"
#include "mail_manager.h"
#include "strong_equip.h"
#include "guanzhi_manager.h"
#include "horse_msg.h"
#include "title_manager.h"
#include "weapon_lucky.h"
#include "wash_manager.h"
#include "ronghe.h"
#include "merge_manager.h"
#include "equip_quality_manager.h"
#include "wing_manager.h"
#include "pet_msg.h"
#include "dragon_ball_manager.h"
#include "use_object_manager.h"
#include "task_msg.h"
#include "zhuansheng_manager.h"
#include "fenjie_manager.h"
#include "dragon_heart_msg.h"
#include "expbead_manager.h"
#include "exp_area_manager.h"
#include "action_manager.h"
#include "world_boss.h"
#include "trigger_manager.h"
#include "bonfire_manager.h"
#include "field_boss_manager.h"
#include "stall_msg.h"
#include "trade_manager.h"
#include "scene_manager.h"
#include "private_boss_msg.h"
#include "boss_home_manager.h"
#include "daily_task_manager.h"
#include "shabake.h"

namespace world{


void World::registerTcpMsgHandler()
{
    //角色和场景
    RolesAndScenes::me().regMsgHandler();
    //角色行为
    RoleAction::me().regMsgHandler();
    //角色pk
    RolePk::me().regMsgHandler();
	//背包
    RolePackage::me().regMsgHandler();
	//GM
    Gm::me().regMsgHandler();
    //buff
    BuffScene::me().regMsgHandler();
	//英雄
	HeroMsgManager::me().regMsgHandler();
    //商城
    StoreMgr::me().regMsgHandler();
    //社会关系
    RelationsManager::me().regMsgHandler();
	//查看
	WatchManager::me().regMsgHandler();
    //邮件
    MailManager::me().regMsgHandler();
	//强化装备
	StrongEquip::me().regMsgHandler();	
	//官职
	GuanzhiManager::me().regMsgHandler();
	//称号
	TitleManager::me().regMsgHandler();
	//坐骑
    HorseMsg::me().regMsgHandler();
	//武器幸运
	WeaponLucky::me().regMsgHandler();
	//洗练
	WashManager::me().regMsgHandler();
	//装备融合
	Ronghe::me().regMsgHandler();
	//合成物品
	MergeManager::me().regMsgHandler();
	//装备升品
	EquipQualityManager::me().regMsgHandler();
	//翅膀晋阶、注灵
	WingManager::me().regMsgHandler();
    //宠物
    PetMsg::me().regMsgHandler();
	//龙珠
	DragonBallManager::me().regMsgHandler();
	//使用道具
	UseObjectManager::me().regMsgHandler();
    //任务
    TaskMsg::me().regMsgHandler();
	//转生
	ZhuanshengManager::me().regMsgHandler();
	//分解
	FenjieManager::me().regMsgHandler();
    //火龙之心
    DragonHeartMsg::me().regMsgHandler();
    //经验珠
    ExpBeadManager::me().regMsgHandler();
	//经验区(泡点)
	ExpAreaManager::me().regMsgHandler();
	//活动状态
	ActionManager::me().regMsgHandler();
    //世界boss
    WorldBoss::me().regMsgHandler();
    //机关
    TriggerManager::me().regMsgHandler();
	//篝火
	BonfireManager::me().regMsgHandler();
    //摆摊
    StallMsg::me().regMsgHandler();
    //帮派任务
    FactionActiveManager::me().regMsgHandler();
    //野外boss
    FieldBossManager::me().regMsgHandler();
	//面对面交易
	TradeManager::me().regMsgHandler();
    //创建动态场景
    SceneManager::me().regMsgHandler();
    //个人boss
    PrivateBossMsg::me().regMsgHandler();
    //boss之家
    BossHomeManager::me().regMsgHandler();
	//日常任务
	DailyTaskManager::me().regMsgHandler();
    //沙巴克
    ShaBaKe::me().regMsgHandler();
}


}
