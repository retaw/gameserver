#include "world_boss.h"
#include "role_manager.h"
#include "roles_and_scenes.h"
#include "scene_manager.h"
#include "channel.h"
#include "mail_manager.h"
#include "action_manager.h"

#include "water/componet/string_kit.h"
#include "water/componet/logger.h"  
#include "water/componet/xmlparse.h"

#include "protocol/rawmsg/public/world_boss.h"
#include "protocol/rawmsg/public/world_boss.codedef.public.h"

#include "protocol/rawmsg/private/world_boss.h"
#include "protocol/rawmsg/private/world_boss.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

using water::componet::XmlParseDoc; 
using water::componet::XmlParseNode;
using namespace water::componet;

WorldBoss::WorldBoss()
: m_step(0)
, m_bossDie(false)
{
}

WorldBoss& WorldBoss::me()
{
    static WorldBoss me;
    return me;
}

void WorldBoss::loadConfig(const std::string& cfgDir)
{
    const std::string file = cfgDir + "/world_boss.xml";                                                 
    XmlParseDoc doc(file);                                                                         
    XmlParseNode root = doc.getRoot();                                                             
    if(!root)                                                                                      
    {                                                                                              
        EXCEPTION(componet::ExceptionBase, file + " parse root node failed");                      
        return;                                                                                    
    }                                                                                              

    XmlParseNode mapNode = root.getChild("Map");
    if(nullptr == mapNode)
    {
        EXCEPTION(componet::ExceptionBase, file + " parse Map node failed");                      
        return;
    }

    m_wbCfg.sceneId = mapNode.getAttr<SceneId>("id");
    m_wbCfg.enterPos = Coord2D(mapNode.getAttr<std::string>("enterpos"));
    m_wbCfg.needLevel = mapNode.getAttr<uint32_t>("needlevel");

    m_wbCfg.doorID = mapNode.getAttr<uint32_t>("door");
    m_wbCfg.doorPos = Coord2D(mapNode.getAttr<std::string>("doorpos"));

    XmlParseNode bossNode = root.getChild("Boss");
    if(!bossNode)
    {
        EXCEPTION(componet::ExceptionBase, file + " parse Boss node failed");                      
        return;
    }
    m_wbCfg.bossPos = Coord2D(bossNode.getAttr<std::string>("pos"));
    for(XmlParseNode itemNode = bossNode.getChild("item"); itemNode; ++itemNode)
    {
        uint16_t lv = itemNode.getAttr<uint16_t>("lv");
        uint32_t id = itemNode.getAttr<uint32_t>("id");
        m_wbCfg.bossInfos.insert({lv, id});
    }

    //个人累积伤害配置
    XmlParseNode damageAwardNode = root.getChild("DamageAward");
    if(damageAwardNode)
    {
        for(XmlParseNode itemNode = damageAwardNode.getChild("item"); itemNode; ++itemNode)
        {
            uint32_t damage = itemNode.getAttr<uint32_t>("damage");
            for(XmlParseNode objNode = itemNode.getChild("obj"); objNode; ++objNode)
            {
                ObjItem obj;
                obj.tplId = objNode.getAttr<TplId>("id");
                obj.num = objNode.getAttr<uint32_t>("num");
                obj.bind = static_cast<Bind>(objNode.getAttr<uint8_t>("bind"));

                LOG_DEBUG("世界boss, 个人累积伤害奖励配置: id:{}, num:{}, bind:{}", obj.tplId, obj.num, obj.bind);
                m_wbCfg.damageAwards[damage].push_back(obj);
            }
        }
    }

    //伤害排名奖励
    XmlParseNode rankAwardNode = root.getChild("RankAward");
    if(rankAwardNode)
    {
        for(XmlParseNode itemNode = rankAwardNode.getChild("item"); itemNode; ++itemNode)
        {
            std::pair<uint16_t, uint16_t> rankKey;
            std::vector<std::string> vs = splitString(itemNode.getAttr<std::string>("rank"), "-");
            if(vs.size() >= 2)
            {
                rankKey.first = atoi(vs[0].c_str());
                rankKey.second = atoi(vs[1].c_str());
            }
            for(XmlParseNode objNode = itemNode.getChild("obj"); objNode; ++objNode)
            {
                ObjItem obj;
                obj.tplId = objNode.getAttr<TplId>("id");
                obj.num = objNode.getAttr<uint32_t>("num");
                obj.bind = static_cast<Bind>(objNode.getAttr<uint8_t>("bind"));

                LOG_DEBUG("世界boss, 伤害排名奖励配置: id:{}, num:{}, bind:{}", obj.tplId, obj.num, obj.bind);
                m_wbCfg.rankAwards[rankKey].push_back(obj);
            }
        }
    }

    //boss最后一击奖励
    XmlParseNode killAwardNode = root.getChild("KillAward");
    if(killAwardNode)
    {
        for(XmlParseNode itemNode = killAwardNode.getChild("item"); itemNode; ++itemNode)
        {
            ObjItem obj;
            obj.tplId = itemNode.getAttr<TplId>("id");
            obj.num = itemNode.getAttr<uint32_t>("num");
            obj.bind = static_cast<Bind>(itemNode.getAttr<uint8_t>("bind"));

            LOG_DEBUG("世界boss, boss最后一击奖励配置: id:{}, num:{}, bind:{}", obj.tplId, obj.num, obj.bind);
            m_wbCfg.killAwards.push_back(obj);
        }
    }

    //宝箱奖励
    XmlParseNode boxNode = root.getChild("Box");
    if(boxNode)
    {
        m_wbCfg.triggerBoxId = boxNode.getAttr<uint32_t>("trigger_id");
        m_wbCfg.boxID = boxNode.getAttr<uint32_t>("id");
        m_wbCfg.boxPos = Coord2D(boxNode.getAttr<std::string>("pos"));
        m_wbCfg.boxHoldTime = boxNode.getAttr<uint16_t>("holdtime");
        LOG_DEBUG("世界boss, 宝箱配置加载, trigger_id:{}, boxID:{}", m_wbCfg.triggerBoxId, m_wbCfg.boxID);
    }
}

void WorldBoss::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(StartWorldBoss, std::bind(&WorldBoss::servermsg_StartWorldBoss, this, _1, _2));
    REG_RAWMSG_PRIVATE(EndWorldBoss, std::bind(&WorldBoss::servermsg_EndWorldBoss, this, _1, _2));
    REG_RAWMSG_PRIVATE(TransitWorldBossInfo, std::bind(&WorldBoss::servermsg_TransitWorldBossInfo, this, _1, _2));

    REG_RAWMSG_PUBLIC(RequestDamageRank, std::bind(&WorldBoss::clientmsg_RequestDamageRank, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqGetDamageAward, std::bind(&WorldBoss::clientmsg_ReqGetDamageAward, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqDamageAwardInfo, std::bind(&WorldBoss::clientmsg_ReqDamageAwardInfo, this, _1, _2, _3));
}

void WorldBoss::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&WorldBoss::timeExec, this, StdInterval::sec_1, _1));
    World::me().regTimer(std::chrono::seconds(3), 
                         std::bind(&WorldBoss::timeExec, this, StdInterval::sec_3, _1));
}

void WorldBoss::timeExec(StdInterval interval, const TimePoint& now)
{
    if(!inTime())
        return;
    switch(interval)
    {
    case StdInterval::sec_1:
        {
            if(1 == m_step && m_bossDie)
            {
                m_step = 2;
                //激活传送门
                auto s = SceneManager::me().getById(m_wbCfg.sceneId);
                if(nullptr == s)
                    return;
                sortRoleDamage();
                s->execRole([this](Role::Ptr role) { refreshDamageRank(role); });
                s->summonTrigger(m_wbCfg.doorID, m_wbCfg.doorPos, 5);
                s->summonTrigger(m_wbCfg.triggerBoxId, m_wbCfg.boxPos, 5);
            }
        }
        break;
    case StdInterval::sec_3:
        {
            if(!m_bossDie)
                sortRoleDamage();
        }
        break;
    default:
        break;
    }
}

bool WorldBoss::inTime() const
{
    return m_inTime;
}

uint16_t WorldBoss::leftTime() const
{
    if(!inTime())
        return 0;

    using namespace std::chrono;
    return duration_cast<seconds>(m_endTime - Clock::now()).count();
}

void WorldBoss::clearHoldBoxState(Role::Ptr role)
{
    if(nullptr == role)
        return;
    if(!role->clearWorldBossBoxStatus())
        return;
    auto s = role->scene();
    if(nullptr == s)
        return;

    s->summonTrigger(m_wbCfg.triggerBoxId, role->pos(), 5);
}

uint16_t WorldBoss::boxNeedHoldSeconds() const
{
    return m_wbCfg.boxHoldTime;
}

/*
 * role: 宝箱归属者
 */
void WorldBoss::boxBelongRole(Role::Ptr role)
{
    if(!inTime())
        return;
    PrivateRaw::BossBoxBelonged send;
    std::memset(send.holder, 0, sizeof(send.holder)-1);
    ProcessIdentity funcId("func", 1);
    if(nullptr == role)
    {
        //说明宝箱消失
        World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(BossBoxBelonged), &send, sizeof(send));
        return;
    }

    m_boxHolder = role->name();
    role->name().copy(send.holder, sizeof(send.holder)-1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(BossBoxBelonged), &send, sizeof(send));

    //给宝箱持有者发送宝箱奖励
    ObjItem obj;
    obj.tplId = m_wbCfg.boxID;
    obj.num = 1;
    obj.bind = Bind::yes;
    MailManager::me().send(role->id(), "世界BOSS宝箱归属奖励", "您夺取了世界BOSS掉落的宝箱，获得宝箱奖励", obj);
}

void WorldBoss::npcDie(TplId npcTplId, Role::Ptr atk)
{
    if(!m_inTime)
        return;
    if(nullptr == atk || !isWorldBossMap(atk->sceneId()))
        return;
    if(!isWorldBoss(npcTplId))
        return;
    
    m_bossDie = true;
    PrivateRaw::KillWorldBoss send;
    std::memset(send.killer, 0, sizeof(send.killer)-1);
    atk->name().copy(send.killer, sizeof(send.killer)-1);
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(KillWorldBoss), &send, sizeof(send));

    //世界boss死亡的一些处理
    Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "世界BOSS被{}玩家击杀, 快来拾取世界BOSS掉落的宝箱吧！", atk->name());
    MailManager::me().send(atk->id(), "世界BOSS最后一击奖励", "您一击终结了世界BOSS, 获得最后一击奖励", m_wbCfg.killAwards);
}

void WorldBoss::npcDamage(TplId npcTplId, PK::Ptr atk, uint32_t dmg)
{
    if(!m_inTime || 0 == dmg)
        return;
    if(nullptr == atk || !isWorldBossMap(atk->sceneId()))
        return;
    if(!isWorldBoss(npcTplId))
        return;

    Role::Ptr atkRole = getRole(atk);
    if(nullptr == atkRole)
        return;

    m_roleDamages[atkRole->name()] += dmg;
    atkRole->m_roleSundry.m_worldBossDamage += dmg;
}

void WorldBoss::refreshDamageRank(Role::Ptr role)
{
    if(nullptr == role || m_damageRankCache.empty())
        return;
    std::vector<uint8_t> buf;
    buf.reserve(512);
    buf.resize(sizeof(PublicRaw::RefreshDamageRank));

    uint16_t rank = 1;
    bool findSelf = false;
    for(const auto& iter : m_damageRankCache)
    {
        if(rank > 10 && findSelf)
            break;
        if(rank <= 10)
        {
            buf.resize(buf.size() + sizeof(PublicRaw::RefreshDamageRank::DamageInfo));
            auto msg = reinterpret_cast<PublicRaw::RefreshDamageRank*>(buf.data());
            iter.first.copy(msg->info[msg->size].name, MAX_NAME_SZIE);
            msg->info[msg->size].damage = iter.second;
            ++msg->size;
        }

        if(iter.first == role->name())
        {
            auto msg = reinterpret_cast<PublicRaw::RefreshDamageRank*>(buf.data());
            msg->selfRank = rank;
            msg->selfDamage = iter.second;
            findSelf = true;
        }
        ++rank;
    }

    role->sendToMe(RAWMSG_CODE_PUBLIC(RefreshDamageRank), buf.data(), buf.size());
}

void WorldBoss::servermsg_StartWorldBoss(const uint8_t* msgData, uint32_t size)
{
    m_inTime = true;
    auto rev = reinterpret_cast<const PrivateRaw::StartWorldBoss*>(msgData);
    m_endTime = Clock::now() + std::chrono::seconds {rev->sec};
    m_boxHolder.clear();
    ActionManager::me().setActionState(ActionType::world_boss, ActionState::begin);
    auto s = SceneManager::me().getById(m_wbCfg.sceneId);
    if(nullptr == s)
        return;

    LOG_DEBUG("世界boss, 开始召唤boss, LV:{}", rev->bossLv);
    auto iter = m_wbCfg.bossInfos.find(rev->bossLv);
    if(iter == m_wbCfg.bossInfos.end())
    {
        LOG_DEBUG("世界boss, 活动开启找不到要召唤的boss配置, lv:{}", rev->bossLv);
        return;
    }

    if(nullptr == s->summonNpc(iter->second, m_wbCfg.bossPos, 5))
    {
        LOG_DEBUG("世界boss, summonNpc失败, bossID:{}, pos:{{},{}}", iter->second, m_wbCfg.bossPos.x, m_wbCfg.bossPos.y);
        return;
    }
    m_step = 1;
    m_bossDie = false;
    m_bossID = iter->second;

    m_damageRankCache.clear();
    m_roleDamages.clear();
}

void WorldBoss::servermsg_EndWorldBoss(const uint8_t* msgData, uint32_t size)
{
    if(!inTime())
        return;
    m_inTime = false;
    ActionManager::me().setActionState(ActionType::world_boss, ActionState::end);
    auto s = SceneManager::me().getById(m_wbCfg.sceneId);
    if(nullptr == s)
        return;

    s->execRole([this](Role::Ptr role) { notifyWorldBossOver(role); });
    s->removeSceneItem();

    //发送伤害排行奖励
    giveDamageRankAward();
    m_damageRankCache.clear();
    m_roleDamages.clear();
    m_boxHolder.clear();
}

void WorldBoss::servermsg_TransitWorldBossInfo(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::TransitWorldBossInfo*>(msgData);
    auto role = RoleManager::me().getById(rev->id);
    if(nullptr == role)
        return;

    PublicRaw::RetWorldBossInfo ret;
    ret.bossLv = rev->bossLv;
    ret.totalDamage = role->m_roleSundry.m_worldBossDamage;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetWorldBossInfo), &ret, sizeof(ret));
}

void WorldBoss::clientmsg_RequestDamageRank(const uint8_t* msgData, uint32_t size, uint64_t rid)
{
    auto role = RoleManager::me().getById(rid);
    if(nullptr == role)
        return;
    if(!isWorldBossMap(role->sceneId()))
        return;
    refreshDamageRank(role);
}

void WorldBoss::clientmsg_ReqGetDamageAward(const uint8_t* msgData, uint32_t size, uint64_t rid)
{
    auto role = RoleManager::me().getById(rid);
    if(nullptr == role)
        return;
    auto rev = reinterpret_cast<const PublicRaw::ReqGetDamageAward*>(msgData);
    if(rev->all)
    {
        giveOrMailDamageAward(role);
    }
    else
    {
        if(rev->damageIndex > role->m_roleSundry.m_worldBossDamage)
            return;
        auto it = m_wbCfg.damageAwards.find(rev->damageIndex);
        if(it == m_wbCfg.damageAwards.end())
            return;
        if(!role->checkPutObj(it->second))
        {
            role->sendSysChat("包裹空间不足, 无法领取奖励");
            return;
        }
        role->putObj(it->second);
        role->m_roleSundry.m_receivedWBDamageAward.insert(rev->damageIndex);
    }

    refreshDamageAwardInfo(role);
}

void WorldBoss::clientmsg_ReqDamageAwardInfo(const uint8_t* msgData, uint32_t size, uint64_t rid)
{
    auto role = RoleManager::me().getById(rid);
    if(nullptr == role)
        return;
    refreshDamageAwardInfo(role);
}

void WorldBoss::refreshDamageAwardInfo(Role::Ptr role)
{
    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::RefreshDamageAwardInfo));
    auto msg = reinterpret_cast<PublicRaw::RefreshDamageAwardInfo*>(buf.data());
    msg->totalDamage = role->m_roleSundry.m_worldBossDamage;

    for(const auto& it : m_wbCfg.damageAwards)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RefreshDamageAwardInfo::DamageAwardInfo));
        auto msg = reinterpret_cast<PublicRaw::RefreshDamageAwardInfo*>(buf.data());
        msg->info[msg->size].damageIndex = it.first;
        if(role->m_roleSundry.m_receivedWBDamageAward.find(it.first) != role->m_roleSundry.m_receivedWBDamageAward.end())
            msg->info[msg->size].state = 2;
        else if(msg->totalDamage >= it.first)
            msg->info[msg->size].state = 1;
        else
            msg->info[msg->size].state = 0;
        ++msg->size;
    }

    role->sendToMe(RAWMSG_CODE_PUBLIC(RefreshDamageAwardInfo), buf.data(), buf.size());
}

void WorldBoss::sortRoleDamage()
{
    m_damageRankCache.clear();
    for(const auto& iter : m_roleDamages)
        m_damageRankCache.push_back({iter.first, iter.second});

    struct DamageComp
    {
        bool operator() (const std::pair<std::string, uint32_t>& lhs, const std::pair<std::string, uint32_t>& rhs) const
        {
            return lhs.second > rhs.second;
        } 
    };
    std::sort(m_damageRankCache.begin(), m_damageRankCache.end(), DamageComp());
}

void WorldBoss::requestIntoWorldBoss(Role::Ptr role)
{
    if(nullptr == role)
        return;
    auto s = role->scene();
    if(nullptr == s || isWorldBossMap(role->sceneId()))
        return;

    if(!m_inTime)
    {
        role->sendSysChat("不在活动时间");
        return;
    }

    if(role->level() < m_wbCfg.needLevel)
    {
        role->sendSysChat("等级不够, 需要{}级才能参加", m_wbCfg.needLevel);
        return;
    }

    if(s->copyType() != CopyMap::none)
    {
        role->sendSysChat("在副本中不能参加世界boss活动");
        return;
    }

    if(role->teamId() > 0)
    {
        role->sendSysChat("你现在是组队状态, 请先脱离队伍再参加活动!");
        return;
    }

    RolesAndScenes::me().gotoOtherScene(role->id(), m_wbCfg.sceneId, m_wbCfg.enterPos);
}

bool WorldBoss::isWorldBoss(TplId npcTplId) const
{
    return npcTplId == m_bossID;
}

bool WorldBoss::isWorldBossMap(SceneId sceneId) const
{
    return m_wbCfg.sceneId == sceneId;
}

void WorldBoss::notifyWorldBossOver(Role::Ptr role)
{
    if(nullptr == role)
        return;
    PublicRaw::NotifyWorldBossOver notify;
    std::memset(notify.boxHolder, 0, sizeof(notify.boxHolder)-1);
    m_boxHolder.copy(notify.boxHolder, sizeof(notify.boxHolder)-1);
    role->sendToMe(RAWMSG_CODE_PUBLIC(NotifyWorldBossOver), &notify, sizeof(notify));
}

void WorldBoss::giveDamageRankAward()
{
    uint16_t rank = 0;
    for(const auto& rankIt : m_damageRankCache)
    {
        ++rank;
        for(const auto& awardIt : m_wbCfg.rankAwards)
        {
            if(rank >= awardIt.first.first && rank <= awardIt.first.second)
            {
                //满足条件
                auto& awardObjs = awardIt.second;
                std::string text = "您参与了世界BOSS活动, 获得第" + toString(rank) + "名排名奖励";
                MailManager::me().send(rankIt.first, "世界BOSS伤害排名奖励", text, awardObjs);
                break;
            }
        }
    }
}

void WorldBoss::giveOrMailDamageAward(Role::Ptr role, bool mail/*=false*/)
{
    if(0 == role->m_roleSundry.m_worldBossDamage)
        return;

    std::vector<ObjItem> finalObjs;
    std::map<std::pair<TplId, Bind>, uint32_t> sumObjs;
    std::set<uint32_t> damageIndexs;
    auto itup = m_wbCfg.damageAwards.upper_bound(role->m_roleSundry.m_worldBossDamage);
    for(auto it = m_wbCfg.damageAwards.begin(); it != itup; ++it)
    {
        if(role->m_roleSundry.m_receivedWBDamageAward.find(it->first) != role->m_roleSundry.m_receivedWBDamageAward.end())
            continue;

        damageIndexs.insert(it->first);
        for(const auto& objIt : it->second)
        {
            std::pair<TplId, Bind> tmp;
            tmp.first = objIt.tplId;
            tmp.second = objIt.bind;
            sumObjs[tmp] += objIt.num;
        }
    }

    for(const auto& it : sumObjs)
    {
        ObjItem obj;
        obj.tplId = it.first.first;
        obj.bind = it.first.second;
        obj.num = it.second;
        finalObjs.push_back(obj);
    }

    if(mail)
    {
        if(finalObjs.empty())
            return;
        MailManager::me().send(role->id(), "世界BOSS自动领取奖励", "您没有领取上一次世界BOSS活动奖励, 现自动为您领取", finalObjs);
    }
    else
    {
        if(finalObjs.empty())
        {
            role->sendSysChat("没有可以领取的奖励");
            return;
        }

        if(!role->checkPutObj(finalObjs))
        {
            role->sendSysChat("包裹空间不足，不能一键领取");
            return;
        }
        role->putObj(finalObjs);
        role->m_roleSundry.m_receivedWBDamageAward.insert(damageIndexs.begin(), damageIndexs.end());
    }
}

IconState WorldBoss::iconState(Role::Ptr me) const
{
    if(me->m_roleSundry.m_worldBossDamage == 0)
        return IconState::no;
    for(const auto& it : m_wbCfg.damageAwards)
    {
        if(me->m_roleSundry.m_receivedWBDamageAward.find(it.first) == me->m_roleSundry.m_receivedWBDamageAward.end()
           && me->m_roleSundry.m_worldBossDamage >= it.first)
            return IconState::yes;
    }
    return IconState::no;
}

}
