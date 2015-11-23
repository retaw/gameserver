#include "world_boss.h"
#include "channel.h"
#include "func.h"
#include "role_manager.h"
#include "global_sundry.h"

#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"

#include "protocol/rawmsg/private/world_boss.h"
#include "protocol/rawmsg/private/world_boss.codedef.private.h"

#include "protocol/rawmsg/public/world_boss.h"
#include "protocol/rawmsg/public/world_boss.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace func{

using water::componet::Clock;
using namespace water::componet;

WorldBoss::WorldBoss()
: m_wbState(WBState::no_start)
, m_killBossCostTime(0)
, m_notifyState(0)
, m_notifyTime(EPOCH)
{
}

WorldBoss& WorldBoss::me()
{
    static WorldBoss me;
    return me;
}

void WorldBoss::loadConfig(const std::string& cfgDir)
{
    using water::componet::XmlParseDoc;
    using water::componet::XmlParseNode;
    const std::string file = cfgDir + "/world_boss.xml";                                                 
    XmlParseDoc doc(file);                                                                         
    XmlParseNode root = doc.getRoot();                                                             
    if(!root)                                                                                      
    {                                                                                              
        EXCEPTION(componet::ExceptionBase, file + " parse root node failed");                      
        return;                                                                                    
    }                                                                                              

    XmlParseNode timeNode = root.getChild("Time");
    if(!timeNode)
    {
        LOG_DEBUG("世界boss, 加载Time节点失败");
        return;
    }
    for(XmlParseNode itemNode = timeNode.getChild("item"); itemNode; ++itemNode)        
    {
        std::string starttime = itemNode.getAttr<std::string>("start");
        std::string endtime = itemNode.getAttr<std::string>("end");

        ::tm start_detail, end_detail;
        if(nullptr == ::strptime(starttime.c_str(), "%H:%M:%S", &start_detail) 
           || nullptr == ::strptime(endtime.c_str(), "%H:%M:%S", &end_detail))
        {
            LOG_DEBUG("世界boss, 加载活动时间失败, start:{}, end:{}", starttime, endtime);
            continue;
        }

        uint32_t start = start_detail.tm_hour * 3600 + start_detail.tm_min * 60 + start_detail.tm_sec;
        uint32_t end = end_detail.tm_hour * 3600 + end_detail.tm_min * 60 + end_detail.tm_sec;

        LOG_DEBUG("世界boss, 活动时间, starttime:{}, endtime:{}, start:{}, end:{}", starttime, endtime, start, end);
        m_activityTime.push_back({start, end});
    }

    XmlParseNode mapNode = root.getChild("Map");
    if(mapNode)
        m_sceneId = mapNode.getAttr<SceneId>("id");

    XmlParseNode bossNode = root.getChild("Boss");
    if(bossNode)
    {
        m_baseBossLv = bossNode.getAttr<uint16_t>("baseLv");
        m_bossUpTime = bossNode.getAttr<uint16_t>("up");

        for(XmlParseNode itemNode = bossNode.getChild("item"); itemNode; ++itemNode)
        {
            uint16_t lv = itemNode.getAttr<uint16_t>("lv");
            LOG_DEBUG("世界boss, 加载boss等级配置, lv:{}", lv);
            m_bossLvInfos.push_back(lv);
        }
    }
}

void WorldBoss::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(BossBoxBelonged, std::bind(&WorldBoss::servermsg_BossBoxBelonged, this, _1, _2));
    REG_RAWMSG_PRIVATE(KillWorldBoss, std::bind(&WorldBoss::servermsg_KillWorldBoss, this, _1, _2));
    REG_RAWMSG_PUBLIC(ReqWorldBossInfo, std::bind(&WorldBoss::clientmsg_ReqWorldBossInfo, this, _1, _2, _3));
}

void WorldBoss::regTimer()
{
    Func::me().regTimer(std::chrono::seconds(1),
                        std::bind(&WorldBoss::timeExec, this, std::placeholders::_1));
}

void WorldBoss::timeExec(const TimePoint& now)
{
    uint32_t sec = activityTime(now);
    switch(m_wbState)
    {
    case WBState::no_start:
        {
            preNotifyWorldBossStart(now);
            if(sec > 0)
                startWorldBoss(now, sec);
        }
        break;
    case WBState::started:
        {
            notifyInWorldBossTime(now);
            if(0 == sec)
                endWorldBoss();
        }
        break;
    case WBState::end:
        {
            if(0 == sec)
                resetWorldBoss();
        }
        break;
    }
}

uint32_t WorldBoss::activityTime(const TimePoint& now) const
{
    TimePoint beginDay = beginOfDay(now);
    for(const auto& iter : m_activityTime)
    {
        if(now >= beginDay + std::chrono::seconds {iter.first} 
           && now <= beginDay + std::chrono::seconds {iter.second})
            return iter.second - iter.first;
    }
    return 0;
}

void WorldBoss::resetWorldBoss()
{
    m_wbState = WBState::no_start;
    m_notifyState = 0;
}

/*
 * sec: 活动持续时长
 */
void WorldBoss::startWorldBoss(const TimePoint& now, uint32_t sec)
{
    m_wbState = WBState::started;
    m_startTime = now;
    m_killBossCostTime = 0;

    PrivateRaw::StartWorldBoss send;
    send.bossLv = bossLv();
    send.sec = sec;
    Func::me().broadcastToWorlds(RAWMSG_CODE_PRIVATE(StartWorldBoss), &send, sizeof(send));
}

void WorldBoss::endWorldBoss()
{
    LOG_DEBUG("世界boss结束");
    m_wbState = WBState::end;
    PrivateRaw::EndWorldBoss send;
    Func::me().broadcastToWorlds(RAWMSG_CODE_PRIVATE(EndWorldBoss), &send, sizeof(send));

    selectNextWorldBossLv();
}

void WorldBoss::preNotifyWorldBossStart(const TimePoint& now)
{
    if(m_notifyState >= 2)
        return;
    TimePoint beginDay = beginOfDay(now);
    for(const auto& iter : m_activityTime)
    {
        if(m_notifyState == 0 
           && beginDay + std::chrono::seconds {iter.first} <= now + std::chrono::minutes {10}
           && beginDay + std::chrono::seconds {iter.first} >= now + std::chrono::minutes {5})
        {
            //10分钟通知
            Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "10分钟后, 世界BOSS将会祸乱我们的家园, 英雄们击杀它可以获取大量经验和神装材料");
            m_notifyState = 1;
            return;
        }
        else if(m_notifyState == 1
                && beginDay + std::chrono::seconds {iter.first} <= now + std::chrono::minutes {5}
                && beginDay + std::chrono::seconds {iter.first} >= now)
        {
            //5分钟通知
            Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "5分钟后, 世界BOSS将会祸乱我们的家园, 英雄们击杀它可以获取大量经验和神装材料");
            m_notifyState = 2;
            return;
        }
    }
}

void WorldBoss::notifyInWorldBossTime(const TimePoint& now)
{
    //活动期间每5分钟通知一次
    if(m_notifyTime + std::chrono::minutes {5} <= Clock::now())
    {
        m_notifyTime = Clock::now();
        Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "世界BOSS正在祸乱我们的家园, 英雄们速速来击杀获取大量经验和神装材料吧!");
    }
}

/*
 *规则:
 * 1> 世界BOSS活动开启配置规定时间以内被杀死则会成长1级
 *  2>世界BOSS活动开启置规定时间以后，但是在规定活动时间以内被击杀时，则BOSS不会升级
 *  3> 活动时间以内还没击杀世界BOSS，世界BOSS降一级(降到基础等级为止)
 */
void WorldBoss::selectNextWorldBossLv()
{
    if(0 == m_killBossCostTime && GlobalSundry::me().m_worldBossLv != m_baseBossLv)
    {
        //降级
        for(const auto& iter : m_bossLvInfos)
        {
            if(iter == GlobalSundry::me().m_worldBossLv)
                break;
            GlobalSundry::me().m_worldBossLv = iter;
        }
    }
    else if(m_killBossCostTime < m_bossUpTime)
    {
        //升级
        auto iter = m_bossLvInfos.begin();
        for(; iter != m_bossLvInfos.end(); ++iter)
        {
            if(GlobalSundry::me().m_worldBossLv == *iter)
            {
                if(++iter != m_bossLvInfos.end())
                    GlobalSundry::me().m_worldBossLv = *iter;
                break;
            }
        }
    }
}

void WorldBoss::servermsg_BossBoxBelonged(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::BossBoxBelonged*>(msgData);
    endWorldBoss();
    LOG_DEBUG("世界boss, 宝箱归属:{}", rev->holder);
}

void WorldBoss::servermsg_KillWorldBoss(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::KillWorldBoss*>(msgData);
    using namespace std::chrono;
    m_killBossCostTime = duration_cast<seconds>(Clock::now() - m_startTime).count();
    LOG_DEBUG("世界boss, 杀死boss耗时:{}, 击杀者:{}", m_killBossCostTime, rev->killer);
}

void WorldBoss::clientmsg_ReqWorldBossInfo(const uint8_t* msgData, uint32_t size, uint64_t rid)
{
    auto role = RoleManager::me().getById(rid);
    if(nullptr == role)
        return;
    PrivateRaw::TransitWorldBossInfo send;
    send.id = rid;
    send.bossLv = bossLv();
    role->sendToWorld(RAWMSG_CODE_PRIVATE(TransitWorldBossInfo), &send, sizeof(send));
}

SceneId WorldBoss::sceneId() const
{
    return m_sceneId;
}

uint16_t WorldBoss::bossLv() const
{
    if(GlobalSundry::me().m_worldBossLv == 0)
        return m_baseBossLv;
    return GlobalSundry::me().m_worldBossLv;
}

}
