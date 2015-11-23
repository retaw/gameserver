#ifndef PROCESS_FUNC_SHA_BA_KE_H
#define PROCESS_FUNC_SHA_BA_KE_H

#include "water/common/roledef.h"
#include "water/common/scenedef.h"
#include "water/componet/datetime.h"
#include "water/componet/serialize.h"

#include <memory>
#include <unordered_map>
#include <set>

namespace func{

enum ShabakeState : uint8_t
{
    no_start    = 0,    //未开始
    started     = 1,
    end         = 2,
};

using namespace water;
using namespace water::componet;

class Role;
class ShaBaKe
{
private:
    ShaBaKe();

public:
    ~ShaBaKe() = default;
    static ShaBaKe& me();

public:
    void loadConfig(const std::string& cfgDir);
    void regTimer();
    void regMsgHandler();

    void serializeData(Serialize<std::string>& iss);
    void deserializeData(Deserialize<std::string>& oss);

    //
    FactionId winFactionId() const;
    //
    ShabakePosition getPositionByName(const std::string& name) const;

private:
    void timeExec(const TimePoint& now);
    //是否在活动时间
    bool inActivityTime(const TimePoint& now) const;
    //获取下次活动开始,结束时间
    std::pair<TimePoint, TimePoint> nextActivityTime(const TimePoint& now) const;
    //获取合服前下次活动开始,结束时间
    std::pair<TimePoint, TimePoint> getBeforeMergeNextActivityTime(const TimePoint& now) const;
    //合服后
    std::pair<TimePoint, TimePoint> getAfterMergeNextActivityTime(const TimePoint& now) const;

private:
    std::pair<uint32_t, uint32_t> getStartEndTimeByStr(const std::string& timestr);
    

private:
    void servermsg_WinShabake(const uint8_t* msgData, uint32_t size);
    void servermsg_SyncCurTempWinFaction(const uint8_t* msgData, uint32_t size);
    void servermsg_SyncShabakeSceneRoleToFunc(const uint8_t* msgData, uint32_t size);
    void servermsg_WorldRetGiveRoleAward(const uint8_t* msgData, uint32_t size);
    void servermsg_ReqShabakeKing(const uint8_t* msgData, uint32_t size, uint64_t pid);

    void clientmsg_ReqShabakeInfo(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqGetShabakeAward(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqShabakaAwardInfo(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqShabakeDailyAwardInfo(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqShabakePosition(const uint8_t* msgData, uint32_t size, RoleId roleId);
    void clientmsg_ReqSetShabakePosition(const uint8_t* msgData, uint32_t size, RoleId roleId);

    void startShabake();
    void endShabake();
    void resetShabake();

    //通知玩家活动结束
    void notifyRoleShabakeOver(RoleId roleId, const std::string& king, const std::string& faction);
    //是否在设置官职时间内
    bool inSetPositionTime() const;
    //自动设置官职(结束10分钟后自动设置)
    void autoSetPosition(bool client=false);
    //刷新官职信息
    void refreshPosition(std::shared_ptr<Role>);
    //通知world发奖励
    void notifyWorldGiveShabakeAward(std::shared_ptr<Role> role, bool first, ShabakeAward awardtype);

    //刷新领奖界面信息
    void refreshShabakeAwardInfo(RoleId roleId);


private:
    //合服前配置
    struct BeforeMergeCfg
    {
        uint32_t first;     //开服后第一次开启活动间隔天数
        uint32_t second;    //第二次活动间隔天数
        std::pair<uint32_t, uint32_t> firstActivityTime; //第一次活动具体时间
        std::pair<uint32_t, uint32_t> secondActivityTime;//第二次

        std::vector<uint32_t> weekday;  //平常周几开活动
        std::pair<uint32_t, uint32_t> weekdayActivityTime;//平常开活动具体时间
    };

    //合服后配置
    struct AfterMergeCfg
    {
        uint32_t first;     //合服后第一次开启活动距离合服时间天数
        std::pair<uint32_t, uint32_t> firstActivityTime; //活动具体时间

        std::vector<uint32_t> weekday;
        std::pair<uint32_t, uint32_t> weekdayActivityTime;
    };

    BeforeMergeCfg m_beforeMergeCfg;
    AfterMergeCfg m_afterMergeCfg;
    SceneId m_palaceScene;  //皇宫地图
    SceneId m_outcityScene; //外城地图


private:
    ShabakeState m_state;   //活动状态
    TimePoint m_endtime;    //活动结束时间(用来自动设置官职)
    uint16_t m_openCount;   //活动开启次数(只记录前面两次)
    FactionId m_winFaction; //上一次赢得争夺战的帮派id
    FactionId m_tempWinFaction; //活动中暂时占领的帮派
    std::unordered_map<RoleId, uint8_t> m_parttakeRoleAward; //争夺战结束后还在活动地图上的玩家
    bool m_autosetFlag;     //服务器自动设置官职标志

    std::pair<TimePoint, TimePoint> m_startingTime; //正在开启活动的时间段

#pragma pack(1)
    struct RolePositionInfo
    {
        Job job;
        Sex sex;
        ShabakePosition position;
    };
#pragma pack()
    std::unordered_map<std::string, RolePositionInfo> m_shabakePositionRole;


    //记录正在领奖玩家的id,相当于一个流程锁
    std::set<RoleId>    m_awardingRole;
};

}

#endif
