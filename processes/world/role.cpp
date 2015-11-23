#include "role.h"
#include "pk_cfg.h"

#include "world.h"
#include "scene_manager.h"
#include "npc_manager.h"
#include "roles_and_scenes.h"
#include "relations_manager.h"
#include "exp_config.h"
#include "relive_config.h"
#include "massive_config.h"
#include "scene_object_manager.h"
#include "store_manager.h"
#include "pet_manager.h"
#include "object_config.h"
#include "stone_package.h"
#include "bubble_point_manager.h"
#include "action_manager.h"
#include "world_boss.h"
#include "function_icon.h"
#include "first_manager.h"
#include "trade_manager.h"
#include "shabake.h"
#include "boss_home_manager.h"

#include "water/componet/logger.h"
#include "water/componet/format.h"
#include "water/componet/scope_guard.h"
#include "water/componet/datetime.h"

#include "protocol/rawmsg/private/relay.codedef.private.h"
#include "protocol/rawmsg/private/relay.h"

#include "protocol/rawmsg/private/channel_info.h"
#include "protocol/rawmsg/private/channel_info.codedef.private.h"

#include "protocol/rawmsg/private/hero.h"
#include "protocol/rawmsg/private/hero.codedef.private.h"

#include "protocol/rawmsg/private/faction.h"
#include "protocol/rawmsg/private/faction.codedef.private.h"

#include "protocol/rawmsg/public/role_scene.h"
#include "protocol/rawmsg/public/role_scene.codedef.public.h"

#include "protocol/rawmsg/public/npc_scene.h"
#include "protocol/rawmsg/public/npc_scene.codedef.public.h"

#include "protocol/rawmsg/public/role_action.h"
#include "protocol/rawmsg/public/role_action.codedef.public.h"

#include "protocol/rawmsg/public/object_scene.h"
#include "protocol/rawmsg/public/object_scene.codedef.public.h"

#include "protocol/rawmsg/public/channel_info.h"
#include "protocol/rawmsg/public/channel_info.codedef.public.h"

#include "protocol/rawmsg/private/friend.h"
#include "protocol/rawmsg/private/friend.codedef.private.h"

#include "protocol/rawmsg/public/hero_scene.h"
#include "protocol/rawmsg/public/hero_scene.codedef.public.h"

#include "protocol/rawmsg/public/pet_scene.h"
#include "protocol/rawmsg/public/pet_scene.codedef.public.h"

#include "protocol/rawmsg/public/fire_scene.h"
#include "protocol/rawmsg/public/fire_scene.codedef.public.h"

#include "protocol/rawmsg/public/dragon_heart.h"
#include "protocol/rawmsg/public/dragon_heart.codedef.public.h"

#include "protocol/rawmsg/public/trigger_scene.h"
#include "protocol/rawmsg/public/trigger_scene.codedef.public.h"

#include "protocol/rawmsg/public/task.h"
#include "protocol/rawmsg/public/task.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include <cstdlib>
#include "water/componet/serialize.h"

namespace world{

using namespace water::componet;

Role::Role(const PrivateRaw::RoleIntoScene* rev)
: PK(rev->basic.id, rev->basic.name, rev->basic.job, SceneItemType::role)
, m_account(rev->account)
, m_sex(rev->basic.sex)
, m_money_1(rev->money_1)
, m_money_2(rev->money_2)
, m_money_3(rev->money_3)
, m_money_4(rev->money_4)
, m_money_5(rev->money_5)
, m_money_6(rev->money_6)
, m_money_7(rev->money_7)
, m_money_8(rev->money_8)
, m_money_9(rev->money_9)
, m_money_10(rev->money_10)
, m_curObjId(rev->curObjId)
, m_exp(rev->exp)
, m_dead(rev->dead)
, m_deathTimePoint(rev->deathTime)
, m_totalOnlineSec(rev->totalOnlineSec)
, m_offlineTime(rev->offlnTime)
, m_greynameTime(rev->greynameTime)
, m_defaultCallHero(rev->defaultCallHero)
, m_summonHeroFlag(rev->summonHero)
, m_nameColor(0)
, m_anger(rev->anger)
, m_stall(0)
, m_collectNpcId(0)
, m_holdBoxTime(EPOCH)
, m_packageSet(SceneItemType::role, rev->basic.job, rev->basic.id, rev->unlockCellNumOfRole, rev->unlockCellNumOfHero, rev->unlockCellNumOfStorage)
, m_levelProps(rev->basic.job, SceneItemType::role)
, m_heroManager(*this, rev->recallHeroTime)
, m_roleCounter(*this)
, m_attackMode(*this, rev->attackMode, rev->evilVal)
, m_mail(*this)
, m_guanzhi(*this, rev->guanzhiLevel)
, m_horse(*this, rev->rideState)
, m_title(*this)
, m_wash(SceneItemType::role, rev->basic.id, *this)
, m_wing(SceneItemType::role, rev->basic.id, *this)
, m_dragonBall(*this)
, m_useObject(*this)
, m_zhuansheng(SceneItemType::role, rev->basic.id, *this)
, m_fenjie(*this)
, m_dragonHeart(*this)
, m_expArea(*this)
, m_roleStall(*this)
, m_dailyTask(*this)
, m_roleTask(*this)
, m_roleFactionTask(*this)
, m_preSceneId(rev->preSceneId)
, m_prePos(Coord2D(rev->prePosX, rev->prePosY))
, m_privateBoss(*this)
{
    getAndInitVariableData(rev);
}

void Role::getAndInitVariableData(const PrivateRaw::RoleIntoScene* rev)
{
    setDir(static_cast<componet::Direction>(rev->dir));
    initAttrMember();
    setLevel(rev->level);
    setTurnLifeLevel(rev->turnLife);
	setPetTplId(rev->petTplId);
    //反序列化得到的所有变长数据存入各自的vector中
    std::string ss;
    ss.append((const char*)rev->buf,(std::string::size_type)rev->size);
    Deserialize<std::string> ds(&ss);
    std::vector<RoleObjData::ObjData> objVec;
    std::vector<SkillData> skillVec;
    std::vector<PKCdStatus> pkCdStatusVec;
    std::vector<BuffData> buffVec;
    std::string counterStr;
    std::vector<HeroInfoPra> heroInfoVec;
    std::vector<MailInfo> mailInfoVec;
    std::string horseStr;
	std::vector<TitleInfo> titleVec;
	std::vector<WashPropInfo> washPropVec;
	std::vector<DragonBallInfo> dragonVec;
	std::string taskStr;
    std::string factionTask;
	std::vector<std::pair<uint8_t, uint32_t> > expSecVec;
    std::string sundry;
    std::string timerSundry;
    std::string bufferStr;

	ds >> objVec;
    ds >> skillVec;
    ds >> pkCdStatusVec;
    ds >> buffVec;
    ds >> counterStr;
    ds >> heroInfoVec;

    uint32_t size = 0;
    ds >> size;
    for(uint32_t i = 0; i < size; ++i)
    {
        MailInfo info;
        ds >> info.mailIndex;
        ds >> info.title;
        ds >> info.text;
        ds >> info.state;
        ds >> info.time;
        for(auto num = 0; num < MAX_MAIL_OBJ_NUM; ++num)
        {
            ds >> info.obj[num].tplId;
            ds >> info.obj[num].num;
            ds >> info.obj[num].bind;
        }
        mailInfoVec.push_back(info);
    }

    ds >> horseStr;
	ds >> titleVec;
	ds >> bufferStr;
	ds >> washPropVec;
    ds >> m_poisonAttackerAttr;
    ds >> m_petPoisonAttr;
    ds >> m_petBuffs;
	ds >> dragonVec;
	ds >> taskStr;
    ds >> factionTask;
	ds >> expSecVec;
    ds >> sundry;
    ds >> timerSundry;

    m_roleFactionTask.initInfo(factionTask);
    m_roleSundry.loadSundry(sundry, id());
    m_roleSundry.loadSundryForTimer(timerSundry);

	m_packageSet.loadFromDB(objVec);
    loadSkill(skillVec);
    loadPassiveSkillCD(pkCdStatusVec);
    loadBuff(buffVec);
    m_roleCounter.loadFromDB(counterStr);
	m_heroManager.loadFromDB(heroInfoVec);
    m_mail.load(mailInfoVec);
    m_horse.loadFromDB(horseStr);
	m_title.loadFromDB(titleVec);
	m_wash.loadFromDB(washPropVec);
	m_dragonBall.loadFromDB(dragonVec);
    m_roleTask.loadFromDB(taskStr);
    m_roleFactionTask.initInfo(factionTask);
	m_expArea.loadFromDB(expSecVec);
    loadBufferData(bufferStr);

    //放最后
    if(0 == rev->hp)//新创建用户直接加满血蓝
    {
        setHp(getMaxHp());
        setMp(getMaxMp());
    }
    else
    {
        setHp(rev->hp);
        setMp(rev->mp);
    }
    m_factionId = rev->factionId;
    m_factionName.append(rev->factionName, NAME_BUFF_SZIE);
    m_factionPosition = FactionPosition(rev->factionPosition);
    m_factionLevel = rev->factionLevel;

}

void Role::setTeamInfo(const PrivateRaw::UpdateTeamInfo* data)
{
    if(data->insertOrErase)
        m_teamId = data->teamId;
    else
        m_teamId = 0;

    sendMainToMe();
    syncScreenDataTo9();
}

TeamId Role::teamId() const
{
    return m_teamId;
}

void Role::setEnemy(std::unordered_set<RoleId>& enemy)
{
    m_enemy.swap(enemy);
}

bool Role::inBlackList(const RoleId roleId) const
{
    auto it = m_blackList.find(roleId);
    if(it == m_blackList.end())
        return false;
    
	return true;
}

void Role::setBlack(std::unordered_set<RoleId>& black)
{
    m_blackList.swap(black);
}

void Role::setFaction(const PrivateRaw::UpdateFaction* rev)
{
    m_factionId = rev->factionId;
    if(rev->factionId == 0)
    {
        m_factionName.clear();
        m_factionPosition = FactionPosition::ordinary;
        m_banggong = 0;
        //帮派商店相关缓存清除
        m_roleSundry.m_facShopTab = 0;
        m_roleSundry.m_facShopGoodsId.clear();
        m_factionLevel = 0;
        //帮派任务清空,
        m_roleFactionTask.clear();
    }
    else
    {
        m_factionName = rev->factionName;
        m_factionPosition = rev->position;
        m_factionLevel = rev->level;
    }

    sendMainToMe();
    syncScreenDataTo9();
}

FactionId Role::factionId() const
{
    return m_factionId;
}

void Role::setFactionLevel(const uint32_t level)
{
    m_factionLevel = level;
}

uint32_t Role::factionLevel()
{
    return m_factionLevel;
}

FactionPosition Role::factionPosition() const
{
    return m_factionPosition;
}

const std::string& Role::account() const
{
    return m_account;
}

std::string Role::toString() const
{
    return water::componet::format("[{}, {}, {}]", id(), name(), account());
}

bool Role::sendToMe(TcpMsgCode msgCode) const
{
    LOG_DEBUG("sendToMe, code={}, role={}", msgCode, *this);

    const uint32_t bufSize = sizeof(PrivateRaw::RelayMsgToClient);
    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf); 

    auto* send = new(buf) PrivateRaw::RelayMsgToClient();
    send->rid     = id();
    send->msgCode = msgCode;
    send->msgSize = 0;
    return World::me().sendToPrivate(m_gatewayId, RAWMSG_CODE_PRIVATE(RelayMsgToClient), buf, bufSize);
}

bool Role::sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
	Scene::Ptr s = scene();
	if(s == nullptr)
		return false;

	Screen* screen = s->getScreenByGridCoord(pos());
	if(screen == nullptr)
		return false;

	if(msgCode == RAWMSG_CODE_PUBLIC(ObjectsAroundMe))
	{
		auto rev = reinterpret_cast<const PublicRaw::ObjectsAroundMe*>(msg);
		if(!rev)
			return false;

		for(ArraySize i = 0; i < rev->size; i++)
		{
			LOG_TRACE("场景物品, 进入九屏, s->c, 发给玩家, 成功, objId={}, tplId={}, pos=({},{}), i={}, size={}, role=({}, {}, screen={})",
					  rev->objects[i].objId, rev->objects[i].tplId, rev->objects[i].posX,
					  rev->objects[i].posY, i, rev->size, name(), id(), screen);
		}
	}
    
	if(msgCode == RAWMSG_CODE_PUBLIC(ObjectLeaveInfo))
	{
		auto rev = reinterpret_cast<const PublicRaw::ObjectLeaveInfo*>(msg);
		if(!rev)
			return false;

		for(ArraySize i = 0 ; i < rev->size; i++)
		{
			LOG_TRACE("场景物品, 离开九屏, s->c, 发给玩家, 成功, objId={}, i={}, size={}, role=({}, {}, screen={})",
					  rev->objId[i], i, rev->size, name(), id(), screen);
		}
	}

	LOG_DEBUG("sendToMe, code={}, size={}, role={}", msgCode, msgSize, *this);

    const uint32_t bufSize = sizeof(PrivateRaw::RelayMsgToClient) + msgSize;

    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf); 

    auto* send = new(buf) PrivateRaw::RelayMsgToClient();
    send->rid     = id();
    send->msgCode = msgCode;
    send->msgSize = msgSize;
    std::memcpy(send->msgData, msg, msgSize);
    return World::me().sendToPrivate(m_gatewayId, RAWMSG_CODE_PRIVATE(RelayMsgToClient), buf, bufSize);
}



ProcessIdentity Role::gatewayId()
{
    return m_gatewayId;
}

void Role::setGatewayId(ProcessIdentity pid)
{
    m_gatewayId = pid;
}


void Role::afterEnterScene()
{
    Scene::Ptr s = scene();
    if(s == nullptr)
        return;

    {
        //平砍技能初始化
        m_skillM.initSkill(10000);
    }

    m_packageSet.setOwner(std::static_pointer_cast<Role>(shared_from_this()));

    LOG_DEBUG("角色进入场景, 发送屏信息, curObjId={}, {}", m_curObjId, *this);
    m_attackMode.initNameColor();
    sendMapIdToMe();
    sendMainToMe();
    sendSkillListToMe();
    sendBuffListToMe();
	sendBufferDataToMe();
	retRoleIntoSceneToSession();
    m_attackMode.setMode(attack_mode::none != s->mapAttackMode() ? s->mapAttackMode() : m_attackMode.saveMode(), true);
    m_mail.notifyNewMail();
    m_horse.afterEnterScene();
	m_heroManager.afterRoleEnterScene();
	PetManager::me().summonPet(petTplId(), petSkillId(), petLevel(), shared_from_this());
    m_roleTask.afterEnterScene();
    m_roleFactionTask.afterEnterScene();
    m_privateBoss.afterEnterScene();
    BossHomeManager::me().afterEnterScene(std::static_pointer_cast<Role>(shared_from_this()));
	m_dailyTask.afterEnterScene();
	ActionManager::me().sendAllActionStateToMe(std::static_pointer_cast<Role>(shared_from_this()));
	BubblePointManager::me().afterRoleEnterScene(std::static_pointer_cast<Role>(shared_from_this()));
    FunctionIcon::me().retAllFunctionIconState(std::static_pointer_cast<Role>(shared_from_this()));
	FirstManager::me().afterRoleEnterScene(std::static_pointer_cast<Role>(shared_from_this()));
    ShaBaKe::me().afterEnterScene(std::static_pointer_cast<Role>(shared_from_this()));

    enterVisualScreens(s->get9ScreensByGridCoord(pos()));

    //玩家进场景完成通知前端
    PublicRaw::RoleEnterSceneOver notify;
    sendToMe(RAWMSG_CODE_PUBLIC(RoleEnterSceneOver), &notify, sizeof(notify));
    LOG_TRACE("角色进入场景成功, {}, sceneId={}, sceneName={}", *this, sceneId(), s->name());
}

void Role::beforeLeaveScene()
{
    //发送下线保存信息给db
    PrivateRaw::SaveOffline saveOffline;
    saveOffline.rid = id();
    //当前坐标
    saveOffline.sceneId = sceneId();
    saveOffline.dir = static_cast<decltype(saveOffline.dir)>(dir());
    saveOffline.posX = pos().x;
    saveOffline.posY = pos().y;
    saveOffline.preSceneId = preSceneId();
    saveOffline.prePosX = prePos().x;
    saveOffline.prePosY = prePos().y;

    //前一张地图id 和 坐标
    auto s = scene();
    if(nullptr != s)
    {
        switch(s->copyType())
        {
        case CopyMap::none:
            {//静态地图处理
                saveOffline.preSceneId = sceneId();
                saveOffline.prePosX = pos().x;
                saveOffline.prePosY = pos().y;
                m_preSceneId = sceneId();
                m_prePos = pos();
            }
            break;
        case CopyMap::world_boss:
            {//离开地图直接返回之前的静态地图类处理
                saveOffline.sceneId = preSceneId();
                saveOffline.posX = prePos().x;
                saveOffline.posY = prePos().y;
            }
            break;
        case CopyMap::shabake:
            {//离开地图直接返回主城类地图处理
                saveOffline.sceneId = ReliveConfig::me().cityId();
                saveOffline.posX = ReliveConfig::me().cityRelivePos().x;
                saveOffline.posY = ReliveConfig::me().cityRelivePos().y;
            }
            break;
        default:
            break;
        }
    }

    //mp、hp
    saveOffline.mp = getMp();
    saveOffline.hp = getHp();
    //其它杂项
	saveOffline.dead = isDead();
	saveOffline.deathTime = getDeathTimePoint();
	saveOffline.totalOnlineSec = getTotalOnlineSec();
    saveOffline.offlnTime = toUnixTime(Clock::now());
    saveOffline.evilVal = m_attackMode.evilVal();
    saveOffline.attackMode = static_cast<uint8_t>(m_attackMode.saveMode());
    saveOffline.greynameTime = m_greynameTime;
	saveOffline.summonHero = getSummonHeroFlag();
    saveOffline.rideState = static_cast<uint8_t>(m_horse.rideState());
	saveOffline.curObjId = getCurObjId();
    saveOffline.petTplId = petTplId();
    saveOffline.petLevel = petLevel();
    saveOffline.anger = m_anger;

    ProcessIdentity receiver("dbcached", 1);
    World::me().sendToPrivate(receiver,RAWMSG_CODE_PRIVATE(SaveOffline),&saveOffline,sizeof(saveOffline));
    LOG_TRACE("角色离开场景, offln数据发送给dbcahed, roleId={}, posX={}, posY={}, mp={}, hp={}, dead={}, deathTime={}, totalOnlineSec={}, curObjId={}",
			  saveOffline.rid, saveOffline.posX, saveOffline.posY, 
			  saveOffline.mp, saveOffline.hp, 
			  saveOffline.dead, toUnixTime(saveOffline.deathTime),
			  saveOffline.totalOnlineSec, saveOffline.curObjId);

    handleInterrupt(interrupt_operate::leaveScene, SceneItemType::role);
    cachePoisonAttrDB(id());
    m_skillM.leaveScene();
    m_horse.leaveScene();
    m_heroManager.beforeRoleLeaveScene();
    FireManager::me().erase(id());
    m_roleTask.beforeLeaveScene();
    m_roleFactionTask.beforeLeaveScene();
	BubblePointManager::me().beforeRoleLeaveScene(std::static_pointer_cast<Role>(shared_from_this()));
	FirstManager::me().beforeRoleLeaveScene(std::static_pointer_cast<Role>(shared_from_this()));

    auto petPtr = pet();
    if(nullptr != petPtr)
        petPtr->erase(false);

    if(nullptr != s)
        leaveVisualScreens(s->get9ScreensByGridCoord(pos()));
}

/*
 * 所有跨天需要处理的东东都放这里
 */
void Role::dealNotSameDay()
{
	if(0 == m_roleCounter.get(CounterType::dayFlag)) //true 表示跨天
    {
        m_roleCounter.add(CounterType::dayFlag);

        //下面是跨天需要处理的各自模块的东东...
		setMoney(MoneyType::money_6, 0);
		m_guanzhi.dealNotSameDay();
		m_expArea.dealNotSameDay();
		m_horse.zeroClear();
		m_roleCounter.clear(CounterType::bonfire);

        m_roleSundry.m_buyStoreDayLimitObj.clear();
        m_roleSundry.privateBossMap.clear();

        WorldBoss::me().giveOrMailDamageAward(std::static_pointer_cast<Role>(shared_from_this()), true);
        m_roleSundry.m_worldBossDamage = 0;
        m_roleSundry.m_receivedWBDamageAward.clear();
		m_dailyTask.dealNotSameDay();
	}
}

void Role::login()
{
    //角色登陆处理
    m_attackMode.login();
    m_roleTask.checkAndUnlockTask(false);

    if(0 == getTotalOnlineSec())
    {
        //新注册用户 处理
        m_roleCounter.add(CounterType::dayFlag);
        m_skillM.unlockSkill(false);
        m_skillM.openJointSkill();
    }

	dealNotSameDay();
}

void Role::offline()
{
    m_anger = 0;
    m_dead = false;
    //角色下线
    m_buffM.processOffline();
	m_heroManager.roleOffline();
    m_horse.clearRideState();

    auto petPtr = pet();
    if(nullptr != petPtr)
        petPtr->erase(true);
    beforeLeaveScene();
}

void Role::retRoleIntoSceneToSession()
{
	PrivateRaw::RetRoleIntoScene send;
	send.rid = id();
	send.gatewayId = gatewayId().value();
	send.sceneId = sceneId();
	send.isSuccessful = true;
	ProcessIdentity sessionId("session", 1);
	World::me().sendToPrivate(sessionId, RAWMSG_CODE_PRIVATE(RetRoleIntoScene), &send, sizeof(send));
	LOG_DEBUG("角色进场景, 成功, sceneId={}, {}", sceneId(), *this);
}

void Role::sendMapIdToMe() const
{
    PublicRaw::NewSceneMapId send;
    send.mapId = sceneId2MapId(sceneId());
    sendToMe(RAWMSG_CODE_PUBLIC(NewSceneMapId), &send, sizeof(send));
}

void Role::sendMainToMe()
{
    resetHpMp();
    PublicRaw::SelfMainInfo send;
    fillMainData(&send.info);
    sendToMe(RAWMSG_CODE_PUBLIC(SelfMainInfo), &send, sizeof(send));
}

void Role::syncScreenDataTo9() const
{
    auto s = scene();
    if(s == nullptr)
        return;
	PublicRaw::BroadcastRoleScreenDataToNine send;
	fillScreenData(&send.info);
	s->sendCmdTo9(RAWMSG_CODE_PUBLIC(BroadcastRoleScreenDataToNine), 
				  &send, sizeof(send), pos());
}

void Role::enterVisualScreens(const std::vector<Screen*>& screens) const
{
    //进入自己视野的role
    std::vector<uint8_t> bufRoles;
    bufRoles.reserve(1024);
    bufRoles.resize(sizeof(PublicRaw::RolesAroundMe));
    uint32_t roleCounter = 0;

    //进入视野的npc
    std::vector<uint8_t> bufNpcs;
    bufNpcs.reserve(512);
    bufNpcs.resize(sizeof(PublicRaw::NpcsAroundMe) - sizeof(PublicRaw::NpcScreenData));
    uint32_t npcCounter = 0;

    //进入视野的object
    std::vector<uint8_t> bufObjects;
    bufObjects.reserve(512);
    bufObjects.resize(sizeof(PublicRaw::ObjectsAroundMe) - sizeof(PublicRaw::ObjectScreenData));
    uint32_t objectCounter = 0;

    //进入视野的hero
    std::vector<uint8_t> bufHeros;
    bufHeros.reserve(512);
    bufHeros.resize(sizeof(PublicRaw::HerosAroundMe));
    uint32_t heroCounter = 0;

    //宠物
    std::vector<uint8_t> bufPets;
    bufPets.reserve(512);
    bufPets.resize(sizeof(PublicRaw::PetsAroundMe));
    uint32_t petCounter = 0;

    //火墙
    std::vector<uint8_t> bufFires;
    bufFires.reserve(128);
    bufFires.resize(sizeof(PublicRaw::FireAroundMe));
    uint32_t fireCounter = 0;

    //机关物件
    std::vector<uint8_t> bufTriggers;
    bufTriggers.reserve(128);
    bufTriggers.resize(sizeof(PublicRaw::TriggerAroundMe));
    uint16_t triggerCounter = 0;

    for(Screen* screen : screens)
    {
        //roles
        for(auto it = screen->roles().begin(); it != screen->roles().end(); ++it)
        {
			Role::Ptr role = it->second;
			bufRoles.resize(bufRoles.size() + sizeof(RoleScreenData));
            auto* msg  = reinterpret_cast<PublicRaw::RolesAroundMe*>(bufRoles.data());
            role->fillScreenData(msg->roles + msg->size);
            ++msg->size;
            ++roleCounter;
        }

        //npcs
        for(auto it = screen->npcs().begin(); it != screen->npcs().end(); ++it)
        {
            Npc::Ptr npc = it->second;
            bufNpcs.resize(bufNpcs.size() + sizeof(PublicRaw::NpcScreenData));
            auto* msg = reinterpret_cast<PublicRaw::NpcsAroundMe*>(bufNpcs.data());
            npc->fillScreenData(msg->npcs + msg->size);
//            LOG_DEBUG("entervisualscreen, npc: tplId={}, id={}, name={}, pos={}", npc->tplId(), npc->id(), npc->name(), npc->pos());
            ++msg->size;
            ++npcCounter;
        }
        
		//objects
        for(auto it = screen->objs().begin(); it != screen->objs().end(); ++it)
        {
            SceneObject::Ptr obj = it->second;
            bufObjects.resize(bufObjects.size() + sizeof(PublicRaw::ObjectScreenData));
            auto* msg = reinterpret_cast<PublicRaw::ObjectsAroundMe*>(bufObjects.data());
            obj->fillScreenData(msg->objects + msg->size);
            ++msg->size;
            ++objectCounter;
        }
        
		//heros
        for(auto it = screen->heros().begin(); it != screen->heros().end(); ++it)
        {
            Hero::Ptr hero = it->second;
            bufHeros.resize(bufHeros.size() + sizeof(HeroScreenData));
            auto* msg = reinterpret_cast<PublicRaw::HerosAroundMe*>(bufHeros.data());
            hero->fillScreenData(msg->heros + msg->size);
            ++msg->size;
            ++heroCounter;
        }

        //pets
        for(auto it = screen->pets().begin(); it != screen->pets().end(); ++it)
        {
            Pet::Ptr pet = it->second;
            bufPets.resize(bufPets.size() + sizeof(PublicRaw::PetScreenData));
            auto* msg = reinterpret_cast<PublicRaw::PetsAroundMe*>(bufPets.data());
            pet->fillScreenData(msg->pets + msg->size);
            ++msg->size;
            ++petCounter;
        }

        //fires
        for(auto it = screen->fires().begin(); it != screen->fires().end(); ++it)
        {
            Fire::Ptr fire = it->second;
            bufFires.resize(bufFires.size() + sizeof(PublicRaw::FireScreenData));
            auto* msg = reinterpret_cast<PublicRaw::FireAroundMe*>(bufFires.data());
            fire->fillScreenData(msg->data + msg->size);
            ++msg->size;
            ++fireCounter;
        }

        //triggers
        for(auto it = screen->triggers().begin(); it != screen->triggers().end(); ++it)
        {
            Trigger::Ptr trigger = it->second;
            bufTriggers.resize(bufTriggers.size() + sizeof(PublicRaw::TriggerScreenData));
            auto* msg = reinterpret_cast<PublicRaw::TriggerAroundMe*>(bufTriggers.data());
            trigger->fillScreenData(msg->data + msg->size);
            ++msg->size;
            ++triggerCounter;
        }
    }
    if(triggerCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(TriggerAroundMe), bufTriggers.data(), bufTriggers.size());
	if(heroCounter)
		sendToMe(RAWMSG_CODE_PUBLIC(HerosAroundMe), bufHeros.data(), bufHeros.size());
    if(objectCounter > 0)
		sendToMe(RAWMSG_CODE_PUBLIC(ObjectsAroundMe), bufObjects.data(), bufObjects.size());
	if(npcCounter > 0)
    {
        auto* msg = reinterpret_cast<PublicRaw::NpcsAroundMe*>(bufNpcs.data());
        LOG_DEBUG("NpcsAroundMe:size={}",msg->size);
        for(int i= 0;i< msg->size; i++)
        {
            LOG_DEBUG("npcid={},pos:({},{})",msg->npcs[i].id,msg->npcs[i].posX,msg->npcs[i].posY);
        }
        sendToMe(RAWMSG_CODE_PUBLIC(NpcsAroundMe), bufNpcs.data(), bufNpcs.size());
    
    }
    if(petCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(PetsAroundMe), bufPets.data(), bufPets.size());
    if(fireCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(FireAroundMe), bufFires.data(), bufFires.size());
    if(roleCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(RolesAroundMe), bufRoles.data(), bufRoles.size());
    else //周围没有玩家, 直接返回
        return;

    //自己进入别人视野
	Scene::Ptr s = scene();
	if(s == nullptr)
		return;

    std::vector<uint8_t> buf;
    buf.resize(sizeof(PublicRaw::RolesAroundMe) + sizeof(RoleScreenData));
    auto msg = new(buf.data()) PublicRaw::RolesAroundMe();
    new(msg->roles) RoleScreenData();
    fillScreenData(msg->roles);
    msg->size = 1;
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(RolesAroundMe), buf.data(), buf.size());
}

void Role::leaveVisualScreens(const std::vector<Screen*>& screens) const
{
    //将别人、npc、object、hero从自己的视野中删除
    std::vector<uint8_t> bufRoles;
    bufRoles.reserve(512);
    bufRoles.resize(sizeof(PublicRaw::RoleLeaveInfo));
    uint32_t roleCounter = 0;

    std::vector<uint8_t> bufNpcs;
    bufNpcs.reserve(512);
    bufNpcs.resize(sizeof(PublicRaw::NpcLeaveInfo));
    uint32_t npcCounter = 0;

    std::vector<uint8_t> bufObjects;
    bufObjects.reserve(512);
    bufObjects.resize(sizeof(PublicRaw::ObjectLeaveInfo));
    uint32_t objectCounter = 0;

    std::vector<uint8_t> bufHeros;
    bufHeros.reserve(512);
    bufHeros.resize(sizeof(PublicRaw::HeroLeaveInfo));
    uint32_t heroCounter = 0;

    std::vector<uint8_t> bufPets;
    bufPets.reserve(512);
    bufPets.resize(sizeof(PublicRaw::PetLeaveInfo));
    uint32_t petCounter = 0;

    std::vector<uint8_t> bufFires;
    bufFires.reserve(128);
    bufFires.resize(sizeof(PublicRaw::FireLeaveInfo));
    uint32_t fireCounter = 0;

    std::vector<uint8_t> bufTriggers;
    bufTriggers.reserve(128);
    bufTriggers.resize(sizeof(PublicRaw::TriggerLeaveInfo));
    uint16_t triggerCounter = 0;

    for(Screen* screen : screens)
    {
        for(auto it = screen->roles().begin(); it != screen->roles().end(); ++it)
        {
            Role::Ptr role = it->second;
            bufRoles.resize(bufRoles.size() + sizeof(RoleId));
            auto* msg  = reinterpret_cast<PublicRaw::RoleLeaveInfo*>(bufRoles.data());
            msg->id[msg->size] = role->id();
            ++msg->size;
            ++roleCounter;
        }

        for(auto it = screen->npcs().begin(); it != screen->npcs().end(); ++it)
        {
            Npc::Ptr npc = it->second;
            bufNpcs.resize(bufNpcs.size() + sizeof(PKId));
            auto* msg = reinterpret_cast<PublicRaw::NpcLeaveInfo*>(bufNpcs.data());
            msg->id[msg->size] = npc->id();
//            LOG_DEBUG("entervisualscreen, npc, tplId={}, id={}, name={}, pos={}", npc->tplId(), npc->id(), npc->name(), npc->pos());
            ++msg->size;
            ++npcCounter;
        }

        for(auto it = screen->objs().begin(); it != screen->objs().end(); ++it)
        {
            SceneObject::Ptr obj = it->second;
            bufObjects.resize(bufObjects.size() + sizeof(ObjectId));
            auto* msg = reinterpret_cast<PublicRaw::ObjectLeaveInfo*>(bufObjects.data());
            msg->objId[msg->size] = obj->objId();
            ++msg->size;
            ++objectCounter;
        }
        
		for(auto it = screen->heros().begin(); it != screen->heros().end(); ++it)
        {
            Hero::Ptr hero = it->second;
            bufHeros.resize(bufHeros.size() + sizeof(HeroId));
            auto* msg  = reinterpret_cast<PublicRaw::HeroLeaveInfo*>(bufHeros.data());
            msg->heroId[msg->size] = hero->id();
            ++msg->size;
            ++heroCounter;
        }

        for(auto it = screen->pets().begin(); it != screen->pets().end(); ++it)
        {
            Pet::Ptr pet = it->second;
            bufPets.resize(bufPets.size() + sizeof(PKId));
            auto* msg = reinterpret_cast<PublicRaw::PetLeaveInfo*>(bufPets.data());
            msg->id[msg->size] = pet->id();
            ++msg->size;
            ++petCounter;
        }

        for(auto it = screen->fires().begin(); it != screen->fires().end(); ++it)
        {
            Fire::Ptr fire = it->second;
            bufFires.resize(bufFires.size() + sizeof(PublicRaw::FireScreenData));
            auto* msg = reinterpret_cast<PublicRaw::FireLeaveInfo*>(bufFires.data());
            fire->fillScreenData(msg->data + msg->size);
            ++msg->size;
            ++fireCounter;
        }

        for(auto it = screen->triggers().begin(); it != screen->triggers().end(); ++it)
        {
            Trigger::Ptr trigger = it->second;
            bufTriggers.resize(bufTriggers.size() + sizeof(PublicRaw::TriggerScreenData));
            auto* msg = reinterpret_cast<PublicRaw::TriggerLeaveInfo*>(bufTriggers.data());
            trigger->fillScreenData(msg->data + msg->size);
            ++msg->size;
            ++triggerCounter;
        }
    }
    if(triggerCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(TriggerLeaveInfo), bufTriggers.data(), bufTriggers.size());
    if(fireCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(FireLeaveInfo), bufFires.data(), bufFires.size());
    if(petCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(PetLeaveInfo), bufPets.data(), bufPets.size());
	if(heroCounter > 0)
		sendToMe(RAWMSG_CODE_PUBLIC(HeroLeaveInfo), bufHeros.data(), bufHeros.size());
    if(objectCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(ObjectLeaveInfo), bufObjects.data(), bufObjects.size());
    if(npcCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(NpcLeaveInfo), bufNpcs.data(), bufNpcs.size());
    if(roleCounter > 0)
        sendToMe(RAWMSG_CODE_PUBLIC(RoleLeaveInfo), bufRoles.data(), bufRoles.size());
    else //周围没人, 直接返回
        return;

    //将自己从别人的视野中删除
	Scene::Ptr s = scene();
	if(nullptr == s)
		return;

    PublicRaw::RoleLeaveInfo send;
    send.size = 1;
    send.id[0] = id();
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(RoleLeaveInfo), &send, sizeof(send));
}

bool Role::changePos(Coord2D newPos, componet::Direction dir, MoveType type)
{
    Scene::Ptr s = scene();
    if(s == nullptr)
        return false;

    Coord2D oldPos = pos();
    if(oldPos == newPos)
        return type == MoveType::chongfeng ? true : false;
    //LOG_DEBUG("role:{}, 收到走路请求, changePos {}->{}, type={}", *this, oldPos, newPos, type);

    Grid* newGrid = s->getGridByGridCoord(newPos);
    if(newGrid == nullptr)
        return false;

    Grid* oldGrid = s->getGridByGridCoord(oldPos);
    if(oldGrid == nullptr)//不可能
        return false;

    Screen* newScreen = s->getScreenByGridCoord(newPos);
    if(newScreen == nullptr)
        return false;

    Screen* oldScreen = s->getScreenByGridCoord(oldPos);
    if(oldScreen == nullptr)
        return false;

    //距离过远
    bool legalMove = false;
    if(type == MoveType::walk)
    {
        legalMove = (oldPos.x == newPos.x || std::abs(oldPos.x - newPos.x) == 1) && (oldPos.y - newPos.y == 0 || std::abs(oldPos.y - newPos.y) == 1);
    }
    else if(type == MoveType::run)
    {
        legalMove = (oldPos.x == newPos.x || std::abs(oldPos.x - newPos.x) == 2) && (oldPos.y - newPos.y == 0 || std::abs(oldPos.y - newPos.y) == 2);
        if(legalMove)
        {
            Grid* middleGrid = s->getGridByGridCoord(Coord2D((oldPos.x + newPos.x) / 2, (oldPos.y + newPos.y) / 2));
            if(middleGrid == nullptr || !middleGrid->enterable(SceneItemType::role))
                legalMove = false;
        }
    }
    else if(type == MoveType::chongfeng || type == MoveType::hitback || type == MoveType::blink)
    {
        legalMove = true;
    }

    if(!legalMove)
    {
        correctPosToMe();
		sendSysChat("请求移动到非法位置{}, 当前位置{}", newPos, pos());
        return false;
    }

    /*
     * 判断是否走完了上一格, 防加速外挂
     */

    //从老格子到新格子
    Role::Ptr me = std::static_pointer_cast<Role>(shared_from_this());;
    if(!newGrid->addRole(me))
    {
        LOG_DEBUG("role:{}, 目标格子不可进入, changePos {}->{}", *this, oldPos, newPos);
        correctPosToMe();
        return false;
    }
    //发生了屏切换, 有离开和进入视野问题要解决
    if(newScreen != oldScreen)
    {
        if(!newScreen->addRole(me))
        {
            LOG_DEBUG("role:{}, 目标屏不可进入, changePos {}->{}", *this, oldPos, newPos);
            newGrid->eraseRole(me);
            correctPosToMe();
            return false;
        }
        oldScreen->eraseRole(me);

        //已序的新老9屏
        std::vector<Screen*> old9Screens = s->get9ScreensByGridCoord(oldPos);
        std::sort(old9Screens.begin(), old9Screens.end());
        std::vector<Screen*> new9Screens = s->get9ScreensByGridCoord(newPos);
        std::sort(new9Screens.begin(), new9Screens.end());
        //脱离视野
        std::vector<Screen*> leavedScreens;
        std::set_difference(old9Screens.begin(), old9Screens.end(),
                            new9Screens.begin(), new9Screens.end(),
                            std::back_inserter(leavedScreens));
        leaveVisualScreens(leavedScreens);
        //进入视野
        std::vector<Screen*> enteredScreens;
        std::set_difference(new9Screens.begin(), new9Screens.end(),
                            old9Screens.begin(), old9Screens.end(),
                            std::back_inserter(enteredScreens));
        enterVisualScreens(enteredScreens);
    }
    oldGrid->eraseRole(me);
    setPos(newPos);

    handleInterrupt(interrupt_operate::move, SceneItemType::role);
    setDir(dir);

	e_onPosChanged(std::static_pointer_cast<Role>(shared_from_this()), oldPos, newPos);

    if(type == MoveType::chongfeng || type == MoveType::hitback)
        return true;

    //发送新坐标给9屏内的玩家
    syncNewPosTo9Screens(type);

    return true;
}

void Role::syncNewPosTo9Screens(MoveType type) const
{
	Scene::Ptr s = scene();
	if(s == nullptr)
		return;

    PublicRaw::UpdateRolePosToClient send;
    send.rid = id();
    send.posX = pos().x;
    send.posY = pos().y;
    send.dir = static_cast<decltype(send.dir)>(dir());
    send.type = type;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(UpdateRolePosToClient), &send, sizeof(send), pos());
}

void Role::correctPosToMe()
{
    PublicRaw::SynchronizeRolePos syncPos;
    syncPos.posX = pos().x;
    syncPos.posY = pos().y;
    sendToMe(RAWMSG_CODE_PUBLIC(SynchronizeRolePos), &syncPos, sizeof(syncPos));
	sendSysChat("通知端同步角色坐标{}", pos());
}

void Role::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
    PK::timerLoop(interval, now);
    m_heroManager.timerLoop(interval, now);
	m_expArea.timerLoop(interval, now);
    switch(interval)
    {
    case StdInterval::msec_100:
        break;
    case StdInterval::msec_500:
		{
            if(checkAutoRelive(now))
            {
                reliveAreaRelive(true);
            }
		}
		break;
    case StdInterval::sec_1:
        {
            dealNotSameDay();
            if(isFeignDead())
            {
                relive(feignDeadRelivePercent() / 10);
            }
            autoAddTotalOnlineSec();
            checkAndClearGreyNameColor(now);
            judgeBoxBelong();
		}
        break;
	case StdInterval::sec_3:
		{
			autoAddHpAndMp();
		}
		break;
    case StdInterval::sec_5:
        {
            m_horse.saveDB();
            //增加合击怒气
            addAnger(Massive::me().m_jointSkillCfg.baseAnger + m_dragonHeart.extraAnger());
        }
        break;
    case StdInterval::min_1:
        {
            m_attackMode.subEvil();
            m_mail.checkMailOverdue(now);
			m_title.checkTitleOverdue(now);
            m_roleSundry.timerSaveSundry();
        }
        break;
    default:
        break;
    }
}



void Role::fillBasicData(RoleBasicData* data) const
{
    data->id  = id();
    std::memset(data->name, 0, sizeof(data->name));
    name().copy(data->name, sizeof(data->name) - 1);
    data->job = job();
    data->sex = m_sex;
}

void Role::fillMainData(RoleMainData* data) const
{
    fillBasicData(data);
    data->maxhp = getMaxHp();
    data->hp = getHp();
    data->maxmp = getMaxMp();
    data->mp = getMp();

    data->maxPAtk = getTotalPAtkMax();
    data->minPAtk = getTotalPAtkMin();
    data->maxMAtk = getTotalMAtkMax();
    data->minMAtk = getTotalMAtkMin();
    data->maxWitch = getTotalWitchMax();
    data->minWitch = getTotalWitchMin();
    data->maxPDef = getTotalPDefMax();
    data->minPDef = getTotalPDefMin();
    data->maxMDef = getTotalMDefMax();
    data->minMDef = getTotalMDefMin();
    data->shot = getTotalShot();
    data->pEscape = getTotalPEscape();
    data->mEscape = getTotalMEscape();
    data->crit = getTotalCrit();
    data->critdmg = getTotalCritDmg();
    data->antiCrit = getTotalAntiCrit();
    data->lucky = getTotalLucky();
    data->pk = m_attackMode.evilVal();
    data->dmgAddLevel = getTotalDmgAddLv();
    data->dmgReduceLevel = getTotalDmgReduceLv();
    data->hpLevel = getHpLv();
    data->mpLevel = getMpLv();

	data->level = level();
	data->gotExp = getCurLevelGotExp();
	data->needExp = getLevelUpNeedExp();
    data->expRate = 0;
    if(data->needExp > 0)
        data->expRate = 1000 * data->gotExp / data->needExp;

	data->weapon = getTplIdByObjChildType(ObjChildType::weapon);
	data->clothes = getTplIdByObjChildType(ObjChildType::clothes);
	data->wing = getTplIdByObjChildType(ObjChildType::wing);

	data->money_1 = getMoney(MoneyType::money_1);
	data->money_2 = getMoney(MoneyType::money_2);
	data->money_3 = getMoney(MoneyType::money_3);
	data->money_4 = getMoney(MoneyType::money_4);
	data->money_5 = getMoney(MoneyType::money_5);
	data->money_6 = getMoney(MoneyType::money_6);
	data->money_7 = getMoney(MoneyType::money_7);
	data->money_8 = getMoney(MoneyType::money_8);
	data->money_9 = getMoney(MoneyType::money_9);
	data->money_10 = getMoney(MoneyType::money_10);

	data->defaultCallHero = getDefaultCallHero();
	data->guanzhiLevel = m_guanzhi.level(); 

    data->factionId = factionId();
    data->factionPosition = factionPosition();
	data->turnLifeLevel = turnLifeLevel();
    std::memset(data->factionName, 0, NAME_BUFF_SZIE);
    if(m_factionId != 0)
        m_factionName.copy(data->factionName, NAME_BUFF_SZIE);
    data->anger = m_anger;
    data->energe = m_roleSundry.m_energe;
}

void Role::fillScreenData(RoleScreenData* data) const
{
    fillBasicData(data);
    data->posX = pos().x;
    data->posY = pos().y;
    data->level = level();
	data->maxhp = getMaxHp();
    data->hp = getHp();
	data->isDead = isDead();
    data->dir = static_cast<decltype(data->dir)>(dir());
    data->pkStatus = m_pkstate.pkstatus();
	
	data->weapon = getTplIdByObjChildType(ObjChildType::weapon);
	data->clothes = getTplIdByObjChildType(ObjChildType::clothes);
	data->wing = getTplIdByObjChildType(ObjChildType::wing);
    data->teamId = m_teamId;
    data->nameColor = nameColor();

	data->titleIdOfNormal = getUsedTitleIdByType(TitleType::normal);
	data->titleIdOfSpecial = getUsedTitleIdByType(TitleType::special);
	data->titleIdOfGuanzhi = getUsedTitleIdByType(TitleType::guanzhi);

    data->rideState = static_cast<uint8_t>(m_horse.rideState());
    data->skin = m_horse.curskin();
    {
        if(RideState::on == m_horse.rideState())
        {
            HeroInfoPra param = m_heroManager.getDefaultHeroInfoPra();
            data->heroJob = param.job;
            data->heroSex = param.sex;
            data->heroClother = param.clother;
        }
    }

    std::memset(data->factionName, 0, NAME_BUFF_SZIE);
    if(m_factionId != 0)
        m_factionName.copy(data->factionName, NAME_BUFF_SZIE);
    data->factionId = factionId();
	data->turnLifeLevel = turnLifeLevel();
	data->bAutoAddExp = m_expArea.isAutoAddExp();
    data->holdBoxLeftTime = holdBoxLeftTime();
	data->duanweiType = FirstManager::me().getDuanweiType(id());
    data->stall = m_stall;
}


void Role::initAttrMember()
{
    attrMembers.clear();
    attrMembers.push_back(&m_levelProps);
	attrMembers.push_back(&m_guanzhi);
	attrMembers.push_back(&m_title);
	attrMembers.push_back(&m_wash);
	attrMembers.push_back(&m_dragonBall);
}


void Role::loadSkill(std::vector<SkillData>& data)
{
    m_skillM.loadFromDB(data);
}

void Role::loadPassiveSkillCD(std::vector<PKCdStatus>& cdstatus)
{
    for(const auto& iter : cdstatus)
        m_skillM.insertPassiveSkillCD(iter);
}

void Role::sendSkillListToMe() const
{
    m_skillM.sendSkillListToMe();
}

void Role::upgradeSkill(uint32_t id, uint32_t upNum/*=1*/, bool GmFlag/*=false*/)
{
    m_skillM.upgradeSkill(id, shared_from_this(), upNum, GmFlag);
}

void Role::strengthenSkill(uint32_t id)
{
    m_skillM.strengthenSkill(id, shared_from_this());
}

void Role::loadBuff(std::vector<BuffData>& buffdata)
{
    m_buffM.loadFromDB(buffdata);
}


void Role::sendBuffListToMe() const
{
    m_buffM.sendBuffListToMe();
}

void Role::reqSelectedBuff(PKId targetId, uint8_t sceneItem)
{
    PK::Ptr target = PK::getPkptr(targetId, static_cast<SceneItemType>(sceneItem));
    if(nullptr == target)
        return;

     target->m_buffM.retBuffList(shared_from_this());
}

void Role::reqSelectedBuffTips(PKId targetId, uint8_t sceneItem, uint32_t buffId)
{
    PK::Ptr target = PK::getPkptr(targetId, static_cast<SceneItemType>(sceneItem));
    if(nullptr == target)
        return;

    target->m_buffM.retBuffTips(shared_from_this(), buffId);
}

void Role::setDead(bool value)
{
	m_dead = value;
}

void Role::dieDropEquip()
{
    auto s = scene();
    if(nullptr == s || 0 == s->dropEquip())
        return;
    uint32_t antiDropEquip = getTotalAntiDropEquip();
    if(antiDropEquip >= 10000)
        return;

    std::vector<RoleId> owners;
    owners.push_back(id());
    std::vector<SceneObject::Ptr> dropObjList;
    auto rolePackageDropExec = [&, this] (CellInfo& cellInfo) -> bool
    {
        if(nullptr == cellInfo.objPtr
           || Bind::yes == cellInfo.bind)
            return true;

        //罪恶值系数
        uint64_t evilParam = 1000 * (1 + m_attackMode.evilVal() * Massive::me().m_dropCfg.evilParam1 / 1000 / (m_attackMode.evilVal() * Massive::me().m_dropCfg.evilParam2 / 1000 + Massive::me().m_dropCfg.constant));
        //掉率
        uint64_t dropRate = Massive::me().m_dropCfg.normalPackage * cellInfo.objPtr->prob() * evilParam * (1-antiDropEquip/10000) / 100 / 1000;
        Random<uint64_t> r(1,10000);
        uint64_t rand = r.get();
        if(rand > dropRate)
        {
            LOG_DEBUG("死亡掉落, 概率性失败, dropRate:{} rand={}", dropRate, rand);
            return true;
        }

        //掉落
        SceneObject::Ptr sceneObj = SceneObjectManager::me().createObj(cellInfo.objPtr->tplId(), cellInfo.item, Bind::no, owners, Clock::now() + std::chrono::seconds {Massive::me().m_dropCfg.ownerTime});
        if(nullptr == sceneObj)
            return true;

        LOG_TRACE("死亡掉落, role={}, obj({}, {}, {}), packageType={}, cell={}", *this, cellInfo.objPtr->name(), cellInfo.objPtr->tplId(), cellInfo.item, PackageType::role, cellInfo.cell);
        if(nullptr != m_packageSet.eraseObjByCell(cellInfo.cell, PackageType::role, "角色死亡掉落"))
            dropObjList.push_back(sceneObj);
        return true;
    };

    auto roleEquipPackageDropExec = [&, this] (CellInfo& cellInfo) -> bool
    {
        if(nullptr == cellInfo.objPtr
           || Bind::yes == cellInfo.bind)
            return true;

        //罪恶值系数
        uint64_t evilParam = 1000 * (1 + m_attackMode.evilVal() * Massive::me().m_dropCfg.evilParam1 / 1000 / (m_attackMode.evilVal() * Massive::me().m_dropCfg.evilParam2 / 1000 + Massive::me().m_dropCfg.constant));
        //掉率
        uint64_t dropRate = Massive::me().m_dropCfg.equipPackage * cellInfo.objPtr->prob() * evilParam * (1-antiDropEquip/10000) / 100 / 1000;
        Random<uint64_t> r(1,10000);
        uint64_t rand = r.get();
        if(rand > dropRate)
        {
            LOG_DEBUG("死亡掉落, 概率性失败, dropRate:{} rand={}", dropRate, rand);
            return true;
        }

        //掉落
        SceneObject::Ptr sceneObj = SceneObjectManager::me().createObj(cellInfo.objPtr->tplId(), cellInfo.item, Bind::no, owners, Clock::now() + std::chrono::seconds {Massive::me().m_dropCfg.ownerTime});
        if(nullptr == sceneObj)
            return true;

        LOG_TRACE("死亡掉落, role={}, obj({}, {}, {}), packageType={}, cell={}", *this, cellInfo.objPtr->name(), cellInfo.objPtr->tplId(), cellInfo.item, PackageType::equipOfRole, cellInfo.cell);
        if(nullptr != m_packageSet.eraseObjByCell(cellInfo.cell, PackageType::equipOfRole, "角色死亡掉落"))
            dropObjList.push_back(sceneObj);
        return true;
    };
    m_packageSet.execPackageCell(PackageType::role, rolePackageDropExec);
    m_packageSet.execPackageCell(PackageType::equipOfRole, roleEquipPackageDropExec);

    s->addSceneObj(pos(), dropObjList);
}

bool Role::isDead() const
{
    return m_dead;
}


void Role::toDie(PK::Ptr attacker)
{
    handleInterrupt(interrupt_operate::dead, nullptr == attacker ? SceneItemType::role : attacker->sceneItemType());
	m_dead = true;
    m_skillEffectM.clear();
    m_buffM.processDeath();

	setDeathTimePoint(Clock::now());
	m_heroManager.roleDie();

    //爆装备的时候请注意syncRoleDie中的假死状态, 假死是不会爆装备的
	syncRoleDie(attacker);
    //成为仇人
    addEnemy(attacker);
    attacker->addEvil(shared_from_this());

    if(!isFeignDead())
        dieDropEquip();

    auto petPtr = pet();
    if(nullptr != petPtr)
        petPtr->erase(true);

	Role::Ptr role = std::static_pointer_cast<Role>(shared_from_this());
	if(role == nullptr)
		return;

	BubblePointManager::me().roleBeKilled(role);
	FirstManager::me().roleBeKilled(role, getRole(attacker));
	
	Scene::Ptr scenePtr = scene();
	if(scenePtr == nullptr)
		return;

	scenePtr->addDeadRole(id(), pos());
	return;
}


void Role::handleInterrupt(interrupt_operate op, SceneItemType interruptModel)
{
    PK::handleInterrupt(op, interruptModel);
    switch(op)
    {
    case interrupt_operate::move:
        {
            m_roleStall.closeStall();
        }
        break;
    case interrupt_operate::attack:
        {
            m_horse.offRide();
            m_roleStall.closeStall();
        }
        break;
    case interrupt_operate::beAttack:
        {
            if(interruptModel != SceneItemType::npc)
			{
				m_horse.offRide();
			}
        }
        break;
    case interrupt_operate::dead:
        {
            m_horse.offRide(true);
            WorldBoss::me().clearHoldBoxState(std::static_pointer_cast<Role>(shared_from_this()));
            ShaBaKe::me().subFactionRole(std::static_pointer_cast<Role>(shared_from_this()));
        }
        break;
    case interrupt_operate::leaveScene:
        {
            WorldBoss::me().clearHoldBoxState(std::static_pointer_cast<Role>(shared_from_this()));
            m_roleStall.closeStall();
            ShaBaKe::me().subFactionRole(std::static_pointer_cast<Role>(shared_from_this()));
        }
        break;
    default:
        break;
    }

	TradeManager::me().cancleTrade(id());
    interruptCollect();
}

void Role::lockOnTarget(PK::Ptr target)
{
    m_target = target;
}

PK::Ptr Role::target() const
{
    return m_target.lock();
}

void Role::setLevel(uint32_t level)
{
    PK::setLevel(level);
    m_levelProps.setLevel(level);
}

void Role::setTurnLifeLevel(TurnLife level)
{
	TurnLife oldLevel = turnLifeLevel();

	PK::setTurnLifeLevel(level);

	m_zhuansheng.calcAttribute();
	updateTurnLifeLevelToDB();		
	LOG_TRACE("转生, 转生成功! name={}, id={}, level={}->level={}", 
			  name(), id(), oldLevel, level);
	return;
}

Sex Role::sex() const
{
	return m_sex;
}

bool Role::updateTurnLifeLevelToDB()
{
	PrivateRaw::UpdateRoleTurnLifeLevel send;
	send.roleId = id();
	send.turnLifeLevel = turnLifeLevel();

	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateRoleTurnLifeLevel), &send, sizeof(send));
	
	return ret;
}

void Role::autoAddHpAndMp()
{
	if(isDead())
		return;

    uint32_t maxHp = getMaxHp();
    uint32_t maxMp = getMaxMp();
	uint32_t addHp = MAX(SAFE_DIV(maxHp * 50, 1000), 10);
	uint32_t addMp = MAX(SAFE_DIV(maxMp * 50, 1000), 10);
	if(isFight())
	{
		addHp = MAX(SAFE_DIV(maxHp * 2, 1000), 2);
		addMp = MAX(SAFE_DIV(maxMp * 2, 1000), 2);
	}

	if(getHp() + addHp >= maxHp)
	{
		addHp = SAFE_SUB(maxHp, getHp());
	}
	if(getMp() + addMp >= maxMp)
	{
		addMp = SAFE_SUB(maxMp, getMp());
	}

	if(0 != addHp)
	{
        changeHpAndNotify(nullptr, addHp, HPChangeType::recovery);
	}
	if(0 != addMp)
	{
        changeMpAndNotify(addMp);
	}
}



/*
 * 生命上限
 */
uint32_t Role::getMaxHp() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(maxhp);
    }
	
	ret += m_packageSet.getHp(PackageType::equipOfRole);
	ret += m_packageSet.getHp(PackageType::stoneOfRole);
    ret += m_horse.getMaxHp(SceneItemType::role);

    //计算有效生命
    ret = ret * (1000 + getHpRatio() + (std::pow(getHpLv(), PK_PARAM.hpParam1()/1000) / PK_PARAM.hpParam2()/1000 - PK_PARAM.hpParam3()/1000) * 1000) / 1000;

    //buff计算放最后面, 下面的所有属性计算都应满足这一点
    ret = m_buffM.getHpMpBuffConstProp(PropertyType::maxhp, ret);
	return ret;
}


/*
 * 生命加成
 */
uint32_t Role::getHpRatio() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(addhpRatio);
    }

    ret += m_horse.getHpRatio(SceneItemType::role);
    ret = m_buffM.getHpMpBuffPercentProp(PropertyType::addhpRatio, ret);
    return ret;
}


/*
 * 生命等级
 */
uint32_t Role::getHpLv() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(hpLv);
    }
	
	ret += m_packageSet.getHpLv(PackageType::equipOfRole);
	ret += m_packageSet.getHpLv(PackageType::stoneOfRole);
    ret += m_horse.getHpLv(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::hpLv, ret);
	return ret;
}



/*
 * 魔法上限
 */
uint32_t Role::getMaxMp() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(maxmp);
    }

	ret += m_packageSet.getMp(PackageType::equipOfRole);
	ret += m_packageSet.getMp(PackageType::stoneOfRole);
    ret += m_horse.getMaxMp(SceneItemType::role);

    //有效魔法
    ret = ret * (1000 + getMpRatio() + (std::pow(getMpLv(), PK_PARAM.mpParam1()/1000) / PK_PARAM.mpParam2()/1000 - PK_PARAM.mpParam3()/1000) * 1000) / 1000;

    ret = m_buffM.getHpMpBuffConstProp(PropertyType::maxmp, ret);
    return ret;
}


/*
 * 魔法加成
 */
uint32_t Role::getMpRatio() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(addmpRatio);
    }

    ret += m_horse.getMpRatio(SceneItemType::role);
    ret = m_buffM.getHpMpBuffPercentProp(PropertyType::addmpRatio, ret);
    return ret;
}


/*
 * 魔法等级
 */
uint32_t Role::getMpLv() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(mpLv);
    }

	ret += m_packageSet.getMpLv(PackageType::equipOfRole);
	ret += m_packageSet.getMpLv(PackageType::stoneOfRole);
    ret += m_horse.getMpLv(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::mpLv, ret);
    return ret;
}



/*
 * 最小物攻
 */
uint32_t Role::getTotalPAtkMin() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_attackMin);
    }

	ret += m_packageSet.getPAtkMin(PackageType::equipOfRole);
	ret += m_packageSet.getPAtkMin(PackageType::stoneOfRole);
    ret += m_horse.getPAtkMin(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::p_attackMin, ret);
    return ret;
}


/*
 * 最大物攻
 */
uint32_t Role::getTotalPAtkMax() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_attackMax);
    }

	ret += m_packageSet.getPAtkMax(PackageType::equipOfRole);
	ret += m_packageSet.getPAtkMax(PackageType::stoneOfRole);
    ret += m_horse.getPAtkMax(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::p_attackMax, ret);
    return ret;
}


/*
 * 最小魔攻
 */
uint32_t Role::getTotalMAtkMin() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_attackMin);
    }

	ret += m_packageSet.getMAtkMin(PackageType::equipOfRole);
	ret += m_packageSet.getMAtkMin(PackageType::stoneOfRole);
    ret += m_horse.getMAtkMin(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::m_attackMin, ret);
    return ret;
}


/*
 * 最大魔攻
 */
uint32_t Role::getTotalMAtkMax() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_attackMax);
    }

	ret += m_packageSet.getMAtkMax(PackageType::equipOfRole);
	ret += m_packageSet.getMAtkMax(PackageType::stoneOfRole);
    ret += m_horse.getMAtkMax(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::m_attackMax, ret);
    return ret;
}


/*
 * 最小道术
 */
uint32_t Role::getTotalWitchMin() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(witchMin);
    }

	ret += m_packageSet.getWitchMin(PackageType::equipOfRole);
	ret += m_packageSet.getWitchMin(PackageType::stoneOfRole);
    ret += m_horse.getWitchMin(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::witchMin, ret);
    return ret;
}


/*
 * 最大道术
 */
uint32_t Role::getTotalWitchMax() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(witchMax);
    }

	ret += m_packageSet.getWitchMax(PackageType::equipOfRole);
	ret += m_packageSet.getWitchMax(PackageType::stoneOfRole);
    ret += m_horse.getWitchMax(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::witchMax, ret);
    return ret;
}


/*
 * min物防
 */
uint32_t Role::getTotalPDefMin() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_defenceMin);
    }

	ret += m_packageSet.getPDefMin(PackageType::equipOfRole);
	ret += m_packageSet.getPDefMin(PackageType::stoneOfRole);
    ret += m_horse.getPDefMin(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::p_defenceMin, ret);
    return ret;
}


/*
 * max物防
 */
uint32_t Role::getTotalPDefMax() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_defenceMax);
    }

	ret += m_packageSet.getPDefMax(PackageType::equipOfRole);
	ret += m_packageSet.getPDefMax(PackageType::stoneOfRole);
    ret += m_horse.getPDefMax(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::p_defenceMax, ret);
    return ret;
}


/*
 * min魔防
 */
uint32_t Role::getTotalMDefMin() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_defenceMin);
    }

	ret += m_packageSet.getMDefMin(PackageType::equipOfRole);
	ret += m_packageSet.getMDefMin(PackageType::stoneOfRole);
    ret += m_horse.getMDefMin(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::m_defenceMin, ret);
    return ret;
}


/*
 * max魔防
 */
uint32_t Role::getTotalMDefMax() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_defenceMax);
    }

	ret += m_packageSet.getMDefMax(PackageType::equipOfRole);
	ret += m_packageSet.getMDefMax(PackageType::stoneOfRole);
    ret += m_horse.getMDefMax(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::m_defenceMax, ret);
    return ret;
}


/*
 * 幸运值
 */
uint32_t Role::getTotalLucky() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(lucky);
    }

	ret += m_packageSet.getLucky(PackageType::equipOfRole);
	ret += m_packageSet.getLucky(PackageType::stoneOfRole);
    ret += m_horse.getLucky(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::lucky, ret);
    return ret;
}


/*
 * 诅咒值
 */
uint32_t Role::getTotalEvil() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(evil);
    }

	ret += m_packageSet.getEvil(PackageType::equipOfRole);
	ret += m_packageSet.getEvil(PackageType::stoneOfRole);
    ret += m_horse.getEvil(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::evil, ret);
	return ret;
}


/*
 * 命中
 */
uint32_t Role::getTotalShot() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(shot);
    }

	ret += m_packageSet.getShot(PackageType::equipOfRole);
	ret += m_packageSet.getShot(PackageType::stoneOfRole);
    ret += m_horse.getShot(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::shot, ret);
    return ret;
}


/*
 * 命中率
 */
uint32_t Role::getTotalShotRatio() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(shotRatio);
    }
 
	ret += m_packageSet.getShotRatio(PackageType::equipOfRole);
	ret += m_packageSet.getShotRatio(PackageType::stoneOfRole);
    ret += m_horse.getShotRatio(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::shotRatio, ret);
	return ret;
}


/*
 * 物闪
 */
uint32_t Role::getTotalPEscape() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(p_escape);
    }

	ret += m_packageSet.getPEscape(PackageType::equipOfRole);
	ret += m_packageSet.getPEscape(PackageType::stoneOfRole);
    ret += m_horse.getPEscape(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::p_escape, ret);
    return ret;
}


/*
 * 魔闪
 */
uint32_t Role::getTotalMEscape() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(m_escape);
    }

	ret += m_packageSet.getMEscape(PackageType::equipOfRole);
	ret += m_packageSet.getMEscape(PackageType::stoneOfRole);
    ret += m_horse.getMEscape(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::m_escape, ret);
    return ret;
}


/*
 * 闪避率
 */
uint32_t Role::getTotalEscapeRatio() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(escapeRatio);
    }
   
	ret += m_packageSet.getEscapeRatio(PackageType::equipOfRole);
	ret += m_packageSet.getEscapeRatio(PackageType::stoneOfRole);
    ret += m_horse.getEscapeRatio(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::escapeRatio, ret);
	return ret;
}


/*
 * 暴击
 */
uint32_t Role::getTotalCrit() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(crit);
    }

	ret += m_packageSet.getCrit(PackageType::equipOfRole);
	ret += m_packageSet.getCrit(PackageType::stoneOfRole);
    ret += m_horse.getCrit(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::crit, ret);
    return ret;
}


/*
 * 暴击率
 */
uint32_t Role::getTotalCritRatio() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(critRatio);
    }

	ret += m_packageSet.getCritRatio(PackageType::equipOfRole);
	ret += m_packageSet.getCritRatio(PackageType::stoneOfRole);
    ret += m_horse.getCritRatio(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::critRatio, ret);
    return ret;
}


/*
 * 坚韧
 */
uint32_t Role::getTotalAntiCrit() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(antiCrit);
    }

	ret += m_packageSet.getAntiCrit(PackageType::equipOfRole);
	ret += m_packageSet.getAntiCrit(PackageType::stoneOfRole);
    ret += m_horse.getAntiCrit(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::antiCrit, ret);
    return ret;
}


/*
 * 暴伤
 */
uint32_t Role::getTotalCritDmg() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(critDamage);
    }

	ret += m_packageSet.getCritDamage(PackageType::equipOfRole);
	ret += m_packageSet.getCritDamage(PackageType::stoneOfRole);
    ret += m_horse.getCritDmg(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::critDamage, ret);
    return ret;
}


/*
 * 增伤
 */
uint32_t Role::getTotalDmgAdd() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(damageAdd);
    }

	ret +=  m_packageSet.getDamageAdd(PackageType::equipOfRole);
	ret +=  m_packageSet.getDamageAdd(PackageType::stoneOfRole);
    ret += m_horse.getDmgAdd(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::damageAdd, ret);
    return ret;
}


/*
 * 增伤等级
 */
uint32_t Role::getTotalDmgAddLv() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(damageAddLv);
    }

	ret += m_packageSet.getDamageAddLv(PackageType::equipOfRole);
	ret += m_packageSet.getDamageAddLv(PackageType::stoneOfRole);
    ret += m_horse.getDmgAddLv(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::damageAddLv, ret);
    return ret;
}


/*
 * 减伤
 */
uint32_t Role::getTotalDmgReduce() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(damageReduce);
    }

	ret += m_packageSet.getDamageReduce(PackageType::equipOfRole);
	ret += m_packageSet.getDamageReduce(PackageType::stoneOfRole);
    ret += m_horse.getDmgReduce(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::damageReduce, ret);
    return ret;
}


/*
 * 减伤等级
 */
uint32_t Role::getTotalDmgReduceLv() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(damageReduceLv);
    }

	ret += m_packageSet.getDamageReduceLv(PackageType::equipOfRole);
	ret += m_packageSet.getDamageReduceLv(PackageType::stoneOfRole);
    ret += m_horse.getDmgReduceLv(SceneItemType::role);
    ret = m_buffM.getBuffProps(PropertyType::damageReduceLv, ret);
    return ret;
}

/*
 * 防爆装备属性
 */
uint32_t Role::getTotalAntiDropEquip() const
{
    uint32_t ret = 0;
    for(const auto& iter : attrMembers)
    {
        ret += iter->GET_ATTRIBUTE(antiDropEquip);
    }

    ret += m_packageSet.getAntiDropEquip(PackageType::equipOfRole);
    ret += m_packageSet.getAntiDropEquip(PackageType::stoneOfRole);
    ret += m_horse.getAntiDropEquip(SceneItemType::role);
    return ret;
}

void Role::setDeathTimePoint(const TimePoint& dieTimePoint)
{
	m_deathTimePoint = dieTimePoint;
	sendSysChat("死亡复活, 死亡时间: {}", toUnixTime(m_deathTimePoint));
	LOG_DEBUG("死亡复活, name={}, id={}, 死亡时间: {}", 
			  name(), id(), toUnixTime(m_deathTimePoint));
}

TimePoint Role::getDeathTimePoint() const
{
	return m_deathTimePoint;
}

void Role::addExp(uint64_t exp)
{
	if(0 == exp)
		return;

	uint64_t oldExp = m_exp;
	m_exp += exp;
    
	//获得组队加成
    uint64_t teamAddExp = exp * RelationsManager::me().getTeamMeberExtraAddExpRatio(std::static_pointer_cast<Role>(shared_from_this()));
    m_exp += teamAddExp;
    m_exp += exp * m_buffM.roleExpAddPercent();

    LOG_TRACE("角色, 获得经验, exp={}->{}, addExp={}, role={}, teamAddExp={}", oldExp, m_exp, exp, *this, teamAddExp);

	if(!updateLevelAndExpToDB())
		return;

	judgeLevelUp();
	sendRoleCurLevelGotExp();
	sendSysChat(ChannelType::screen_right_down, "获得 主角经验: {}", exp);
	return;
}

uint64_t Role::getExp() const
{
	return m_exp;
}

void Role::judgeLevelUp()
{
	uint32_t levelUpNum = getRoleCanLevelUpNum();
	if(0 == levelUpNum)
		return;

	for(uint32_t i = 0; i < levelUpNum; i++)
	{
		levelUp();
	}

	// S -> C
	sendMainToMe();
    syncScreenDataTo9();
	return;
}

uint32_t Role::getRoleCanLevelUpNum() const
{
	uint32_t curLevel = level();
	const auto& cfg = ExpConfig::me().expCfg;
	if(curLevel >= cfg.m_expMap.size())
		return 0;

	uint32_t count = 0;
	for(auto pos = cfg.m_expMap.begin(); pos != cfg.m_expMap.end(); ++pos)
	{
		if(getExp() < pos->second.needExp_role)
			continue;
	
		if(turnLifeLevel() < pos->second.needTurnLife)
			break;

		count++;
	}

	if(count <= curLevel)
		return 0;

	return SAFE_SUB(count, curLevel);
}

void Role::levelUp(uint32_t upNum/* = 1*/, bool GmFlag/* = false*/)
{
	uint32_t curLevel = level();
	uint32_t nextLevel = curLevel + upNum;
    
	const auto& cfg = ExpConfig::me().expCfg;
    if(curLevel >= cfg.m_expMap.size())
        return;

    auto pos = cfg.m_expMap.find(nextLevel);
    if(pos == cfg.m_expMap.end())
        return;

    const uint64_t needExp = pos->second.needExp_role;

    if(GmFlag)
        m_exp = needExp;
    if(m_exp < needExp)
        return;

	setLevel(nextLevel);

	//策划要求升级后满血满蓝
	setHp(getMaxHp());
	setMp(getMaxMp());

	// world -> db
	if(!updateLevelAndExpToDB())
		return;
    //world -> func
    updateFuncAndSessionRoleLevel();

	if(GmFlag)
	{
		// S -> C
		sendMainToMe();
		syncScreenDataTo9();
	}

	LOG_TRACE("角色, 升级, level={}->{}, exp={}, needExp={}", 
			  curLevel, nextLevel, m_exp, needExp);

    m_roleTask.checkAndUnlockTask();
    m_skillM.unlockSkill();
	m_dailyTask.roleLevelUp();
	return;
}

bool Role::updateLevelAndExpToDB()
{
	// world -> db
	PrivateRaw::UpdateRoleLevelExp send;
	send.rid = id();
	send.level = level();
	send.exp = getExp();

	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateRoleLevelExp), &send, sizeof(send));
	LOG_TRACE("角色，经验，send UpdateRoleLevelExp to {}, {}, roleId={}, level={}, exp={}",
			  dbcachedId, ret ? "ok" : "falied",
			  send.rid, send.level, send.exp);

	return ret;
}

void Role::updateFuncAndSessionRoleLevel()
{
    PrivateRaw::UpdateFuncAndSessionRoleLevel send;
    send.roleId = id();
    send.level = level();
    ProcessIdentity funcId("func", 1);
    ProcessIdentity sessionId("session", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(UpdateFuncAndSessionRoleLevel), &send, sizeof(send));
    World::me().sendToPrivate(sessionId, RAWMSG_CODE_PRIVATE(UpdateFuncAndSessionRoleLevel), &send, sizeof(send));
}

void Role::sendRoleCurLevelGotExp()
{
	PublicRaw::RetRoleCurLevelGotExp send;
	send.gotExp = getCurLevelGotExp();
    send.expRate = 0;
    uint64_t needExp = getLevelUpNeedExp();
    if(needExp > 0)
        send.expRate = 1000 * send.gotExp / getLevelUpNeedExp();
	sendToMe(RAWMSG_CODE_PUBLIC(RetRoleCurLevelGotExp), &send, sizeof(send));
}

//当前等级已获得的经验值 == 累计经验值 - 当前等级需要经验值
uint64_t Role::getCurLevelGotExp() const
{
	const auto& cfg = ExpConfig::me().expCfg;
	auto pos = cfg.m_expMap.find(level());
	if(pos == cfg.m_expMap.end())
		return 0;

	uint64_t curLevelNeedExp = pos->second.needExp_role;
	return SAFE_SUB(getExp(), curLevelNeedExp);
}

// 升级需要经验值 == 下一级需要经验值 - 当前等级需要经验值
uint64_t Role::getLevelUpNeedExp() const
{
	uint32_t curLevel = level();
	uint32_t nextLevel = curLevel + 1;
	
	const auto& cfg = ExpConfig::me().expCfg;
	auto iterCur = cfg.m_expMap.find(curLevel);
	if(iterCur == cfg.m_expMap.end())
		return 0;

	uint64_t curLevelNeedExp = iterCur->second.needExp_role;
	if(curLevel == cfg.m_expMap.size())	//当前为最大等级时，则累计经验值 - 当前等级需要经验值
	{
		auto pos = cfg.m_expMap.find(curLevel);
		if(pos == cfg.m_expMap.end())
			return 0;

		return SAFE_SUB(getExp(), curLevelNeedExp);
	}

	auto iterNext = cfg.m_expMap.find(nextLevel);
	if(iterNext == cfg.m_expMap.end())
		return 0;

	uint64_t nextLevelNeedExp = iterNext->second.needExp_role;
	return SAFE_SUB(nextLevelNeedExp, curLevelNeedExp);
}

//通知客户端角色死亡
void Role::syncRoleDie(PK::Ptr attacker)
{
	if(!isDead())
		return;

	Scene::Ptr s = scene();
	if(s == nullptr)
		return;

    if(dealFeignDead())
    {
        //假死
        //PublicRaw::RoleFeignDeath send;
        //sendToMe(RAWMSG_CODE_PUBLIC(RoleFeignDeath), &send, sizeof(send));
    }
    else
    {
        const auto& cfg = ReliveConfig::me().reliveCfg;
        std::string winnerName = "无名氏";
        if(attacker != nullptr)
        {
            winnerName = attacker->name();
        }

        PublicRaw::RetRoleDie send;
        send.roleId = id();
        std::strncpy(send.attackerName, winnerName.c_str(), NAME_BUFF_SZIE);
        send.dieTimer = toUnixTime(getDeathTimePoint());
        send.sec = cfg.sec;
        sendToMe(RAWMSG_CODE_PUBLIC(RetRoleDie), &send, sizeof(send));
    }

	PublicRaw::RetRoleDieToNine sendNine;
	sendNine.roleId = id();
	s->sendCmdTo9(RAWMSG_CODE_PUBLIC(RetRoleDieToNine), &sendNine, sizeof(sendNine), pos());
}


//请求复活角色
void Role::requestRelive(const uint8_t* msgData, uint32_t msgSize)
{
	auto rev = reinterpret_cast<const PublicRaw::RequestReliveRole*>(msgData);
	if(!rev)
		return;
    LOG_DEBUG("复活, role:{}, sceneid:{}, 收到端复活请求,类型:{}", name(), sceneId(), rev->type);

	switch(rev->type)
	{
	case ReliveType::reliveArea:
		reliveAreaRelive();
		break;
	case ReliveType::perfect:
		perfectRelive();
		break;
	case ReliveType::weak:
		weakRelive();
		break;
	default:
		break;
	}

	return;
}

//检查是否可以自动回城复活
bool Role::checkAutoRelive(const TimePoint& now)
{
	if(!isDead())
		return false;

	if(EPOCH == getDeathTimePoint())
		return false;

	const auto& cfg = ReliveConfig::me().reliveCfg;
	if(now >= getDeathTimePoint() + std::chrono::seconds {cfg.sec})
		return true;

	return false;
}

//复活点复活
void Role::reliveAreaRelive(bool autoRelive/* = false*/)
{
	Scene::Ptr scenePtr = scene();
	if(nullptr == scenePtr)
		return;

	if(scenePtr->copyType() == CopyMap::first_pk)
	{
		goBackCityRelive();
        return;
	}
    else if(scenePtr->copyType() == CopyMap::shabake)
    {
        ShaBaKe::me().dealRelive(std::static_pointer_cast<Role>(shared_from_this()), scenePtr, autoRelive);
        return;
    }

	if(scenePtr->existReliveArea())
        reliveByDefaultReliveArea(autoRelive);
    else
        goBackCityRelive();
}

//主城复活
void Role::goBackCityRelive()
{
	if(!isDead())
		return;

	Scene::Ptr curScene = scene();
	if(curScene == nullptr)
		return;

	relive();
	if(curScene->id() == ReliveConfig::me().cityId())
	{
		changePos(ReliveConfig::me().cityRelivePos(), dir(), MoveType::blink);
	}
	else
	{
		RolesAndScenes::me().gotoOtherScene(id(), ReliveConfig::me().cityId(), ReliveConfig::me().cityRelivePos());
	}

	return;
}

//完美原地复活
void Role::perfectRelive()
{
	if(!isDead())
		return;

	Scene::Ptr scenePtr = scene();
	if(nullptr == scenePtr)
		return;
    if(scenePtr->copyType() == CopyMap::shabake)
        return;
	
	const auto& cfg = ReliveConfig::me().reliveCfg;
	auto pos = cfg.reliveMap.find((uint8_t)ReliveType::perfect);
	if(pos == cfg.reliveMap.end())
		return;

	uint32_t needTplId = pos->second.tplId;
	uint32_t needNum = pos->second.needNum;

	uint32_t objNum = m_packageSet.getObjNum(needTplId, PackageType::role);
	if(needNum > objNum)
	{
		sendSysChat("复活丹不足");
		return;
	}

	if(!m_packageSet.eraseObj(needTplId, needNum, PackageType::role, "完美复活"))
		return;
		
	relive();
	return;
}

//虚弱原地复活
void Role::weakRelive()
{
	if(!isDead())
		return;

	Scene::Ptr scenePtr = scene();
	if(nullptr == scenePtr)
		return;
    if(scenePtr->copyType() == CopyMap::shabake)
        return;
	
	const auto& cfg = ReliveConfig::me().reliveCfg;
	auto pos = cfg.reliveMap.find((uint8_t)ReliveType::weak);
	if(pos == cfg.reliveMap.end())
		return;

	uint32_t needTplId = pos->second.tplId;
	uint32_t needNum = pos->second.needNum;

	uint32_t objNum = m_packageSet.getObjNum(needTplId, PackageType::role);
	if(needNum > objNum)
	{
		sendSysChat("返生丹不足");
		return;
	}

	if(!m_packageSet.eraseObj(needTplId, needNum, PackageType::role, "虚弱复活"))
		return;
	
	relive(pos->second.percent);
	return;
}

//到本地图默认复活点复活
void Role::reliveByDefaultReliveArea(bool autoRelive/* = false*/)
{
	if(!isDead())
		return;

	Scene::Ptr scenePtr = scene();
	if(nullptr == scenePtr)
		return;

    if(!autoRelive 
       && scenePtr->reliveYB() > 0 
       && !reduceMoney(MoneyType::money_4, scenePtr->reliveYB(), "复活消耗"))
        return;

	relive();
	changePos(scenePtr->randomRelivePos(), dir(), MoveType::blink);
	return;
}

void Role::relive(const uint32_t percent /* = 100 */)
{
	if(!isDead())
		return;

	uint32_t newHp = SAFE_DIV(getMaxHp() * percent, 100);
	uint32_t newMp = SAFE_DIV(getMaxMp() * percent, 100);

	Scene::Ptr scenePtr = scene();
	if(scenePtr != nullptr)
	{
		scenePtr->eraseDeadRole(id(), pos());
	}
	setHp(newHp);
	setMp(newMp);
	setDead(false);
	setDeathTimePoint(EPOCH);
    clearFeignDead();
	sendMainToMe();
	syncScreenDataTo9();

	sendSysChat("死亡复活, 复活时间: {}", toUnixTime(Clock::now()));
	LOG_DEBUG("死亡复活, name={}, id={}, 复活时间: {}", 
			  name(), id(), toUnixTime(Clock::now()));
	return;
}

void Role::addEnemy(PK::Ptr atk)
{
    if(atk->sceneItemType() == SceneItemType::npc)
       return;
    auto s = scene();
    if(nullptr != s && 0 == s->crime())
        return;
    PrivateRaw::InsertEnemy send;
    send.roleId = id();

    if(atk->sceneItemType() == SceneItemType::role)
    {
        send.enemyId = atk->id();
    }
    if(atk->sceneItemType() == SceneItemType::hero)
    {
        auto hero = std::static_pointer_cast<Hero>(atk);
        send.enemyId = hero->getOwnerId();    
    }
    send.beKilledType = BeKilledType::role;
	ProcessIdentity dbcachedId("func", 1);
	World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(InsertEnemy), &send, sizeof(send));
    LOG_DEBUG("添加仇人, 向func发送仇人信息, roleId = {}, enemyId = {}",
              send.roleId, send.enemyId);
}

TplId Role::getTplIdByObjChildType(ObjChildType childType) const
{
	return m_packageSet.getTplIdByObjChildType(childType, PackageType::equipOfRole);
}

std::string Role::getMoneyName(MoneyType type) const
{
	switch(type)
	{
	case MoneyType::money_1:
		return "绑定金币";
	case MoneyType::money_2:
		return "金币";
	case MoneyType::money_3:
		return "绑定元宝";
	case MoneyType::money_4:
		return "元宝";
	case MoneyType::money_5:
		return "声望";
	case MoneyType::money_6:
		return "强化值";
	case MoneyType::money_7:
		return "战功";
	case MoneyType::money_8:
		return "角色灵力值";
	case MoneyType::money_9:
		return "英雄灵力值";
	case MoneyType::money_10:
		return "龙魂";
	default:
		break;
	}

	return "";
}

uint64_t Role::getMoney(MoneyType type) const
{
	switch(type)
	{
	case MoneyType::money_1: 
		return m_money_1;
	case MoneyType::money_2:
		return m_money_2;
	case MoneyType::money_3:
		return m_money_3;
	case MoneyType::money_4:
		return m_money_4;
	case MoneyType::money_5:
		return m_money_5;
	case MoneyType::money_6:
		return m_money_6;
	case MoneyType::money_7:
		return m_money_7;
	case MoneyType::money_8:
		return m_money_8;
	case MoneyType::money_9:
		return m_money_9;
	case MoneyType::money_10:
		return m_money_10;
	default:
		break;
	}

	return (uint64_t)-1;
}

bool Role::setMoney(MoneyType type, uint64_t money)
{
	switch(type)
	{
	case MoneyType::money_1:
		m_money_1 = money;
		break;
	case MoneyType::money_2:
		m_money_2 = money;
		break;
	case MoneyType::money_3:
		m_money_3 = money;
		break;
	case MoneyType::money_4:
		m_money_4 = money;
		break;
	case MoneyType::money_5:
		m_money_5 = money;
		break;
	case MoneyType::money_6:
		m_money_6 = money;
		break;
	case MoneyType::money_7:
		m_money_7 = money;
		break;
	case MoneyType::money_8:
		m_money_8 = money;
		break;
	case MoneyType::money_9:
		m_money_9 = money;
		break;
	case MoneyType::money_10:
		m_money_10 = money;
		break;
	default:
		return false;
	}

	return true;
}

bool Role::checkMoney(MoneyType type, uint64_t needMoney)
{
	const uint64_t totalMoney = getMoney(type);
	if((uint64_t)-1 == totalMoney)
		return false;

	//检查绑定金币时，特殊处理, 因绑定金币不足时，可扣非绑金币
	if(type == MoneyType::money_1)
	{
		const uint64_t goldNum = getMoney(MoneyType::money_2);
		if((uint64_t)-1 == goldNum)
			return false;

		if(totalMoney + goldNum >= needMoney)
			return true;
	}
	else
	{
		if(totalMoney >= needMoney)
			return true;
	}

	sendSysChat("{}不足", getMoneyName(type));
	return false;
}

bool Role::addMoneyPrivate(MoneyType type, uint64_t money, const std::string& text)
{
	const uint64_t oldMoney = getMoney(type);
	if((uint64_t)-1 == oldMoney)
		return false;

	const uint64_t newMoney = oldMoney + money;
	if(!setMoney(type, newMoney))
		return false;

	if(!updateMoneyToDB(type))
		return false;

	sendMainToMe();
	sendSysChat(ChannelType::screen_right_down, "获得 {}: {}", getMoneyName(type), money);
	LOG_TRACE("货币, 获得, text={}, type={}, money={}->{}, addMoney={}, role={}",
			  text, type, oldMoney, newMoney, money, *this);

	return true;
}

bool Role::reduceMoneyPrivate(MoneyType type, uint64_t money, const std::string& text)
{
	//扣绑定金币时, 特殊处理
	if(type == MoneyType::money_1)
	{
		return reduceGoldCoin(type, money, text);
	}

	const uint64_t oldMoney = getMoney(type);
	if((uint64_t)-1 == oldMoney)
		return false;

	if(money > oldMoney)
	{
		sendSysChat("{}不足", getMoneyName(type));
		return false;
	}

	const uint64_t newMoney = SAFE_SUB(oldMoney, money);
	if(!setMoney(type, newMoney))
		return false;

	if(!updateMoneyToDB(type))
		return false;

	sendMainToMe();
	sendSysChat(ChannelType::screen_right_down, "消耗 {}: {}", getMoneyName(type), money);
	LOG_TRACE("货币, 扣除, text={}, type={}, money={}->{}, needMoney={}, role={}",
			  text, type, oldMoney, newMoney, money, *this);
	
	return true;
}

//扣绑定金币时，先扣绑定金币，再扣非绑金币(特殊处理)
bool Role::reduceGoldCoin(MoneyType type, uint64_t money, const std::string& text)
{
	if(type != MoneyType::money_1)
		return false;

	const uint64_t oldGoldBind = getMoney(MoneyType::money_1);
	const uint64_t oldGold = getMoney(MoneyType::money_2);
	if((uint64_t)-1 == oldGoldBind || (uint64_t)-1 == oldGold)
		return false;

	if(money > (oldGoldBind + oldGold))
	{
		sendSysChat("{}不足", getMoneyName(type));
		return false;
	}

	if(oldGoldBind >= money)
	{
		const uint64_t newGoldBind = SAFE_SUB(oldGoldBind, money);
		if(!setMoney(type, newGoldBind))
			return false;
		
		if(!updateMoneyToDB(type))
			return false;

		sendMainToMe();
		sendSysChat(ChannelType::screen_right_down, "消耗 {}: {}", getMoneyName(type), money);
		LOG_TRACE("货币, 扣除, 绑定金币, text={}, type={}, money={}->{}, needMoney={}, role={}",
				  text, type, oldGoldBind, newGoldBind, money, *this);

		return true;
	}
	else
	{
		const uint64_t newGoldBind = 0;		//剩余绑定金币数量肯定为0
		const uint64_t needGold = SAFE_SUB(money, oldGoldBind);	//还需要扣除的非绑金币数量
		const uint64_t newGold =  SAFE_SUB(oldGold, needGold);	//剩余的非绑金币数量
	
		if(!setMoney(MoneyType::money_1, newGoldBind))
			return false;

		if(!setMoney(MoneyType::money_2, newGold))
			return false;

		if(!updateMoneyToDB(MoneyType::money_1))
			return false;
	
		if(!updateMoneyToDB(MoneyType::money_2))
			return false;

		sendMainToMe();
		sendSysChat(ChannelType::screen_right_down, "消耗 {}: {}", getMoneyName(MoneyType::money_1), oldGoldBind);
		sendSysChat(ChannelType::screen_right_down, "消耗 {}: {}", getMoneyName(MoneyType::money_2), needGold);
		
		LOG_TRACE("货币, 扣除, 绑定金币, 步骤一, text={}, type={}, money={}->{}, reduceMoney={}, needMoney={}, role={}",
				  text, type, oldGoldBind, newGoldBind, oldGoldBind, money, *this);
		
		LOG_TRACE("货币, 扣除, 绑定金币, 步骤二, text={}, type={}, money={}->{}, reduceMoney={}, needMoney={}, role={}",
				  text, type, oldGoldBind, newGoldBind, needGold, money, *this);

		return true;
	}

	return false;
}

bool Role::checkMoneyByObj(TplId tplId, uint16_t objNum)
{
	if(0 == objNum)
		return false;

    uint64_t price = StoreMgr::me().objPrice(tplId);
    if((uint64_t)-1 == price || 0 == price)
    {
        LOG_DEBUG("一键自动, 道具不足自动扣元宝, 商城找不到该道具tplId={}, price={}",
				  tplId, price);
        return false;
    }

    uint64_t needMoney = objNum * price;
	if(getMoney(MoneyType::money_4) < needMoney)
    {
        sendSysChat("元宝不足");
        return false;
    }
	
	return true;
}

bool Role::checkMoneyByObjs(const std::vector<std::pair<uint32_t, uint16_t> >& objVec)
{
	uint64_t needMoney = 0;
	for(auto iter = objVec.begin(); iter != objVec.end(); ++iter)
	{
		uint64_t price = StoreMgr::me().objPrice(iter->first);
		if((uint64_t)-1 == price || 0 == price)
		{
			LOG_DEBUG("一键自动, 道具不足自动扣元宝, 商城找不到该道具tplId={}, price={}",
					  iter->first, price);
			return false;
		}

		needMoney += iter->second * price;
	}

	if(0 == needMoney)
		return false;

	if(getMoney(MoneyType::money_4) < needMoney)
    {
        sendSysChat("元宝不足");
        return false;
    }
	
	return true;
}

bool Role::autoReduceObjMoney(TplId tplId, uint16_t objNum, const std::string& text)
{
    if(0 == objNum)
        return false;

    if(m_packageSet.getObjNum(tplId, PackageType::role) >= objNum)
    {
        m_packageSet.eraseObj(tplId, objNum, PackageType::role, "一键自动");
        return true;
    }

    uint64_t price = StoreMgr::me().objPrice(tplId);
    if((uint64_t)-1 == price || 0 == price)
    {
        LOG_DEBUG("一键自动, 道具不足自动扣元宝, 商城找不到该道具tplId={}, price={}",
				  tplId, price);
        return false;
    }

    uint64_t needMoney = objNum * price;
    if(getMoney(MoneyType::money_4) < needMoney)
    {
        sendSysChat("元宝不足");
        return false;
    }

    return reduceMoneyPrivate(MoneyType::money_4, needMoney, text);
}

bool Role::checkBanggong(uint64_t num)
{
    return (m_banggong >= num)?true:false;
}

bool Role::reduceBanggong(uint64_t num, const std::string& text)
{
    if(m_banggong < num)
        return false;
    LOG_TRACE("{},消耗帮贡{}", text, num);
    m_banggong = m_banggong - num;
    synBanggong();
    return true;
}

void Role::synBanggong()
{
    PrivateRaw::SynBanggong send;
    send.roleId = id();
    send.banggong = m_banggong;
	ProcessIdentity funcId("func", 1);
	World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(SynBanggong), (uint8_t*)&send, sizeof(PrivateRaw::SynBanggong));

}

uint64_t Role::banggong()
{
    return m_banggong;
}

void Role::setBanggongWithoutSysn(const uint64_t banggong)
{
    m_banggong = banggong;
}

void Role::setBanggong(const uint64_t banggong)
{
    m_banggong = banggong;
    synBanggong();
}

bool Role::updateMoneyToDB(MoneyType type)
{
	const uint64_t money = getMoney(type);
	if((uint64_t)-1 == money)
		return false;

	PrivateRaw::UpdateRoleMoney send;
	send.roleId = id();
	send.type = type;
	send.money = money;
	
	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateRoleMoney), &send, sizeof(send));
	LOG_TRACE("货币，send UpdateRoleMoney to {}, {}, roleId={}, type={}, money={}",
			  dbcachedId, ret ? "ok" : "falied", send.roleId, send.type, send.money);

	return ret;
}

void Role::addFaction(uint64_t factionExp, uint64_t factionResource, uint64_t banggong)
{
    PrivateRaw::ObjectDonate send;
    send.roleId = id();
    send.exp = factionExp;
    send.resource = factionResource;
    send.banggong = banggong;
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(ObjectDonate), &send, sizeof(send));

    setBanggongWithoutSysn(banggong);

}

void Role::sendSysChatPrivate(ChannelType type, const std::string& text)
{
	switch(type)
	{
	case ChannelType::system:
	case ChannelType::system_msgbox:
	case ChannelType::screen_right_down:
		sendSysMsgToMe(type, text);
		break;
	case ChannelType::screen_top:
	case ChannelType::screen_middle:
		sendSysNotifyToGlobal(type, text);
		break;
	default:
		break;
	}

	return;
}

void Role::sendSysMsgToMe(ChannelType type, const std::string& text)
{
	if(text.empty())
		return;

	const ArraySize textSize = text.size() + 1;
	const uint32_t bufSize = sizeof(const PublicRaw::SystemChannelMsgToClient) + textSize;
	auto buf = new uint8_t[bufSize];
	ON_EXIT_SCOPE_DO(delete[] buf);

	auto send = new(buf) PublicRaw::SystemChannelMsgToClient();
	send->type = type;
	send->textSize = textSize;
	memcpy(send->text, text.c_str(), textSize);

	sendToMe(RAWMSG_CODE_PUBLIC(SystemChannelMsgToClient), buf, bufSize);
	return;
}

void Role::sendSysNotifyToGlobal(ChannelType type, const std::string& text)
{
	if(text.empty())
		return;

	const ArraySize textSize = text.size() + 1;
	const uint32_t bufSize = sizeof(const PrivateRaw::SendSysNotifyToGlobal) + textSize;
	auto buf = new uint8_t[bufSize];
	ON_EXIT_SCOPE_DO(delete[] buf);

	auto send = new(buf) PrivateRaw::SendSysNotifyToGlobal();
	send->type = type;
	send->textSize = textSize;
	memcpy(send->text, text.c_str(), textSize);

	ProcessIdentity funcId("func", 1);
	const bool ret = World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(SendSysNotifyToGlobal), buf, bufSize);
	LOG_TRACE("频道, send SendSysNotifyToGlobal to {} {}, type={}, textSize={}, text={}",
			  funcId, ret ? "ok" : "falied",
			  send->type, send->textSize, send->text);

	return;
}

void Role::addBuyDayLimit(uint32_t objId, uint16_t num)
{
    m_roleSundry.m_buyStoreDayLimitObj[objId] += num;
    m_roleSundry.saveSundry();
}

uint16_t Role::getBuyDayLimit(uint32_t objId) const
{
    auto it = m_roleSundry.m_buyStoreDayLimitObj.find(objId);
    if(it == m_roleSundry.m_buyStoreDayLimitObj.end())
        return 0;

    return it->second;
}

void Role::autoAddTotalOnlineSec()
{
	m_totalOnlineSec += 1;
}

uint32_t Role::getTotalOnlineSec() const
{
	return m_totalOnlineSec;
}

void Role::autoAddCurObjId()
{
	m_curObjId += 1;
}

uint64_t Role::getCurObjId() const
{
	return m_curObjId;
}

void Role::setDefaultCallHero(Job job)
{
	m_defaultCallHero = job;
	updateDefaultCallHeroToDB();
}

Job Role::getDefaultCallHero() const
{
	return m_defaultCallHero;
}

void Role::updateDefaultCallHeroToDB()
{
	PrivateRaw::UpdateDefaultCallHero send;
	send.roleId = id();
	send.defaultCallHero = getDefaultCallHero();
	
	ProcessIdentity dbcachedId("dbcached", 1);
	World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateDefaultCallHero), &send, sizeof(send));
	
	return;
}

uint8_t Role::nameColor() const
{
    return m_nameColor;
}

void Role::checkAndClearGreyNameColor(const TimePoint& now)
{
    if(m_greynameTime != EPOCH
       && m_greynameTime + std::chrono::seconds {Massive::me().m_evilCfg.greynameTime} <= now)
        clearNameColor(name_color::grey);
}

void Role::updateGreynameTime()
{
    if(!issetNameColor(name_color::grey))
        return;
    m_greynameTime = Clock::now();
}

void Role::setNameColor(name_color color, bool sync)
{
    if(sync && issetNameColor(color))
        return;

    //LOG_TRACE("name_color, role={}, 设置color={}", *this, color);
    m_nameColor |= static_cast<uint8_t>(color);
    if(color == name_color::grey)
    {
        m_greynameTime = Clock::now();
        m_nameColor &= ~static_cast<uint8_t>(name_color::yellow);
    }

    if(sync)
    {
        syncNameColorTo9(nameColor());

        //同步英雄名称颜色
        Hero::Ptr hero = m_heroManager.getSummonHero();
        if(nullptr != hero)
            hero->syncNameColorTo9(nameColor());
    }
}

void Role::clearNameColor(name_color color)
{
    if(!issetNameColor(color))
        return;

    //LOG_TRACE("name_color, role={}, 清除color={}", *this, color);
    m_nameColor &= ~static_cast<uint8_t>(color);
    if(color == name_color::grey)
    {
        m_greynameTime = EPOCH;
        m_attackMode.judgeAndSetNameColor(true);
    }
    syncNameColorTo9(nameColor());

    //同步英雄
    Hero::Ptr hero = m_heroManager.getSummonHero();
    if(nullptr != hero)
        hero->syncNameColorTo9(nameColor());
}

bool Role::issetNameColor(name_color color)
{
    return m_nameColor & static_cast<uint8_t>(color);
}

uint32_t Role::offlineTime() const
{
    return m_offlineTime;
}

TimePoint Role::greynameTime() const
{
    return m_greynameTime;
}


/*
 * 获取role指针的全局函数,避免大量从pk到role的代码重写
 */
Role::Ptr getRole(PK::Ptr me)
{
    if(nullptr == me)
        return nullptr;
    if(me->sceneItemType() == SceneItemType::role)
        return std::static_pointer_cast<Role>(me);
    else if(me->sceneItemType() == SceneItemType::hero)
    {
        Hero::Ptr hero = std::static_pointer_cast<Hero>(me);
        return hero->getOwner();
    }
    else if(me->sceneItemType() == SceneItemType::pet)
    {
        Pet::Ptr pet = std::static_pointer_cast<Pet>(me);
        PK::Ptr owner = pet->getOwner();
        if(nullptr == owner)
            return nullptr;
        if(owner->sceneItemType() == SceneItemType::role)
            return std::static_pointer_cast<Role>(owner);
        else if(owner->sceneItemType() == SceneItemType::hero)
        {
            Hero::Ptr hero = std::static_pointer_cast<Hero>(me);
            return hero->getOwner();
        }
    }

    return nullptr;
}

bool Role::checkPutObj(const std::vector<ObjItem>& objVec)
{
	if(objVec.empty())
		return false;

	std::vector<ObjItem> realObjVec; 
	realObjVec.clear();
	for(auto iter = objVec.begin(); iter != objVec.end(); ++iter)
	{
		ObjBasicData data;
		if(!ObjectConfig::me().getObjBasicData(iter->tplId, &data))
			return false;

		if(data.childType == static_cast<uint16_t>(ObjChildType::role_exp_usage)
		   || data.childType == static_cast<uint16_t>(ObjChildType::hero_exp_usage)
		   || data.childType == static_cast<uint16_t>(ObjChildType::money_usage))
			continue;

		if(iter->num > data.maxStackNum)
			return false;

		realObjVec.push_back(*iter);
	}
	
	if(realObjVec.size() <= m_packageSet.getEmptyCellNum(PackageType::role))
		return true;
	
	return false;
}

bool Role::checkPutObj(TplId tplId, uint32_t num, Bind bind, const PackageType packageType)
{
    ObjBasicData data;
    if(!ObjectConfig::me().getObjBasicData(tplId, &data))
        return false;
  
	if(data.childType == static_cast<uint16_t>(ObjChildType::role_exp_usage)
       || data.childType == static_cast<uint16_t>(ObjChildType::hero_exp_usage)
       || data.childType == static_cast<uint16_t>(ObjChildType::money_usage))
        return true;
	
	return m_packageSet.checkPutObj(tplId, num, bind, packageType);
}

std::vector<ObjItem> Role::putObj(const std::vector<ObjItem>& objVec)
{
	if(!checkPutObj(objVec))
		return objVec;

	std::vector<ObjItem> putFailedVec;
	putFailedVec.clear();
	for(auto iter = objVec.begin(); iter != objVec.end(); ++iter)
	{
		uint32_t putNum = putObj(iter->tplId, iter->num, iter->bind, PackageType::role, iter->skillId, iter->strongLevel, iter->luckyLevel);
		if(putNum != iter->num)
		{
			ObjItem temp;
			temp.tplId = iter->tplId;
			temp.num = SAFE_SUB(iter->num, putNum);
			temp.bind = iter->bind;
			temp.skillId = iter->skillId;
			temp.strongLevel = iter->strongLevel;
			temp.luckyLevel = iter->luckyLevel;
			putFailedVec.push_back(temp);
		}
	}

	return putFailedVec;
}

uint32_t Role::putObj(TplId tplId, uint32_t num, Bind bind, const PackageType packageType, const uint32_t skillId, const uint8_t strongLevel, const uint8_t luckyLevel)
{
    ObjBasicData data;
    if(!ObjectConfig::me().getObjBasicData(tplId, &data))
        return 0;
    
	if(data.childType == static_cast<uint16_t>(ObjChildType::role_exp_usage))
    {
        addExp(num);
        return num;
    }
    else if(data.childType == static_cast<uint16_t>(ObjChildType::hero_exp_usage))
    {
        auto hero = m_heroManager.getDefaultHero();
        if(nullptr == hero)
            return 0;

        hero->addExp(num);
        return num;
    }
    else if(data.childType == static_cast<uint16_t>(ObjChildType::money_usage))
    {
        addMoney(static_cast<MoneyType>(data.moneyType), num, "");
        return num;
    }
    else
	{
	    return m_packageSet.putObj(tplId, num, bind, packageType, skillId, strongLevel, luckyLevel);
	}
}

bool Role::eraseObj(TplId tplId, uint16_t num, PackageType packageType, const std::string& text)
{
	return m_packageSet.eraseObj(tplId, num, packageType, text);
} 

bool Role::eraseObj(TplId tplId, uint16_t num, Bind bind, PackageType packageType, const std::string& text)
{
	return m_packageSet.eraseObj(tplId, num, packageType, text);
}

Object::Ptr Role:: eraseObjByCell(uint16_t cell, PackageType packageType, const std::string& text)
{
	return m_packageSet.eraseObjByCell(cell, packageType, text);
}

Object::Ptr Role::eraseObjByCell(uint16_t cell, uint16_t num, PackageType packageType, const std::string& text)
{
	return m_packageSet.eraseObjByCell(cell, num, packageType, text);
}

uint16_t Role::getObjNum(TplId tplId, const PackageType packageType) const
{
	return m_packageSet.getObjNum(tplId, packageType);
}

uint16_t Role::getObjNumByCell(uint16_t cell, const PackageType packageType) const
{
	return m_packageSet.getObjNumByCell(cell, packageType);
}

Object::Ptr Role::getObjByCell(uint16_t cell, const PackageType packageType) const
{
	return m_packageSet.getObjByCell(cell, packageType);
}

Bind Role::getBindByCell(uint16_t cell, const PackageType packageType) const
{
	return m_packageSet.getBindByCell(cell, packageType);
}

void Role::setSummonHeroFlag(bool flag)
{
	m_summonHeroFlag = flag;
}

bool Role::getSummonHeroFlag() const
{
	return m_summonHeroFlag;
}

PackageType Role::getPackageTypeByHeroJob(Job job) const
{
	if(job == Job::warrior)
		return PackageType::equipOfWarrior;
	else if(job == Job::magician)
		return PackageType::equipOfMagician;
	else if(job == Job::taoist)
		return PackageType::equipOfTaoist;

	return PackageType::none;
}

Job Role::getHeroJobByPackageType(PackageType packageType) const
{
	if(packageType == PackageType::equipOfWarrior || packageType == PackageType::stoneOfWarrior)
	{
		return Job::warrior;
	}
	else if(packageType == PackageType::equipOfMagician || packageType == PackageType::stoneOfMagician)
	{
		return Job::magician;
	}
	else if(packageType == PackageType::equipOfTaoist || packageType == PackageType::equipOfTaoist)
	{
		return Job::taoist;
	}

	return Job::none;
}

Hero::Ptr Role::getHeroByJob(Job job) const
{
	return m_heroManager.getHeroByJob(job);
}

void Role::addTitle(uint32_t typeId)
{
	m_title.addTitle(typeId);
}

uint32_t Role::getUsedTitleIdByType(TitleType type) const
{
	return m_title.getUsedTitleIdByType(type);
}

void Role::setBufferData(const std::vector<uint16_t>& bufferVec)
{
    m_bufferVec.clear();
	m_bufferVec = bufferVec;
	
	updateBufferDataToDB();
}

void Role::loadBufferData(const std::string& bufferStr)
{
    Deserialize<std::string> oss(&bufferStr);
    oss >> m_bufferVec;
}

void Role::sendBufferDataToMe()
{
	std::vector<uint8_t> buf;
	buf.reserve(1024);
	buf.resize(sizeof(PublicRaw::RetRoleBufData));

	auto* msg  = reinterpret_cast<PublicRaw::RetRoleBufData*>(buf.data());
	msg->size = 0;

	for(ArraySize i = 0; i < m_bufferVec.size(); i++)
	{
		buf.resize(buf.size() + sizeof(msg->buf[0]));
		auto* msg  = reinterpret_cast<PublicRaw::RetRoleBufData*>(buf.data());

		msg->buf[i] = m_bufferVec[i];
		++msg->size;
	}

	sendToMe(RAWMSG_CODE_PUBLIC(RetRoleBufData), buf.data(), buf.size());
	return;
}

void Role::updateBufferDataToDB()
{
	std::vector<uint8_t> buf;
	buf.reserve(1024);

    Serialize<std::string> iss;
    iss.reset();
    iss << m_bufferVec;

	buf.resize(sizeof(PrivateRaw::UpdateRoleBufferData) + iss.tellp());
	auto* msg  = reinterpret_cast<PrivateRaw::UpdateRoleBufferData*>(buf.data());
	msg->roleId = id();
    msg->size = iss.tellp();
    std::memcpy(msg->buf, iss.buffer()->data(), iss.tellp());

	ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateRoleBufferData), buf.data(), buf.size());
	
	return;
}

uint32_t Role::getMinStrongLevel() const
{
	EquipPackage::Ptr packagePtr = std::static_pointer_cast<EquipPackage>(m_packageSet.getPackageByPackageType(PackageType::equipOfRole));
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->getMinStrongLevel();
}

uint32_t Role::getStoneTotalLevel() const
{
	StonePackage::Ptr packagePtr = std::static_pointer_cast<StonePackage>(m_packageSet.getPackageByPackageType(PackageType::stoneOfRole));
	if(packagePtr == nullptr)
		return 0;

	return packagePtr->getStoneTotalLevel();
}

//任务调度函数
void Role::dispatchTask(TaskContent content, TaskParam param)
{
    m_roleTask.dispatch(content, param);
}

bool Role::inArea(AreaType type)
{
	Scene::Ptr s = scene();
	if(s == nullptr)
		return false;

	return s->isArea(pos(), type);
}

uint8_t Role::vipLevel() const
{
	return m_vipLevel;
}

uint32_t Role::mohun() const
{
    return m_mohun;
}

bool Role::checkEnerge(uint16_t energe) const
{
    return m_roleSundry.m_energe + m_dragonHeart.reduceEnergeCost() >= energe;
}

bool Role::addEnerge(uint16_t energe)
{
    if(m_roleSundry.m_energe >= energeLimit())
        return false;
    m_roleSundry.m_energe += energe;
    if(m_roleSundry.m_energe >= energeLimit())
        m_roleSundry.m_energe = energeLimit();

    retEnerge();
    m_roleSundry.saveSundry();
    return true;
}

void Role::subEnerge(uint16_t energe)
{
    uint16_t cost = SAFE_SUB(energe, m_dragonHeart.reduceEnergeCost());
    m_roleSundry.m_energe = SAFE_SUB(m_roleSundry.m_energe, cost);
    retEnerge();
    m_roleSundry.saveSundry();
}

uint16_t Role::energeLimit() const
{
    return m_dragonHeart.addEnergeLimit() + Massive::me().m_jointSkillCfg.energeLimit;
}

void Role::retEnerge() const
{
    PublicRaw::RetEnerge send;
    send.energe = m_roleSundry.m_energe;
    sendToMe(RAWMSG_CODE_PUBLIC(RetEnerge), &send, sizeof(send));
}

void Role::requestAddEnerge(uint8_t autoyb)
{
    if(m_roleSundry.m_energe >= energeLimit())
    {
        sendSysChat("当前龙心能量已达上限, 不需要充能");
        return;
    }

    auto objIter = ObjectConfig::me().objectCfg.m_objBasicDataMap.find(Massive::me().m_jointSkillCfg.costObj);
    if(objIter == ObjectConfig::me().objectCfg.m_objBasicDataMap.end())
    {
        LOG_DEBUG("龙心, 充填能量, 道具配置表中找不到该道具, objId={}", Massive::me().m_jointSkillCfg.costObj);
        return;
    }

    if(autoyb)
    {
        if(!autoReduceObjMoney(Massive::me().m_jointSkillCfg.costObj, 1, "龙心充填能量"))
        {
            PublicRaw::AutoAddEnergeFail ret;
            sendToMe(RAWMSG_CODE_PUBLIC(AutoAddEnergeFail), &ret, sizeof(ret));
            return;
        }
    }
    else
    {
        if(getObjNum(Massive::me().m_jointSkillCfg.costObj, PackageType::role) < 1)
        {//道具不够
            LOG_DEBUG("龙心, 充填能量, 材料不够");
            return;
        }
        eraseObj(Massive::me().m_jointSkillCfg.costObj, 1, PackageType::role, "龙心充填能量");
    }

    addEnerge(energeLimit() * objIter->second.energe / 1000);
}

bool Role::checkAnger() const
{
    return m_anger >= Massive::me().m_jointSkillCfg.costAnger;
}

void Role::addAnger(uint16_t num)
{
    if(isJointReadyState())
        return;
    if(m_anger >= Massive::me().m_jointSkillCfg.angerLimit)
        return;
    m_anger += num;
    if(m_anger >= Massive::me().m_jointSkillCfg.angerLimit)
        m_anger = Massive::me().m_jointSkillCfg.angerLimit;
    refreshAnger();
}

void Role::subAnger()
{
    m_anger = SAFE_SUB(m_anger, Massive::me().m_jointSkillCfg.costAnger);
    refreshAnger();
}

void Role::refreshAnger() const
{
    PublicRaw::RefreshAnger ret;
    ret.angerLimit = Massive::me().m_jointSkillCfg.angerLimit;
    ret.anger = m_anger;
    sendToMe(RAWMSG_CODE_PUBLIC(RefreshAnger), &ret, sizeof(ret));
}

uint16_t Role::jointSkillReadyTime() const
{
    return m_dragonHeart.extendReadyTime() + Massive::me().m_jointSkillCfg.readyTime;
}

void Role::intoCopyMap(MapId mapId)
{
}

void Role::exitCopyMap()
{
    //现在统一退回之前的静态地图(可根据副本类型特殊处理)
    RolesAndScenes::me().gotoOtherScene(id(), preSceneId(), prePos());
}

SceneId Role::preSceneId() const
{
    return m_preSceneId;
}

Coord2D Role::prePos() const
{
    return m_prePos;
}

/*
 * 传送门接口(注: 传送门不适用于多人副本跨地图传送)
 */
void Role::transferByTriggerDoor(uint32_t triggerTplId)
{
    auto s = scene();
    if(nullptr == s)
        return;

    TriggerTpl::Ptr triggerTpl = TriggerCfg::me().getById(triggerTplId);
    if(nullptr == triggerTpl)
    {
        LOG_DEBUG("机关, 传送门,找不到该机关配置, id:{}", triggerTplId);
        return;
    }

    bool transFailed = true;
    auto noticeClientTransFailed = [&transFailed, this] () -> void
    {
        if(transFailed)
        {
            PublicRaw::GotoOtherSceneByTransmissionFailed send;
            sendToMe(RAWMSG_CODE_PUBLIC(GotoOtherSceneByTransmissionFailed), &send, sizeof(send));
        }
    };
    ON_EXIT_SCOPE(noticeClientTransFailed);

    MapId newMapId = triggerTpl->mapId;
    Coord2D newPos(triggerTpl->posx, triggerTpl->posy);

    MapTpl::Ptr newMapTpl = MapBase::me().getMapTpl(newMapId);
    if(nullptr != newMapTpl && s->mapId() != newMapId)
    {
        //跨地图跳转, 区分newMapId为静态或者副本
        switch(newMapTpl->type)
        {
        case CopyMap::none:
            {
                //静态地图
                if(!RolesAndScenes::me().gotoOtherScene(id(), newMapId, newPos))
                    return;
            }
            break;
        case CopyMap::shabake:
            {
                if(!ShaBaKe::me().canTransfer(factionId(), triggerTplId))
                    return;
                if(!RolesAndScenes::me().gotoOtherScene(id(), newMapId, newPos))
                    return;
            }
            break;
        default:
            {
                //单人副本, 创建新的副本地图
                //createCopy();
            }
            break;
        }

        transFailed = false;
        return;
    }

    //同地图跳转
    transFailed = false;
    auto ValidPosExec = [&](Coord2D pos)
    {
        if(!s->enterable(pos, SceneItemType::role))
            return false;
        newPos = pos;
        return true;
    };
    s->tryExecSpiral(newPos, 10, ValidPosExec);
    changePos(newPos, dir(), MoveType::blink);
}

/*
 * 占有世界boss宝箱
 */
void Role::pickupWorldBossBox()
{
    if(!WorldBoss::me().inTime())
        return;
    if(isDead())
        return;
    m_holdBoxTime = Clock::now();
    m_pkstate.setStatus(visual_status::holdbox);
    syncScreenDataTo9();
}

bool Role::clearWorldBossBoxStatus()
{
    if(!m_pkstate.clearStatus(visual_status::holdbox))
        return false;
    m_holdBoxTime = EPOCH;
    //syncScreenDataTo9();
    return true;
}

void Role::judgeBoxBelong()
{
    if(!WorldBoss::me().inTime())
        return;
    if(EPOCH == m_holdBoxTime)
        return;

    if(m_holdBoxTime + std::chrono::seconds {WorldBoss::me().boxNeedHoldSeconds()} <= Clock::now() )
    {
        WorldBoss::me().boxBelongRole(std::static_pointer_cast<Role>(shared_from_this()));
        m_pkstate.clearStatus(visual_status::holdbox);
        m_holdBoxTime = EPOCH;
    }
}

uint16_t Role::holdBoxLeftTime() const
{
    if(EPOCH == m_holdBoxTime)
        return 0;

    using namespace std::chrono;
    uint16_t holdtime = duration_cast<seconds>(Clock::now() - m_holdBoxTime).count();
    return SAFE_SUB(WorldBoss::me().boxNeedHoldSeconds(), holdtime);
}

void Role::setStall()
{
    if(m_stall >= 1)
        return;
    m_stall = 1;
    setDir(Direction::down);
    syncScreenDataTo9();
    LOG_TRACE("摆摊, role:{} 成功", *this);
}

bool Role::clearStall()
{
    if(m_stall < 1)
        return false;
    m_stall = 0;
    syncScreenDataTo9();
    LOG_TRACE("收摊, role:{}", *this);
    return true;
}

bool Role::isStall() const
{
    return m_stall >= 1;
}

void Role::startCollect(PKId npcId)
{
    if(isDead())
        return;
    Npc::Ptr npc = NpcManager::me().getById(npcId);
    if(nullptr == npc || npc->type() != NpcType::collect || npc->isDead())
        return;

    //已经在采集了, 不能采集下一个
    if(0 != m_collectNpcId && m_collectTp + std::chrono::seconds{10} > Clock::now())
        return;

    if(npc->collectRoleId() && npc->collectRoleId() != id())
    {
        sendSysChat("该资源已被采集");
        return;
    }

    npc->setCollectRoleId(id());
    m_collectNpcId = npcId;
    m_collectTp = Clock::now();

    PublicRaw::RetCanCollect ret;
    ret.sec = npc->belongtime();
    sendToMe(RAWMSG_CODE_PUBLIC(RetCanCollect), &ret, sizeof(ret));
}

void Role::interruptCollect()
{
    Npc::Ptr npc = NpcManager::me().getById(m_collectNpcId);
    if(nullptr == npc)
        return;
    //LOG_DEBUG("采集, 触发中断, npc清空标识");
    npc->setCollectRoleId(0);
    m_collectNpcId = 0;
    m_collectTp = EPOCH;

    PublicRaw::InterruptCollect ret;
    sendToMe(RAWMSG_CODE_PUBLIC(InterruptCollect), &ret, sizeof(ret));
}

void Role::finishCollect()
{
    if(isDead())
        return;
    Npc::Ptr npc = NpcManager::me().getById(m_collectNpcId);
    if(nullptr == npc)
        return;
    if(m_collectTp + std::chrono::seconds{npc->belongtime()} > Clock::now())
    {
        LOG_DEBUG("采集, 还在采集中, role:{}", name());
        return;
    }
    //LOG_DEBUG("采集, 完成, npcid:{}, role:{}", m_collectNpcId, name());
    npc->setDead();

    TaskParam param;
    param.element.npcId = m_collectNpcId;
    dispatchTask(TaskContent::collection, param);
    m_collectNpcId = 0;
    m_collectTp = EPOCH;
}

void Role::underAttack(PK::Ptr atk)
{
    auto hero = m_heroManager.getSummonHero();
    if(hero == nullptr)
        return;
    hero->underAttack(atk);
}

}
