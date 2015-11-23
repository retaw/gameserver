#include "role_manager.h"


#include "protocol/rawmsg/public/role_action.codedef.public.h"
#include "protocol/rawmsg/public/role_scene.codedef.public.h"
#include "protocol/rawmsg/public/channel_info.codedef.public.h"
#include "protocol/rawmsg/public/role_pk.codedef.public.h"
#include "protocol/rawmsg/public/package.codedef.public.h"
#include "protocol/rawmsg/public/object_scene.codedef.public.h"
#include "protocol/rawmsg/public/friend.codedef.public.h"
#include "protocol/rawmsg/public/buff_scene.codedef.public.h"
#include "protocol/rawmsg/public/hero_scene.codedef.public.h"
#include "protocol/rawmsg/public/store.codedef.public.h"
#include "protocol/rawmsg/public/team.codedef.public.h"
#include "protocol/rawmsg/public/watch.codedef.public.h"
#include "protocol/rawmsg/public/mail.codedef.public.h"
#include "protocol/rawmsg/public/guanzhi.codedef.public.h"
#include "protocol/rawmsg/public/title.codedef.public.h"
#include "protocol/rawmsg/public/horse.codedef.public.h"
#include "protocol/rawmsg/public/faction.codedef.public.h"
#include "protocol/rawmsg/public/wash.codedef.public.h"
#include "protocol/rawmsg/public/merge.codedef.public.h"
#include "protocol/rawmsg/public/equip_equality.codedef.public.h"
#include "protocol/rawmsg/public/wing.codedef.public.h"
#include "protocol/rawmsg/public/pet_scene.codedef.public.h"
#include "protocol/rawmsg/public/dragon_ball.codedef.public.h"
#include "protocol/rawmsg/public/use_object.codedef.public.h"
#include "protocol/rawmsg/public/zhuansheng.codedef.public.h"
#include "protocol/rawmsg/public/fenjie.codedef.public.h"
#include "protocol/rawmsg/public/task.codedef.public.h"
#include "protocol/rawmsg/public/dragon_heart.codedef.public.h"
#include "protocol/rawmsg/public/expbead.codedef.public.h"
#include "protocol/rawmsg/public/exp_area.codedef.public.h"
#include "protocol/rawmsg/public/action.codedef.public.h"
#include "protocol/rawmsg/public/trigger_scene.codedef.public.h"
#include "protocol/rawmsg/public/world_boss.codedef.public.h"
#include "protocol/rawmsg/public/first.codedef.public.h"
#include "protocol/rawmsg/public/faction_active.codedef.public.h"
#include "protocol/rawmsg/public/bonfire.codedef.public.h"
#include "protocol/rawmsg/public/field_boss.codedef.public.h"
#include "protocol/rawmsg/public/stall.codedef.public.h"
#include "protocol/rawmsg/public/trade.codedef.public.h"
#include "protocol/rawmsg/public/private_boss.codedef.public.h"
#include "protocol/rawmsg/public/boss_home.codedef.public.h"
#include "protocol/rawmsg/public/daily_task.codedef.public.h"
#include "protocol/rawmsg/public/shabake.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include "water/componet/logger.h"

namespace gateway{


#define CLIENT_MSG_TO_WORLD(clientMsgName) \
protocol::rawmsg::RawmsgManager::me().regHandler(RAWMSG_CODE_PUBLIC(clientMsgName), std::bind(&RoleManager::relayRoleMsgToWorld, this, RAWMSG_CODE_PUBLIC(clientMsgName), _1, _2, _3));


#define CLIENT_MSG_TO_FUNC(clientMsgName) \
protocol::rawmsg::RawmsgManager::me().regHandler(RAWMSG_CODE_PUBLIC(clientMsgName), std::bind(&RoleManager::relayRoleMsgToFunc, this, RAWMSG_CODE_PUBLIC(clientMsgName), _1, _2, _3));

void RoleManager::regRoleMsgRelay()
{
    using namespace std::placeholders;

    /************转发到world************/
    CLIENT_MSG_TO_WORLD(RoleMoveToPos);
	CLIENT_MSG_TO_WORLD(RequestSetRoleBufData);
	CLIENT_MSG_TO_WORLD(GotoOtherSceneByTransmission);
    CLIENT_MSG_TO_WORLD(RoleRequestAttack);
	CLIENT_MSG_TO_WORLD(RequestReliveRole);
	CLIENT_MSG_TO_WORLD(RequestPackageObjList);
	CLIENT_MSG_TO_WORLD(RequestPackageUnlockCellNum);
	CLIENT_MSG_TO_WORLD(RequestDestoryObjByCell);
	CLIENT_MSG_TO_WORLD(RequestDiscardObjByCell);
	CLIENT_MSG_TO_WORLD(RequestSplitObjNum);
	CLIENT_MSG_TO_WORLD(RequestExchangeCell);
	CLIENT_MSG_TO_WORLD(RequestMoveObj);
	CLIENT_MSG_TO_WORLD(RequestSortObj);
	CLIENT_MSG_TO_WORLD(RequestUnlockCellNeedSec);
	CLIENT_MSG_TO_WORLD(RequestUnlockCellNeedMoney);
	CLIENT_MSG_TO_WORLD(RequestUnlockCell);
	CLIENT_MSG_TO_WORLD(RequestPickupObject);
	CLIENT_MSG_TO_WORLD(RequestStrongEquip);
	CLIENT_MSG_TO_WORLD(RequestLevelUpWeaponLucky);
	CLIENT_MSG_TO_WORLD(RoleRequestUpgradeSkill);
    CLIENT_MSG_TO_WORLD(HeroRequestUpgradeSkill);
    CLIENT_MSG_TO_WORLD(RoleRequestStrengthenSkill);
    CLIENT_MSG_TO_WORLD(HeroRequestStrengthenSkill);
    CLIENT_MSG_TO_WORLD(RoleRequestSelectedBuff);
    CLIENT_MSG_TO_WORLD(RoleRequestSelectedBuffTips);
	CLIENT_MSG_TO_WORLD(RequestCreatedHeroList);
	CLIENT_MSG_TO_WORLD(RequestCreateHero);
	CLIENT_MSG_TO_WORLD(RequestSummonHero);
	CLIENT_MSG_TO_WORLD(RequestRecallHero);
    CLIENT_MSG_TO_WORLD(RequestHeroChangePos);
    CLIENT_MSG_TO_WORLD(HeroAIMode);
    CLIENT_MSG_TO_WORLD(HeroLockOnTarget);
    CLIENT_MSG_TO_WORLD(RoleLockOnTarget);
    CLIENT_MSG_TO_WORLD(HeroDisableSkillList);
	CLIENT_MSG_TO_WORLD(RequestSetDefaultCallHero);
	CLIENT_MSG_TO_WORLD(RoleRequestBuyGoods);
    CLIENT_MSG_TO_WORLD(RoleRequestSellObj);
    CLIENT_MSG_TO_WORLD(RoleRequestRepurchaseObj);
	CLIENT_MSG_TO_WORLD(RequestChangeAttackMode);
	CLIENT_MSG_TO_WORLD(PetRequestMoveToPos);
	CLIENT_MSG_TO_WORLD(RoleIntoCopyMap);
	CLIENT_MSG_TO_WORLD(RoleLeaveCopyMap);

    //邮件
    CLIENT_MSG_TO_WORLD(RequestMailList);
    CLIENT_MSG_TO_WORLD(RequestMailDetailInfo);
    CLIENT_MSG_TO_WORLD(RequestGetMailObj);
    CLIENT_MSG_TO_WORLD(RequestDeleteMail);


	//官职称号
	CLIENT_MSG_TO_WORLD(RequestGuanzhiRewardState);
	CLIENT_MSG_TO_WORLD(RequestGetGuanzhiReward);
	CLIENT_MSG_TO_WORLD(RequestGuanzhiLevelUp);
	CLIENT_MSG_TO_WORLD(RequestTitleList);
	CLIENT_MSG_TO_WORLD(RequestOperateNormalTitle);

    //坐骑
    CLIENT_MSG_TO_WORLD(RequestRaiseInfo);
    CLIENT_MSG_TO_WORLD(RequestRide);
    CLIENT_MSG_TO_WORLD(RequestRaise);
    CLIENT_MSG_TO_WORLD(RequestActiveSkins);

	//洗练
	CLIENT_MSG_TO_WORLD(RequestPropList);
	CLIENT_MSG_TO_WORLD(RequestLockOrUnlockProp);
	CLIENT_MSG_TO_WORLD(RequestWash);
	CLIENT_MSG_TO_WORLD(RequestReplaceCurProp);

	//融合
	CLIENT_MSG_TO_WORLD(RequestRongheEquip);
	//合成物品
	CLIENT_MSG_TO_WORLD(RequestMergeObj);
	//装备升品
	CLIENT_MSG_TO_WORLD(RequestImproveEquipQuality);
	
	//翅膀
	CLIENT_MSG_TO_WORLD(RequestWingLevelUp);
	CLIENT_MSG_TO_WORLD(RequestWingZhuling);

	//龙珠
	CLIENT_MSG_TO_WORLD(RequestDragonBallInfo);
	CLIENT_MSG_TO_WORLD(RequestLevelUpDragonBall);

    //任务
    CLIENT_MSG_TO_WORLD(RequestAcceptTask);
    CLIENT_MSG_TO_WORLD(NotifyArriveTaskTargetNpc);
    CLIENT_MSG_TO_WORLD(RequestSubmitTask);
    CLIENT_MSG_TO_WORLD(RequestCollect);
    CLIENT_MSG_TO_WORLD(FinishCollect);
    
    //飞鞋
    CLIENT_MSG_TO_WORLD(RequestFeixieTransfer);

	//使用道具
	CLIENT_MSG_TO_WORLD(RequestUseObject);
	//转生
	CLIENT_MSG_TO_WORLD(RequestZhuansheng);
	//分解
	CLIENT_MSG_TO_WORLD(RequestFenjie);
    //火龙之心
	CLIENT_MSG_TO_WORLD(RequestDragonSkills);
	CLIENT_MSG_TO_WORLD(RequestUpgradeDragonSkill);
	CLIENT_MSG_TO_WORLD(RequestAddEnerge);
    //经验珠
	CLIENT_MSG_TO_WORLD(ExpBeadShow);
	CLIENT_MSG_TO_WORLD(ExpBeadMainWindow);
	CLIENT_MSG_TO_WORLD(ExpBeadGet);
	CLIENT_MSG_TO_WORLD(ExpBeadRefresh);
	CLIENT_MSG_TO_WORLD(BuyGetTimes);
	
	//经验区
	CLIENT_MSG_TO_WORLD(RequestAutoAddExpList);
	CLIENT_MSG_TO_WORLD(RequestOpenAutoAddExp);
	CLIENT_MSG_TO_WORLD(RequestCloseAutoAddExp);

	//活动状态
	CLIENT_MSG_TO_WORLD(RequestSpanSecOfActionEnd);
	CLIENT_MSG_TO_WORLD(RequestJoinAction);

    //帮派商店
    CLIENT_MSG_TO_FUNC(RequestFactionShop);
    CLIENT_MSG_TO_WORLD(RequestBuyFactionObject);
    CLIENT_MSG_TO_FUNC(RefreshFactionShop);

    //机关
    CLIENT_MSG_TO_WORLD(TouchTrigger);

    //世界boss
	CLIENT_MSG_TO_WORLD(RequestDamageRank);
	CLIENT_MSG_TO_WORLD(ReqGetDamageAward);
	CLIENT_MSG_TO_WORLD(ReqDamageAwardInfo);

    //帮派任务
    CLIENT_MSG_TO_WORLD(FactionActive);
    CLIENT_MSG_TO_WORLD(AcceptFactionTask);
    CLIENT_MSG_TO_WORLD(FinishFactionTask);
    CLIENT_MSG_TO_WORLD(FactionTaskBuyVipGift);
    CLIENT_MSG_TO_WORLD(QuitFactionTask);
	//篝火
	CLIENT_MSG_TO_WORLD(RequestBonfireTeamInfo);
	CLIENT_MSG_TO_WORLD(RequestBonfireOwnerName);
	CLIENT_MSG_TO_WORLD(RequestJoinBonfire);
    CLIENT_MSG_TO_WORLD(FactionTask);
    //野外boss
    CLIENT_MSG_TO_WORLD(TransFerToFieldBoss);
    //boss之家
    CLIENT_MSG_TO_WORLD(TransFerToBossHome);
    CLIENT_MSG_TO_WORLD(TransFerToNextBossHome);
    CLIENT_MSG_TO_WORLD(LeaveBossHome);

    //摆摊
	CLIENT_MSG_TO_WORLD(ReqStallSellLog);
	CLIENT_MSG_TO_WORLD(ReqOpenStall);
	CLIENT_MSG_TO_WORLD(ReqCloseStall);
	CLIENT_MSG_TO_WORLD(ReqBuyStallObj);
	CLIENT_MSG_TO_WORLD(ReqWatchOthersStall);

	//面对面交易
	CLIENT_MSG_TO_WORLD(RequestTrade);
	CLIENT_MSG_TO_WORLD(RequestTradeAskList);
	CLIENT_MSG_TO_WORLD(RefuseTrade);
	CLIENT_MSG_TO_WORLD(RefuseAllTrade);
	CLIENT_MSG_TO_WORLD(AgreeTrade);
	CLIENT_MSG_TO_WORLD(RequestInputTradeMoney);
	CLIENT_MSG_TO_WORLD(RequestPutTradeObj);
	CLIENT_MSG_TO_WORLD(RequestRemoveTradeObj);
	CLIENT_MSG_TO_WORLD(RequestTradeInfo);
	CLIENT_MSG_TO_WORLD(RequestLockTrade);
	CLIENT_MSG_TO_WORLD(RequestUnlockTrade);
	CLIENT_MSG_TO_WORLD(RequestCancelTrade);
	CLIENT_MSG_TO_WORLD(ConfirmTrade);

    //个人boss
    CLIENT_MSG_TO_WORLD(PrivateBoss);
    CLIENT_MSG_TO_WORLD(TransFerToPrivateBoss);
    CLIENT_MSG_TO_WORLD(LeavePrivateBoss);
    CLIENT_MSG_TO_WORLD(ReqRefreshPrivateBoss);

	//日常任务
	CLIENT_MSG_TO_WORLD(RequestDailyTaskInfo);
	CLIENT_MSG_TO_WORLD(RequestAcceptDailyTask);
	CLIENT_MSG_TO_WORLD(ReuqestRefreshDailyTaskStar);
	CLIENT_MSG_TO_WORLD(RequestDailyTaskTopStar);
	CLIENT_MSG_TO_WORLD(RequestGetDailyTaskReward);
	CLIENT_MSG_TO_WORLD(RequestFinishAllDailyTask);
	CLIENT_MSG_TO_WORLD(RequestGetTopStarTaskNumReward);


    /************转发到func************/
    CLIENT_MSG_TO_FUNC(NormalChannelMsgFromClient);
    CLIENT_MSG_TO_FUNC(RequestFriendToS);
    CLIENT_MSG_TO_FUNC(RetRequestFriendToS);
    CLIENT_MSG_TO_FUNC(EraseFriendToS);
    CLIENT_MSG_TO_FUNC(InsertBlackList);
    CLIENT_MSG_TO_FUNC(EraseBlackList);
    CLIENT_MSG_TO_FUNC(RequestAllRelationerInfo);
	CLIENT_MSG_TO_FUNC(EraseEnemy);
	CLIENT_MSG_TO_FUNC(RequestRecommendFriend);
	CLIENT_MSG_TO_FUNC(GmMsgFromClient);
	CLIENT_MSG_TO_FUNC(RoleRequestLimitObjTabNum);
	CLIENT_MSG_TO_FUNC(RequestWatchRole);
	CLIENT_MSG_TO_FUNC(RequestWatchHero);
    //组队
    CLIENT_MSG_TO_FUNC(CreateTeam);
    CLIENT_MSG_TO_FUNC(BreakTeam);
    CLIENT_MSG_TO_FUNC(LeaveTeam);
    CLIENT_MSG_TO_FUNC(KickOutTeam);
    CLIENT_MSG_TO_FUNC(ChangeCaptain);
    CLIENT_MSG_TO_FUNC(ApplyJoinToS);
    CLIENT_MSG_TO_FUNC(RetApplyJoinToS);
    CLIENT_MSG_TO_FUNC(InviteJoinToS);
    CLIENT_MSG_TO_FUNC(RetInviteJoinToS);
    CLIENT_MSG_TO_FUNC(TeamMembers);
    CLIENT_MSG_TO_FUNC(NearbyTeams);
    CLIENT_MSG_TO_FUNC(FormTeam);
    //帮派
    CLIENT_MSG_TO_FUNC(ReqFactionHall);
    CLIENT_MSG_TO_FUNC(ReqFactionList);
    CLIENT_MSG_TO_FUNC(ReqMyApplyRecord);
    CLIENT_MSG_TO_FUNC(CreateFaction);
    CLIENT_MSG_TO_FUNC(ApplyJoinFactionToS);
    CLIENT_MSG_TO_FUNC(ApplyList);
    CLIENT_MSG_TO_FUNC(DealApplyRecord);
    CLIENT_MSG_TO_FUNC(CancelApplyRecord);
    CLIENT_MSG_TO_FUNC(AppointLeader);
    CLIENT_MSG_TO_FUNC(InviteJoinFactionToS);
    CLIENT_MSG_TO_FUNC(RetInviteJoinFactionToS);
    CLIENT_MSG_TO_FUNC(KickOutFromFaction);
    CLIENT_MSG_TO_FUNC(LeaveFaction);
    CLIENT_MSG_TO_FUNC(SaveNotice);
    CLIENT_MSG_TO_FUNC(ReqFactionLog);
    CLIENT_MSG_TO_FUNC(FactionLevelUp);
    CLIENT_MSG_TO_FUNC(ReqFactionMembers);

    //世界boss
    CLIENT_MSG_TO_FUNC(ReqWorldBossInfo);

	//天下第一
	CLIENT_MSG_TO_FUNC(RequestFirstApplyInfo);
	CLIENT_MSG_TO_FUNC(RequestApplyFirst);
	CLIENT_MSG_TO_FUNC(RequestIntoFistMap);
	CLIENT_MSG_TO_FUNC(RequestFirstWinnerInfo);

    //野外boss
    CLIENT_MSG_TO_FUNC(FieldBoss);
    //boss之家
    CLIENT_MSG_TO_FUNC(BossHome);

    //沙巴克
	CLIENT_MSG_TO_FUNC(ReqShabakeInfo);
	CLIENT_MSG_TO_FUNC(ReqGetShabakeAward);
	CLIENT_MSG_TO_FUNC(ReqShabakaAwardInfo);
	CLIENT_MSG_TO_FUNC(ReqShabakeDailyAwardInfo);
	CLIENT_MSG_TO_FUNC(ReqShabakePosition);
	CLIENT_MSG_TO_FUNC(ReqSetShabakePosition);

};

}
