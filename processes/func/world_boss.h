#ifndef PROCESS_FUNC_WORLD_BOSS_H
#define PROCESS_FUNC_WORLD_BOSS_H

#include "water/common/scenedef.h"
#include "water/componet/datetime.h"
#include "water/componet/coord.h"
#include <vector>
#include <vector>

namespace func{

//世界boss副本状态
enum class WBState : uint8_t
{
    no_start    = 0, //未开启
    started     = 1, //已开启
    end         = 2, //已结束
};

using water::componet::TimePoint;
using water::componet::Coord2D;

class WorldBoss
{
public:
    ~WorldBoss() = default;

private:
    WorldBoss();

public:
    static WorldBoss& me();

    void loadConfig(const std::string& cfgDir);
    void regMsgHandler();
    void regTimer();

    void timeExec(const TimePoint& now);
    uint32_t activityTime(const TimePoint& now) const;

    SceneId sceneId() const;
    uint16_t bossLv() const;


private:
    void resetWorldBoss();
    void startWorldBoss(const TimePoint& now, uint32_t sec);
    void endWorldBoss();

    //预通知(10分钟 5分钟)
    void preNotifyWorldBossStart(const TimePoint& now);
    //进行中的活动通知
    void notifyInWorldBossTime(const TimePoint& now);

    //根据boss被击杀耗时来处理下一次boss等级
    void selectNextWorldBossLv();

private:
    void servermsg_BossBoxBelonged(const uint8_t* msgData, uint32_t size);
    void servermsg_KillWorldBoss(const uint8_t* msgData, uint32_t size);
    void clientmsg_ReqWorldBossInfo(const uint8_t* msgData, uint32_t size, uint64_t rid);

private:
    //配置
    std::vector<std::pair<uint32_t, uint32_t>> m_activityTime; //活动时间
    std::vector<uint16_t> m_bossLvInfos;
    SceneId  m_sceneId;    //世界boss地图
    uint16_t m_baseBossLv; //boss基础等级
    uint16_t m_bossUpTime; //boss升级击杀的耗时时长

private:
    WBState     m_wbState;
    TimePoint   m_startTime;  //世界boss开始时间
    uint16_t    m_killBossCostTime; //杀死boss耗时时长(秒)

    uint8_t     m_notifyState; //0:还未通知  1:10分钟已通知  2:5分钟已通知
    TimePoint   m_notifyTime;
};

}

#endif
