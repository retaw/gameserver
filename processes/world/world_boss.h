#ifndef PROCESS_WORLD_WORLD_BOSS_H
#define PROCESS_WORLD_WORLD_BOSS_H

#include "water/common/roledef.h"
#include "water/common/scenedef.h"
#include "water/common/funcdef.h"
#include "water/componet/coord.h"
#include "water/componet/datetime.h"
#include <memory>
#include <map>
#include <unordered_map>

namespace world{

using water::componet::Coord2D;
using water::componet::TimePoint;

class PK;
class Role;
class WorldBoss
{
private:
    WorldBoss();

public:
    ~WorldBoss() = default;

public:
    static WorldBoss& me();

    void loadConfig(const std::string& cfgDir);
    void regMsgHandler();
    void regTimer();
    void timeExec(StdInterval interval, const TimePoint& now);


public:
    //是否活动时间
    bool inTime() const;
    //获取世界boss剩余时间
    uint16_t leftTime() const;
    //玩家请求进入世界boss副本
    void requestIntoWorldBoss(std::shared_ptr<Role> role);
    //清空宝箱持有状态
    void clearHoldBoxState(std::shared_ptr<Role> role);
    //宝箱需持有时间(秒)
    uint16_t boxNeedHoldSeconds() const;
    //宝箱有归属, 世界boss提前结束
    void boxBelongRole(std::shared_ptr<Role> role);
    //
    void npcDie(TplId npcTplId, std::shared_ptr<Role> atk);
    //npc伤害
    void npcDamage(TplId npcTplId, std::shared_ptr<PK> atk, uint32_t dmg);
    //刷新客户端伤害排行榜
    void refreshDamageRank(std::shared_ptr<Role> role);

    //发放玩家伤害累积奖励
    void giveOrMailDamageAward(std::shared_ptr<Role> role, bool mail=false);

    //得到世界boss Icons状态
    IconState iconState(std::shared_ptr<Role>) const;

private:
    void servermsg_StartWorldBoss(const uint8_t* msgData, uint32_t size);
    void servermsg_EndWorldBoss(const uint8_t* msgData, uint32_t size);
    void servermsg_TransitWorldBossInfo(const uint8_t* msgData, uint32_t size);

    void clientmsg_RequestDamageRank(const uint8_t* msgData, uint32_t size, uint64_t rid);
    void clientmsg_ReqGetDamageAward(const uint8_t* msgData, uint32_t size, uint64_t rid);
    void clientmsg_ReqDamageAwardInfo(const uint8_t* msgData, uint32_t size, uint64_t rid);

private:
    //伤害排序
    void sortRoleDamage();
    //判断是否为世界boss
    bool isWorldBoss(TplId npcTplId) const;
    //是否为世界boss地图
    bool isWorldBossMap(SceneId sceneId) const;

    //通知玩家世界boss副本结束
    void notifyWorldBossOver(std::shared_ptr<Role> role);
    //发放世界boss伤害排名奖励
    void giveDamageRankAward();
    //刷新伤害奖励领取界面信息
    void refreshDamageAwardInfo(std::shared_ptr<Role> role);

private:
    struct WBCfg
    {
        SceneId sceneId;  //世界boss sceneid(注:世界boss做成静态地图)
        Coord2D enterPos;      //进地图坐标点
        uint32_t needLevel;

        //传送门配置
        uint32_t doorID;
        Coord2D doorPos;

        //boss配置
        Coord2D bossPos;    //boss出生点
        std::unordered_map<uint16_t, uint32_t> bossInfos; //<lv, bossID>

        //个人累积伤害奖励
        std::map<uint32_t, std::vector<ObjItem>> damageAwards; //<damage, >

        //排名奖励
        std::map<std::pair<uint16_t, uint16_t>, std::vector<ObjItem>> rankAwards;

        //最后一击奖励
        std::vector<ObjItem> killAwards;

        //宝箱
        uint16_t triggerBoxId;
        uint32_t boxID;
        Coord2D boxPos; //宝箱刷新出来坐标
        uint16_t boxHoldTime; //宝箱归属需要的持有时间
    };

    WBCfg m_wbCfg;
    
private:
    uint16_t    m_bossID;   //bossID
    bool        m_inTime;   //是否开启
    TimePoint   m_endTime;  //世界boss结束时间
    uint8_t     m_step;     //1:第一阶段  2:第二阶段

    std::unordered_map<std::string, uint32_t>   m_roleDamages; //本次活动对boss伤害角色列表
    std::vector<std::pair<std::string, uint32_t>> m_damageRankCache; //伤害排行榜
    bool        m_bossDie;
    std::string m_boxHolder; //宝箱获得者(临时缓存一下)
};

}

#endif
