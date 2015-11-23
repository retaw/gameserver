#include "shabake.h"
#include "role_manager.h"
#include "scene_manager.h"
#include "world.h"
#include "role_counter.h"
#include "roles_and_scenes.h"
#include "reward_manager.h"
#include "npc_manager.h"
#include "action_manager.h"
#include "relive_config.h"
#include "mail_manager.h"

#include "water/componet/string_kit.h"
#include "water/componet/exception.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/shabake.h"
#include "protocol/rawmsg/public/shabake.codedef.public.h"
#include "protocol/rawmsg/private/shabake.h"
#include "protocol/rawmsg/private/shabake.codedef.private.h"

namespace world{

ShaBaKe::ShaBaKe()
:m_inTime(false)
,m_existKingStatue(false)
,m_kingStatue("")
,m_tempWinFaction(0)
{
}

ShaBaKe& ShaBaKe::me()
{
    static ShaBaKe me;
    return me;
}

void ShaBaKe::loadConfig(const std::string& cfgDir)
{
    const std::string file = cfgDir + "/shabake.xml";
    XmlParseDoc doc(file);
    XmlParseNode root = doc.getRoot();
    if(!root)
    {
        EXCEPTION(ExceptionBase, file + " parse root node fail");
        return;
    }

    XmlParseNode otherNode = root.getChild("Other");
    if(otherNode)
    {
        m_sbkCfg.ownmin = otherNode.getAttr<uint16_t>("ownmin");
        m_sbkCfg.level = otherNode.getAttr<uint32_t>("level");
        m_sbkCfg.statueId = otherNode.getAttr<uint32_t>("statueid");
        m_sbkCfg.statuePos = fromString<Coord2D>(otherNode.getAttr<std::string>("statuepos"));
        LOG_DEBUG("沙巴克, 加载 ownmin:{}, level:{}, statueId:{}, statuepos:{}", m_sbkCfg.ownmin, m_sbkCfg.level, m_sbkCfg.statueId, m_sbkCfg.statuePos);
    }

    XmlParseNode mapNode = root.getChild("Map");
    if(mapNode)
    {
        m_sbkCfg.palaceScene = mapNode.getAttr<SceneId>("palace");
        m_sbkCfg.outcityScene = mapNode.getAttr<SceneId>("outcity");

        m_sbkCfg.offensivePos = fromString<Coord2D>(mapNode.getAttr<std::string>("offensivePos"));
        m_sbkCfg.defensivePos = fromString<Coord2D>(mapNode.getAttr<std::string>("defensivePos"));

        m_sbkCfg.defensiveTriggerDoor = mapNode.getAttr<uint32_t>("defensiveTriggerDoor");
        m_sbkCfg.defensiveTriggerDoorPos = fromString<Coord2D>(mapNode.getAttr<std::string>("defensiveTriggerDoorPos"));

        m_sbkCfg.outcityWallDoor = mapNode.getAttr<uint32_t>("outcityWallDoor");
        m_sbkCfg.outcityWallDoorPos = fromString<Coord2D>(mapNode.getAttr<std::string>("outcityWallDoorPos"));
        std::vector<std::string> posvec = splitString(mapNode.getAttr<std::string>("blockDoorPos"), ";");
        for(const auto& pos : posvec)
            m_sbkCfg.blockDoorPos.push_back(fromString<Coord2D>(pos));

        for(XmlParseNode itemNode = mapNode.getChild("Item"); itemNode; ++itemNode)
        {
            WallNpc npc;
            npc.npcid = itemNode.getAttr<uint32_t>("wallnpc");
            npc.npcpos = fromString<Coord2D>(itemNode.getAttr<std::string>("pos"));
            npc.doorid = itemNode.getAttr<uint16_t>("doorid");
            npc.doorpos = fromString<Coord2D>(itemNode.getAttr<std::string>("doorpos"));
            m_sbkCfg.wallNpcInfo.push_back(npc);
        }
    }

    XmlParseNode awardNode = root.getChild("Award");
    if(awardNode)
    {
        XmlParseNode winNode = awardNode.getChild("Win");
        if(winNode)
        {
            for(XmlParseNode itemNode = winNode.getChild("Item"); itemNode; ++itemNode)
            {
                uint8_t type = itemNode.getAttr<uint8_t>("type");
                if(type == 1)
                    m_sbkCfg.firstWinReward = itemNode.getAttr<uint32_t>("reward");
                else
                    m_sbkCfg.winReward = itemNode.getAttr<uint32_t>("reward");
            }
        }

        XmlParseNode kingNode = awardNode.getChild("King");
        if(kingNode)
        {
            for(XmlParseNode itemNode = kingNode.getChild("Item"); itemNode; ++itemNode)
            {
                uint8_t type = itemNode.getAttr<uint8_t>("type");
                if(type == 1)
                    m_sbkCfg.firstKingReward = itemNode.getAttr<uint32_t>("reward");
                else
                    m_sbkCfg.kingReward = itemNode.getAttr<uint32_t>("reward");
            }
        }

        XmlParseNode parttakeNode = awardNode.getChild("Parttake");
        if(parttakeNode)
        {
            m_sbkCfg.parttakeReward = parttakeNode.getAttr<uint32_t>("reward");
        }

        XmlParseNode dailyNode = awardNode.getChild("Daily");
        if(dailyNode)
        {
            for(XmlParseNode itemNode = dailyNode.getChild("Item"); itemNode; ++itemNode)
            {
                ShabakePosition position = static_cast<ShabakePosition>(itemNode.getAttr<uint8_t>("position"));
                m_sbkCfg.dailyReward[position] = itemNode.getAttr<uint32_t>("reward");
            }
        }
    }
}

void ShaBaKe::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&ShaBaKe::timeExec, this, StdInterval::sec_1, _1));
    World::me().regTimer(std::chrono::seconds(5), 
                         std::bind(&ShaBaKe::timeExec, this, StdInterval::sec_5, _1));
}

void ShaBaKe::timeExec(StdInterval interval, const TimePoint& now)
{
    switch(interval)
    {
    case StdInterval::sec_1:
        {
            if(!m_inTime)
                return;
            if(m_tempWinTime != EPOCH && m_tempWinTime + std::chrono::seconds{m_sbkCfg.ownmin * 60} <= Clock::now())
                winShabake();
        }
        break;
    case StdInterval::sec_5:
        {
            auto s = SceneManager::me().getById(ReliveConfig::me().cityId());
            if(nullptr != s && !m_existKingStatue)
            {
                PrivateRaw::ReqShabakeKing req;
                ProcessIdentity funcId("func", 1);
                World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(ReqShabakeKing), &req, sizeof(req));
            }
        }
        break;
    default:
        break;
    }
}

void ShaBaKe::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(StartShabake, std::bind(&ShaBaKe::servermsg_StartShabake, this, _1, _2));
    REG_RAWMSG_PRIVATE(SyncCurTempWinFaction, std::bind(&ShaBaKe::servermsg_SyncCurTempWinFaction, this, _1, _2));
    REG_RAWMSG_PRIVATE(EndShabake, std::bind(&ShaBaKe::servermsg_EndShabake, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(NotifyWorldGiveRoleAward, std::bind(&ShaBaKe::servermsg_NotifyWorldGiveRoleAward, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RetShabakeKing, std::bind(&ShaBaKe::servermsg_RetShabakeKing, this, _1, _2));
    REG_RAWMSG_PRIVATE(FuncReqDailyAwardInfo, std::bind(&ShaBaKe::servermsg_FuncReqDailyAwardInfo, this, _1, _2));
}

void ShaBaKe::servermsg_StartShabake(const uint8_t* msgData, uint32_t size)
{
    m_inTime = true;
    ActionManager::me().setActionState(ActionType::shabake, ActionState::begin);
    auto rev = reinterpret_cast<const PrivateRaw::StartShabake*>(msgData);
    m_starttime = Clock::now();
    m_endtime = m_starttime + std::chrono::seconds{rev->sec};
    m_tempWinFaction = rev->lastWinFaction;
    m_palaceFactionRole.clear();
    m_wallNpcIds.clear();

    Scene::Ptr palace_s = SceneManager::me().getById(m_sbkCfg.palaceScene);
    Scene::Ptr outcity_s = SceneManager::me().getById(m_sbkCfg.outcityScene);
    if(nullptr == palace_s || nullptr == outcity_s)
    {
        LOG_DEBUG("沙巴克, 活动开启,没有该地图, palaceScene:{}, outcityScene:{}", m_sbkCfg.palaceScene, m_sbkCfg.outcityScene);
        return;
    }

    m_tempWinFactionName = rev->winFactionName;
    LOG_DEBUG("沙巴克, 活动开启, 占领帮派:{}, 持续时长:{}", m_tempWinFactionName, rev->sec);
    if(m_tempWinFaction > 0)
        m_tempWinTime = Clock::now();
    //活动开始前的各项准备
    //城墙
    for(const auto& it : m_sbkCfg.wallNpcInfo)
    {
        Npc::Ptr npc = outcity_s->summonNpc(it.npcid, it.npcpos);
        if(nullptr == npc)
        {
            LOG_DEBUG("沙巴克, 召唤城墙npc失败 npid:{}, pos:{}", it.npcid, it.npcpos);
            continue;
        }
        npc->setOwnerId(m_tempWinFaction);
        m_wallNpcIds.insert(npc->id());
    }

    //城门
    Npc::Ptr npcdoor = outcity_s->summonNpc(m_sbkCfg.outcityWallDoor, m_sbkCfg.outcityWallDoorPos);
    if(nullptr != npcdoor)
    {
        npcdoor->setOwnerId(m_tempWinFaction);
        for(const auto& pos : m_sbkCfg.blockDoorPos)
        {//召唤动态阻挡
            Trigger::Ptr blocktrigger = outcity_s->summonTrigger(DYNAMIC_BLOCK, pos, 0);
            if(nullptr == blocktrigger)
            {
                LOG_DEBUG("沙巴克, 动态阻挡召唤失败");
                continue;
            }
            m_cacheBlockDoor.push_back(blocktrigger->id());
        }
    }
    else
        LOG_DEBUG("沙巴克, 召唤外城城门npc失败, id:{}, pos:{}", m_sbkCfg.outcityWallDoor, m_sbkCfg.outcityWallDoorPos);

    //防守方进出皇宫传送点
    if(nullptr == outcity_s->summonTrigger(m_sbkCfg.defensiveTriggerDoor, m_sbkCfg.defensiveTriggerDoorPos, 0))
    {
        LOG_DEBUG("沙巴克, 召唤防守方自由进出皇宫传送点失败, doorid:{}, pos:{}", m_sbkCfg.defensiveTriggerDoor, m_sbkCfg.defensiveTriggerDoorPos);
    }
}

void ShaBaKe::servermsg_EndShabake(const uint8_t* msgData, uint32_t size, uint64_t pid)
{
    if(!m_inTime)
        return;
    m_inTime = false;
    ActionManager::me().setActionState(ActionType::shabake, ActionState::end);
    auto rev = reinterpret_cast<const PrivateRaw::EndShabake*>(msgData);
    m_kingStatue = rev->king;
    LOG_DEBUG("沙巴克, 活动结束, 城主:{}", m_kingStatue);

    Scene::Ptr palace_s = SceneManager::me().getById(m_sbkCfg.palaceScene);
    Scene::Ptr outcity_s = SceneManager::me().getById(m_sbkCfg.outcityScene);
    if(nullptr == palace_s || nullptr == outcity_s)
        return;

    ProcessIdentity funcId(pid);
    {//同步当前所有在场玩家到func
        std::vector<uint8_t> buf;
        buf.reserve(256);
        buf.resize(sizeof(PrivateRaw::SyncShabakeSceneRoleToFunc));

        auto GetRoleIdExec = [&] (Role::Ptr role)
        {
            if(nullptr == role)
                return;
            buf.resize(buf.size() + sizeof(RoleId));
            auto msg = reinterpret_cast<PrivateRaw::SyncShabakeSceneRoleToFunc*>(buf.data());
            msg->roleId[msg->size++] = role->id();
        };

        palace_s->execRole(GetRoleIdExec);
        outcity_s->execRole(GetRoleIdExec);

        World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(SyncShabakeSceneRoleToFunc), buf.data(), buf.size());
    }

    outcity_s->removeSceneItem();
}

void ShaBaKe::servermsg_SyncCurTempWinFaction(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::SyncCurTempWinFaction*>(msgData);
    m_tempWinFaction = rev->factionId;

    Scene::Ptr palace_s = SceneManager::me().getById(m_sbkCfg.palaceScene);
    Scene::Ptr outcity_s = SceneManager::me().getById(m_sbkCfg.outcityScene);
    if(nullptr == palace_s || nullptr == outcity_s)
        return;

    m_tempWinFactionName = rev->factionName;
    m_tempWinTime = Clock::now();
    auto refreshProgressExec = [&] (Role::Ptr role)
    {
        if(nullptr == role)
            return;
        refreshShabakeProgress(role);
    };

    palace_s->execRole(refreshProgressExec);
    outcity_s->execRole(refreshProgressExec);

    for(const auto& it : m_wallNpcIds)
    {
        auto npc = NpcManager::me().getById(it);
        if(nullptr == npc || npc->isDead())
            continue;
        npc->setOwnerId(rev->factionId);
    }
    LOG_DEBUG("沙巴克, 当前占领帮派:{}", m_tempWinFactionName);
}

void ShaBaKe::servermsg_NotifyWorldGiveRoleAward(const uint8_t* msgData, uint32_t size, uint64_t pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::NotifyWorldGiveRoleAward*>(msgData);
    ProcessIdentity funcId(pid);
    PrivateRaw::WorldRetGiveRoleAward ret;
    ret.roleId = rev->roleId;
    ret.awardtype = rev->awardtype;
    ret.success = false;

    auto role = RoleManager::me().getById(rev->roleId);
    if(nullptr == role)
    {
        LOG_DEBUG("沙巴克, 发奖励时， 玩家不在线, role:{}", rev->roleId);
        World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(WorldRetGiveRoleAward), &ret, sizeof(ret));
        return;
    }

    uint32_t rewardId = 0;
    switch(rev->awardtype)
    {
    case ShabakeAward::king:
        {
            rewardId = m_sbkCfg.kingReward;
            if(rev->first)//首次城主奖励
                rewardId = m_sbkCfg.firstKingReward;
        }
        break;
    case ShabakeAward::win:
        {
            rewardId = m_sbkCfg.winReward;
            if(rev->first)
                rewardId = m_sbkCfg.firstWinReward;
        }
        break;
    case ShabakeAward::parttake:
        {
            rewardId = m_sbkCfg.parttakeReward;
        }
        break;
    case ShabakeAward::daily:
        {
            if(0 != role->m_roleCounter.get(CounterType::shabakeDailyAward))
            {
                //role->sendSysChat("今天已经领取过");
                World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(WorldRetGiveRoleAward), &ret, sizeof(ret));
                return;
            }
            auto it = m_sbkCfg.dailyReward.find(rev->position);
            if(it == m_sbkCfg.dailyReward.end())
            {
                LOG_DEBUG("沙巴克, 日常奖励配置为空, position:{}", rev->position);
                World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(WorldRetGiveRoleAward), &ret, sizeof(ret));
                return;
            }
            rewardId = it->second;
        }
        break;
    default:
        return;
    }

	std::vector<ObjItem> objVec;
	if(!RewardManager::me().getFixReward(rewardId, 1, role->level(), role->job(), objVec))
	{
		LOG_ERROR("沙巴克, 获取固定奖励失败, 发放奖励失败, name={}, roleId={}, level={}, job={}, rewardId={}",
				  role->name(), role->id(), role->level(), role->job(), rewardId);
		return;
	}

	if(!objVec.empty() && !role->checkPutObj(objVec))
	{
		std::string text = "由于背包空间不足, 通过邮件发放沙巴克奖励, 请注意查收";
		MailManager::me().send(role->id(), "沙巴克奖励", text, objVec);
	}
	else
	{
		role->putObj(objVec);
	}

    if(rev->awardtype == ShabakeAward::daily)
    {
        role->m_roleCounter.add(CounterType::shabakeDailyAward);
        refreshShabakeDailyAwardInfo(role, rev->position);
    }

    ret.success = true;
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(WorldRetGiveRoleAward), &ret, sizeof(ret));
}

void ShaBaKe::servermsg_RetShabakeKing(const uint8_t* msgData, uint32_t size)
{
    auto s = SceneManager::me().getById(ReliveConfig::me().cityId());
    if(nullptr == s)
        return;
    auto rev = reinterpret_cast<const PrivateRaw::RetShabakeKing*>(msgData);
    m_kingStatue = rev->king;
    if(!m_existKingStatue)
    {
        m_existKingStatue = true;
        Npc::Ptr npc = s->summonNpc(m_sbkCfg.statueId, m_sbkCfg.statuePos, 5);
        LOG_DEBUG("沙巴克, 召唤城主雕塑{}, id:{}, king:{}, pos:{}", nullptr==npc ? "失败":"成功", m_sbkCfg.statueId, m_kingStatue, m_sbkCfg.statuePos);
    }
}

void ShaBaKe::servermsg_FuncReqDailyAwardInfo(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::FuncReqDailyAwardInfo*>(msgData);
    auto role = RoleManager::me().getById(rev->roleId);
    if(nullptr == role)
        return;

    refreshShabakeDailyAwardInfo(role, rev->position);
}

Coord2D ShaBaKe::getReliveOrBornPos(FactionId roleFaction) const
{
    if(m_tempWinFaction == 0 || m_tempWinFaction != roleFaction)
        return m_sbkCfg.offensivePos;
    return m_sbkCfg.defensivePos;
}

bool ShaBaKe::canTransfer(FactionId roleFaction, uint32_t triggerTplId) const
{
    if(triggerTplId == m_sbkCfg.defensiveTriggerDoor)
        return 0 != m_tempWinFaction && m_tempWinFaction == roleFaction;

    return true;
}

void ShaBaKe::syncCurTempWinFactionToFunc(FactionId winfaction)
{
    PrivateRaw::SyncCurTempWinFaction sync;
    sync.factionId = winfaction;

    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(SyncCurTempWinFaction), &sync, sizeof(sync));
}

void ShaBaKe::requestIntoShabake(Role::Ptr role)
{
    if(!m_inTime)
    {
        role->sendSysChat("不在活动时间");
        return;
    }

    RolesAndScenes::me().gotoOtherScene(role->id(), m_sbkCfg.outcityScene, getReliveOrBornPos(role->factionId()));
}

uint32_t ShaBaKe::lefttime() const
{
    if(!m_inTime)
        return 0;
    using namespace std::chrono;
    return duration_cast<seconds>(m_endtime - Clock::now()).count();
}

void ShaBaKe::afterEnterScene(Role::Ptr role)
{
    auto s = role->scene();
    if(nullptr == s)
        return;
    if(s->copyType() != CopyMap::shabake)
        return;
    refreshShabakeProgress(role);
    if(isPalaceScene(role->sceneId()) && role->factionId() > 0)
    {
        m_palaceFactionRole[role->factionId()].insert(role->id());
        changeWinFaction();
    }
}

void ShaBaKe::subFactionRole(Role::Ptr role)
{
    auto s = role->scene();
    if(nullptr == s)
        return;
    if(!m_inTime || !isPalaceScene(s->id()))
        return;
    auto it = m_palaceFactionRole.find(role->factionId());
    if(it != m_palaceFactionRole.end())
    {
        it->second.erase(role->id());
        if(it->second.size() <= 0)
            m_palaceFactionRole.erase(it);
    }
    changeWinFaction();
}

void ShaBaKe::changeWinFaction()
{
    if(m_palaceFactionRole.size() == 1)
    {
        auto winIt = m_palaceFactionRole.begin();
        if(winIt->first != m_tempWinFaction)
        {//发生占领权变更
            m_tempWinFaction = winIt->first;
            syncCurTempWinFactionToFunc(winIt->first);
        }
    }
}

void ShaBaKe::dealRelive(Role::Ptr role, Scene::Ptr s, bool autoRelive)
{
    if(!role->isDead())
        return;

    if(!autoRelive 
       && s->reliveYB() > 0 
       && !role->reduceMoney(MoneyType::money_4, s->reliveYB(), "复活消耗"))
        return;
    Coord2D relivePos = getReliveOrBornPos(role->factionId());
    role->relive();

    auto ValidPosExec = [&](Coord2D pos)
    {
        if(!s->enterable(pos, SceneItemType::role))
            return false;
        relivePos = pos;
        return true;
    };
    s->tryExecSpiral(relivePos, 10, ValidPosExec);
    RolesAndScenes::me().gotoOtherScene(role->id(), m_sbkCfg.outcityScene, relivePos);
}

bool ShaBaKe::isPalaceScene(SceneId sceneId) const
{
    return m_sbkCfg.palaceScene == sceneId;
}

void ShaBaKe::refreshShabakeProgress(Role::Ptr role)
{
    using namespace std::chrono;
    PublicRaw::RefreshProgressOfShabake refresh;
    refresh.starttime = toUnixTime(m_starttime);
    refresh.endtime = toUnixTime(m_endtime);
    refresh.wintime = 0;

    if(m_tempWinTime != EPOCH)
    {
        refresh.wintime = duration_cast<seconds>(Clock::now() - m_tempWinTime).count();
    }
    std::memset(refresh.curWinFaction, 0, sizeof(refresh.curWinFaction));
    m_tempWinFactionName.copy(refresh.curWinFaction, sizeof(refresh.curWinFaction)-1);

    role->sendToMe(RAWMSG_CODE_PUBLIC(RefreshProgressOfShabake), &refresh, sizeof(refresh));
}

void ShaBaKe::npcDie(Npc::Ptr npc)
{
    if(!m_inTime)
        return;
    if(npc->sceneId() != m_sbkCfg.outcityScene)
        return;
    if(npc->tplId() == m_sbkCfg.outcityWallDoor)
    {//外城城门npc
        for(const auto& it : m_cacheBlockDoor)
        {
            Trigger::Ptr trigger = TriggerManager::me().getById(it);
            if(nullptr != trigger)
                trigger->markErase();
        }
        return;
    }

    //墙壁
    for(const auto& it : m_sbkCfg.wallNpcInfo)
    {
        if(npc->tplId() == it.npcid)
        {
            m_wallNpcIds.erase(npc->id());
            auto s = npc->scene();
            if(nullptr == s)
                return;
            if(nullptr == s->summonTrigger(it.doorid, it.doorpos, 5))
            {
                LOG_DEBUG("沙巴克, 召唤墙壁进攻方传送门失败, door:{}, pos:{}", it.doorid, it.doorpos);
            }
            return;
        }
    }
}

std::string ShaBaKe::kingStatue() const
{
    return m_kingStatue;
}

uint32_t ShaBaKe::kingStatueId() const
{
    return m_sbkCfg.statueId;
}

void ShaBaKe::winShabake()
{
    Scene::Ptr palace_s = SceneManager::me().getById(m_sbkCfg.palaceScene);
    Scene::Ptr outcity_s = SceneManager::me().getById(m_sbkCfg.outcityScene);
    if(nullptr == palace_s || nullptr == outcity_s)
        return;

    if(m_tempWinFaction <= 0)
        return;

    PrivateRaw::WinShabake send;
    send.factionId = m_tempWinFaction;

    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(WinShabake), &send, sizeof(send));
}

void ShaBaKe::refreshShabakeDailyAwardInfo(Role::Ptr role, ShabakePosition position)
{
    PublicRaw::RetShabakeDailyAwardInfo ret;
    if(0 == role->m_roleCounter.get(CounterType::shabakeDailyAward))
        ret.canGet = true;
    ret.position = position;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetShabakeDailyAwardInfo), &ret, sizeof(ret));
}

}
