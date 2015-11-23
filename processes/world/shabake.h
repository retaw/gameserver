#ifndef PROCESS_WORLD_SHA_BA_KE_H
#define PROCESS_WORLD_SHA_BA_KE_H

#include "water/common/roledef.h"
#include "water/common/factiondef.h"
#include "water/common/scenedef.h"
#include "water/componet/coord.h"
#include "water/componet/datetime.h"

#include <map>
#include <set>
#include <memory>

namespace world{

using namespace water;
using namespace water::componet;

class Role;
class Npc;
class Scene;
class ShaBaKe
{
private:
    ShaBaKe();

public:
    ~ShaBaKe() = default;
    static ShaBaKe& me();

public:
    void loadConfig(const std::string& cfgDir);
    void regMsgHandler();
    void regTimer();

    //获取出生(复活)点
    Coord2D getReliveOrBornPos(FactionId roleFaction) const;
    //能否通过机关传送门
    bool canTransfer(FactionId roleFaction, uint32_t triggerTplId) const;

    //一旦出现占领帮派变更,就同步到func
    void syncCurTempWinFactionToFunc(FactionId winfaction);
    //请求进入沙巴克地图
    void requestIntoShabake(std::shared_ptr<Role>);
    //沙巴克剩余时间
    uint32_t lefttime() const;

    //进地图处理
    void afterEnterScene(std::shared_ptr<Role>);
    //复活处理
    void dealRelive(std::shared_ptr<Role> role, std::shared_ptr<Scene> s, bool autoRelive);
    //删除沙巴克皇宫帮派玩家
    void subFactionRole(std::shared_ptr<Role>);

    //npc死亡处理
    void npcDie(std::shared_ptr<Npc>);

    //城主名字
    std::string kingStatue() const;
    //城主雕塑id
    uint32_t kingStatueId() const;

private:
    void servermsg_StartShabake(const uint8_t* msgData, uint32_t size);
    void servermsg_SyncCurTempWinFaction(const uint8_t* msgData, uint32_t size);
    void servermsg_EndShabake(const uint8_t* msgData, uint32_t size, uint64_t pid);
    void servermsg_NotifyWorldGiveRoleAward(const uint8_t* msgData, uint32_t size, uint64_t pid);
    void servermsg_RetShabakeKing(const uint8_t* msgData, uint32_t size);
    void servermsg_FuncReqDailyAwardInfo(const uint8_t* msgData, uint32_t size);

    void timeExec(StdInterval interval, const TimePoint& now);

    //是否皇宫地图
    bool isPalaceScene(SceneId) const;
    //更新沙巴克争夺进度
    void refreshShabakeProgress(std::shared_ptr<Role>);
    //赢得沙巴克争夺
    void winShabake();
    //沙巴克占领帮派变更
    void changeWinFaction();

    //刷新日常奖励
    void refreshShabakeDailyAwardInfo(std::shared_ptr<Role>, ShabakePosition);

private:
    struct WallNpc
    {
        uint32_t npcid;     //墙壁npcid
        Coord2D npcpos;     //墙壁npc坐标
        uint16_t doorid;    //进攻方传送门id
        Coord2D doorpos;    //进攻方进出皇宫的传送门位置
    };

    struct ShabakeCfg
    {
        SceneId palaceScene;    //皇宫地图
        SceneId outcityScene;   //外城地图
        Coord2D offensivePos;   //进攻方出生点(死亡复活点)
        Coord2D defensivePos;   //防守方

        uint16_t ownmin;        //活动开启后持续占领皇宫多长时间提前结束活动
        uint32_t level;         //参加活动需要等级
        uint32_t statueId;      //雕塑id
        Coord2D statuePos;      //雕塑pos

        uint32_t defensiveTriggerDoor;  //防守方自由进出皇宫的传送门机关id
        Coord2D defensiveTriggerDoorPos;//防守方自由进出皇宫的传送门机关坐标

        std::vector<WallNpc> wallNpcInfo;   //皇宫墙壁npc信息

        uint32_t outcityWallDoor;   //外城城门npcid
        Coord2D outcityWallDoorPos; //外城城门npc坐标
        std::vector<Coord2D> blockDoorPos; //外城城门动态阻挡坐标

        uint32_t firstWinReward; //首次占领奖励
        uint32_t winReward; //占领奖励

        uint32_t firstKingReward; // 首位城主奖励
        uint32_t kingReward; //城主奖励

        uint32_t parttakeReward; //参与奖

        std::map<ShabakePosition, uint32_t> dailyReward; //占领帮派日常奖励
    };

    ShabakeCfg m_sbkCfg;

private:
    bool    m_inTime;
    bool    m_existKingStatue;
    std::string m_kingStatue; //城主雕塑名称
    TimePoint m_starttime;  //开始时间
    TimePoint m_endtime;    //活动标准结束时间
    FactionId m_tempWinFaction;
    std::string m_tempWinFactionName;
    TimePoint m_tempWinTime; //占领开始时间
    std::map<FactionId, std::set<RoleId>> m_palaceFactionRole;   //皇宫一层玩家(有帮派)记录<帮派id, 玩家数量>
    
    std::vector<TriggerId> m_cacheBlockDoor;
    std::set<PKId> m_wallNpcIds;
};

}

#endif
