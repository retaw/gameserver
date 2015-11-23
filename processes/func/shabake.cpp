#include "shabake.h"
#include "role_manager.h"
#include "faction_manager.h"
#include "all_role_info_manager.h"
#include "channel.h"
#include "func.h"
#include "global_sundry.h"

#include "water/componet/string_kit.h"
#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"
#include "water/componet/exception.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/shabake.h"
#include "protocol/rawmsg/public/shabake.codedef.public.h"
#include "protocol/rawmsg/private/shabake.h"
#include "protocol/rawmsg/private/shabake.codedef.private.h"


namespace func{

ShaBaKe::ShaBaKe()
:m_state(ShabakeState::no_start)
,m_openCount(0)
,m_winFaction(0)
,m_tempWinFaction(0)
,m_autosetFlag(false)
{
}

ShaBaKe& ShaBaKe::me()
{
    static ShaBaKe me;
    return me;
}

void ShaBaKe::loadConfig(const std::string& cfgDir)
{
    using water::componet::XmlParseDoc;
    using water::componet::XmlParseNode;
    const std::string file = cfgDir + "/shabake.xml";
    XmlParseDoc doc(file);                                                                         
    XmlParseNode root = doc.getRoot();                                                             
    if(!root)
    {
        EXCEPTION(ExceptionBase, file + " parse root node failed");                      
        return;
    }

    XmlParseNode mapNode = root.getChild("Map");
    if(!mapNode)
    {
        EXCEPTION(ExceptionBase, file + " parse Map node failed");                      
        return;
    }
    m_palaceScene = mapNode.getAttr<SceneId>("palace");
    m_outcityScene = mapNode.getAttr<SceneId>("outcity");

    XmlParseNode beforeNode = root.getChild("BeforeMerge");
    if(!beforeNode)
    {
        EXCEPTION(ExceptionBase, file + " parse BeforeMerge node failed");                      
        return;
    }
    std::vector<std::string> firstvec = splitString(beforeNode.getAttr<std::string>("first"), ",");
    std::vector<std::string> secondvec = splitString(beforeNode.getAttr<std::string>("second"), ",");
    std::vector<std::string> weekvec = splitString(beforeNode.getAttr<std::string>("weekday"), ",");
    if(firstvec.size() < 2 || secondvec.size() < 2 || weekvec.size() < 2)
    {
        EXCEPTION(ExceptionBase, file + " beforeMerge first or second parse error")
        return;
    }
    m_beforeMergeCfg.first = fromString<uint32_t>(firstvec[0]);
    m_beforeMergeCfg.firstActivityTime = getStartEndTimeByStr(firstvec[1]);
    m_beforeMergeCfg.second = fromString<uint32_t>(secondvec[0]);
    m_beforeMergeCfg.secondActivityTime = getStartEndTimeByStr(secondvec[1]);

    std::vector<std::string> weekdayVec = splitString(weekvec[0], "-");
    for(const auto& it : weekdayVec)
    {
        m_beforeMergeCfg.weekday.push_back(fromString<uint32_t>(it));
    }
    m_beforeMergeCfg.weekdayActivityTime = getStartEndTimeByStr(weekvec[1]);

    XmlParseNode afterNode = root.getChild("AfterMerge");
    if(!afterNode)
    {
        EXCEPTION(ExceptionBase, file + " parse AfterMerge node failed");                      
        return;
    }
    firstvec.clear();
    weekvec.clear();
    firstvec = splitString(afterNode.getAttr<std::string>("first"), ",");
    weekvec = splitString(afterNode.getAttr<std::string>("weekday"), ",");
    if(firstvec.size() < 2 || weekvec.size() < 2)
    {
        EXCEPTION(ExceptionBase, file + " afterMerge first or weekday parse error")
        return;
    }
    m_afterMergeCfg.first = fromString<uint32_t>(firstvec[0]);
    m_afterMergeCfg.firstActivityTime = getStartEndTimeByStr(firstvec[1]);

    weekdayVec.clear();
    weekdayVec = splitString(weekvec[0], "-");
    for(const auto& it : weekdayVec)
    {
        m_afterMergeCfg.weekday.push_back(fromString<uint32_t>(it));
    }
    m_afterMergeCfg.weekdayActivityTime = getStartEndTimeByStr(weekvec[1]);
}

std::pair<uint32_t, uint32_t> ShaBaKe::getStartEndTimeByStr(const std::string& timestr)
{
    std::pair<uint32_t, uint32_t> ret(0,0);
    std::vector<std::string> timevec = splitString(timestr, "~");
    if(timevec.size() < 2)
    {
        LOG_DEBUG("沙巴克, 加载活动时间失败,splitString");
        return ret;
    }

    ::tm start_detail, end_detail;
    if(nullptr == ::strptime(timevec[0].c_str(), "%H:%M:%S", &start_detail) 
       || nullptr == ::strptime(timevec[1].c_str(), "%H:%M:%S", &end_detail))
    {
        LOG_DEBUG("沙巴克, 加载活动时间失败,strptime");
        return ret;
    }

    ret.first = start_detail.tm_hour * 3600 + start_detail.tm_min * 60 + start_detail.tm_sec;
    ret.second = end_detail.tm_hour * 3600 + end_detail.tm_min * 60 + end_detail.tm_sec;
    LOG_DEBUG("沙巴克, 加载活动时间, {}, start:{}, end:{}", timestr, ret.first, ret.second);
    return ret;
}

void ShaBaKe::regTimer()
{
    Func::me().regTimer(std::chrono::seconds(2),
                        std::bind(&ShaBaKe::timeExec, this, std::placeholders::_1));
}

void ShaBaKe::timeExec(const TimePoint& now)
{
    switch(m_state)
    {
    case ShabakeState::no_start:
        {
            if(inActivityTime(now))
                startShabake();
        }
        break;
    case ShabakeState::started:
        {
            if(!inActivityTime(now))
                endShabake();
        }
        break;
    case ShabakeState::end:
        {
            //if(!inActivityTime(now))
            resetShabake();
        }
        break;
    }
    autoSetPosition();
}

bool ShaBaKe::inActivityTime(const TimePoint& now) const
{
    std::pair<TimePoint, TimePoint> nexttime = nextActivityTime(now);
    if(nexttime.first == EPOCH)
        return false;

    return nexttime.first <= now && nexttime.second >= now;
}

std::pair<TimePoint, TimePoint> ShaBaKe::nextActivityTime(const TimePoint& now) const
{
    if(m_state == ShabakeState::started)
        return m_startingTime;

    std::pair<TimePoint, TimePoint> ret(EPOCH, EPOCH);
    if(!Func::me().mergeFlag())//未合服
        return getBeforeMergeNextActivityTime(now);
    return getAfterMergeNextActivityTime(now);
}

std::pair<TimePoint, TimePoint> ShaBaKe::getBeforeMergeNextActivityTime(const TimePoint& now) const
{
    using namespace std::chrono;
    std::pair<TimePoint, TimePoint> ret(EPOCH, EPOCH);
    if(m_openCount < 1)
    {
        TimePoint opentime = Func::me().opentime();
        TimePoint beginOfOpentimeDay = beginOfDay(opentime);
        ret.first = beginOfOpentimeDay + seconds{(m_beforeMergeCfg.first-1) * 24 * 60 * 60 + m_beforeMergeCfg.firstActivityTime.first};
        ret.second = ret.first + seconds{m_beforeMergeCfg.firstActivityTime.second - m_beforeMergeCfg.firstActivityTime.first};
        return ret;
    }
    else if(m_openCount >= 1 && m_openCount < 2)
    {
        TimePoint opentime = Func::me().opentime();
        TimePoint beginOfOpentimeDay = beginOfDay(opentime);
        ret.first = beginOfOpentimeDay + seconds{(m_beforeMergeCfg.second-1) * 24 * 60 * 60 + m_beforeMergeCfg.secondActivityTime.first};
        ret.second = ret.first + seconds{m_beforeMergeCfg.secondActivityTime.second - m_beforeMergeCfg.firstActivityTime.first};
        return ret;
    }

    if(m_beforeMergeCfg.weekday.empty())
        return ret;
    //平常开活动
    TimePoint beginweekday = beginOfWeek(now, 1);
    for(const auto& it : m_beforeMergeCfg.weekday)
    {
        ret.first = beginweekday + seconds{(it-1) * 24 * 60 * 60 + m_beforeMergeCfg.weekdayActivityTime.first};
        if(now <= ret.first + seconds{10})//给10秒的容错区间(因为一旦满负荷运行并不能保证一个timetick循环过程不耗费太长时间)
        {
            ret.second = ret.first + seconds{m_beforeMergeCfg.weekdayActivityTime.second - m_beforeMergeCfg.weekdayActivityTime.first};
            return ret;
        }
    }

    auto it = m_beforeMergeCfg.weekday.begin();
    beginweekday = beginweekday + seconds{7 * 24 * 60 * 60}; //获取下一个礼拜
    ret.first = beginweekday + seconds{(*it - 1) * 24 * 60 * 60 + m_beforeMergeCfg.weekdayActivityTime.first};
    ret.second = ret.first + seconds{m_beforeMergeCfg.weekdayActivityTime.second - m_beforeMergeCfg.weekdayActivityTime.first};
    return ret;
}

std::pair<TimePoint, TimePoint> ShaBaKe::getAfterMergeNextActivityTime(const TimePoint& now) const
{
    using namespace std::chrono;
    std::pair<TimePoint, TimePoint> ret(EPOCH, EPOCH);
    if(m_openCount < 1)
    {
        TimePoint opentime = Func::me().opentime();
        TimePoint beginOfOpentimeDay = beginOfDay(opentime);
        ret.first = beginOfOpentimeDay + seconds{(m_beforeMergeCfg.first-1) * 24 * 60 * 60 + m_afterMergeCfg.firstActivityTime.first};
        ret.second = ret.first + seconds{m_afterMergeCfg.firstActivityTime.second - m_afterMergeCfg.firstActivityTime.first};
        return ret;
    }

    if(m_afterMergeCfg.weekday.empty())
        return ret;
    //平常开活动
    TimePoint beginweekday = beginOfWeek(now, 1);
    for(const auto& it : m_afterMergeCfg.weekday)
    {
        ret.first = beginweekday + seconds{(it-1) * 24 * 60 * 60 + m_afterMergeCfg.weekdayActivityTime.first};
        if(now <= ret.first + seconds{10})
        {
            ret.second = ret.first + seconds{m_afterMergeCfg.weekdayActivityTime.second - m_afterMergeCfg.weekdayActivityTime.first};
            return ret;
        }
    }

    auto it = m_afterMergeCfg.weekday.begin();
    beginweekday = beginweekday + seconds{7 * 24 * 60 * 60}; //获取下一个礼拜
    ret.first = beginweekday + seconds{(*it - 1) * 24 * 60 * 60 + m_afterMergeCfg.weekdayActivityTime.first};
    ret.second = ret.first + seconds{m_afterMergeCfg.weekdayActivityTime.second - m_afterMergeCfg.weekdayActivityTime.first};
    return ret;
}

void ShaBaKe::startShabake()
{
    TimePoint now = Clock::now();
    std::pair<TimePoint, TimePoint> nexttime = nextActivityTime(now);
    if(nexttime.first + std::chrono::seconds{10} <= now)
    {
        LOG_DEBUG("沙巴克, 异常开启该活动");
        return;
    }

    m_state = ShabakeState::started;
    m_tempWinFaction = m_winFaction;
    m_startingTime = nexttime;

    if(m_openCount < 2)
        ++m_openCount;

    using namespace std::chrono;
    PrivateRaw::StartShabake send;
    send.lastWinFaction = m_tempWinFaction;
    std::memset(send.winFactionName, 0, sizeof(send.winFactionName));
    std::string factionName = FactionManager::me().getFactionName(m_tempWinFaction);
    if(factionName != "")
        factionName.copy(send.winFactionName, sizeof(send.winFactionName)-1);
    send.sec = duration_cast<seconds>(nexttime.second - nexttime.first).count();
    Func::me().broadcastToWorlds(RAWMSG_CODE_PRIVATE(StartShabake), &send, sizeof(send));

    LOG_DEBUG("沙巴克, 活动开启");
}

void ShaBaKe::endShabake()
{
    if(m_state != ShabakeState::started)
        return;
    m_state = ShabakeState::end;
    m_endtime = Clock::now();
    m_winFaction = m_tempWinFaction;

    m_shabakePositionRole.clear();
    m_autosetFlag = false;

    RoleId leader = FactionManager::me().getFactionLeaderRoleId(m_winFaction);
    if(0 == leader)
    {
        LOG_DEBUG("沙巴克, 结束, 获取不到城主, factionId:{}", m_winFaction);
    }

    RoleInfo::Ptr roleInfo = RoleInfoManager::me().getRoleInfoById(leader);
    if(nullptr == roleInfo)
    {
        LOG_DEBUG("沙巴克, RoleInfoManager获取不到数据, roleId:{}", leader);
    }

    PrivateRaw::EndShabake send;
    std::memset(send.king, 0, sizeof(send.king));
    if(nullptr != roleInfo)
    {
        roleInfo->name.copy(send.king, sizeof(send.king)-1);

        RolePositionInfo info;
        info.job = roleInfo->job;
        info.sex = roleInfo->sex;
        info.position = ShabakePosition::king;
        m_shabakePositionRole.insert({roleInfo->name, info});
    }
    Func::me().broadcastToWorlds(RAWMSG_CODE_PRIVATE(EndShabake), &send, sizeof(send));

    std::string factionName = FactionManager::me().getFactionName(m_winFaction);
    LOG_DEBUG("沙巴克, 争夺战结束, 胜利帮派:{}", factionName);
    Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "恭喜{}帮派成功占领沙巴克成为名副其实的沙巴克王者", factionName);
}

void ShaBaKe::resetShabake()
{
    m_state = ShabakeState::no_start;
}

void ShaBaKe::serializeData(Serialize<std::string>& iss)
{
    iss << m_openCount;
    iss << m_winFaction;
    iss << m_parttakeRoleAward;
    iss << m_autosetFlag;
    iss << (uint32_t)m_shabakePositionRole.size();
    for(const auto& it : m_shabakePositionRole)
    {
        iss << it.first;
        iss << it.second;
    }
}

void ShaBaKe::deserializeData(Deserialize<std::string>& oss)
{
    oss >> m_openCount;
    oss >> m_winFaction;
    oss >> m_parttakeRoleAward;
    oss >> m_autosetFlag;
    uint32_t size = 0;
    oss >> size;
    for(uint32_t i = 0; i < size; ++i)
    {
        std::string name;
        RolePositionInfo info;
        oss >> name;
        oss >> info;
        m_shabakePositionRole.insert({name, info});
    }
}

FactionId ShaBaKe::winFactionId() const
{
    return m_winFaction;
}

ShabakePosition ShaBaKe::getPositionByName(const std::string& name) const
{
    auto it = m_shabakePositionRole.find(name);
    if(it == m_shabakePositionRole.end())
        return ShabakePosition::none;
    return it->second.position;
}

void ShaBaKe::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(WinShabake, std::bind(&ShaBaKe::servermsg_WinShabake, this, _1, _2));
    REG_RAWMSG_PRIVATE(SyncCurTempWinFaction, std::bind(&ShaBaKe::servermsg_SyncCurTempWinFaction, this, _1, _2));
    REG_RAWMSG_PRIVATE(SyncShabakeSceneRoleToFunc, std::bind(&ShaBaKe::servermsg_SyncShabakeSceneRoleToFunc, this, _1, _2));
    REG_RAWMSG_PRIVATE(WorldRetGiveRoleAward, std::bind(&ShaBaKe::servermsg_WorldRetGiveRoleAward, this, _1, _2));
    REG_RAWMSG_PRIVATE(ReqShabakeKing, std::bind(&ShaBaKe::servermsg_ReqShabakeKing, this, _1, _2, _3));
    
    REG_RAWMSG_PUBLIC(ReqShabakeInfo, std::bind(&ShaBaKe::clientmsg_ReqShabakeInfo, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqGetShabakeAward, std::bind(&ShaBaKe::clientmsg_ReqGetShabakeAward, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqShabakaAwardInfo, std::bind(&ShaBaKe::clientmsg_ReqShabakaAwardInfo, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqShabakeDailyAwardInfo, std::bind(&ShaBaKe::clientmsg_ReqShabakeDailyAwardInfo, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqShabakePosition, std::bind(&ShaBaKe::clientmsg_ReqShabakePosition, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqSetShabakePosition, std::bind(&ShaBaKe::clientmsg_ReqSetShabakePosition, this, _1, _2, _3));

}

void ShaBaKe::servermsg_WinShabake(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::WinShabake*>(msgData);
    m_tempWinFaction = rev->factionId;

    endShabake();
}

void ShaBaKe::servermsg_SyncCurTempWinFaction(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::SyncCurTempWinFaction*>(msgData);
    m_tempWinFaction = rev->factionId;

    std::string factionName = FactionManager::me().getFactionName(m_tempWinFaction);
    Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "{}帮派已占据了沙巴克皇宫, 距离占领沙巴克只剩一步之遥", factionName);

    //同步到各world
    PrivateRaw::SyncCurTempWinFaction send;
    std::memset(send.factionName, 0, sizeof(send.factionName));
    send.factionId = rev->factionId;
    factionName.copy(send.factionName, sizeof(send.factionName)-1);
    Func::me().broadcastToWorlds(RAWMSG_CODE_PRIVATE(SyncCurTempWinFaction), &send, sizeof(send));
}

void ShaBaKe::servermsg_SyncShabakeSceneRoleToFunc(const uint8_t* msgData, uint32_t size)
{
    m_parttakeRoleAward.clear();
    std::string kingName = FactionManager::me().getFactionLeaderName(m_winFaction);
    std::string factionName = FactionManager::me().getFactionName(m_winFaction);
    auto rev = reinterpret_cast<const PrivateRaw::SyncShabakeSceneRoleToFunc*>(msgData);
    for(ArraySize size = 0; size < rev->size; ++size)
    {
        if(m_parttakeRoleAward.insert({rev->roleId[size], 0}).second)
            notifyRoleShabakeOver(rev->roleId[size], kingName, factionName);
    }
}

void ShaBaKe::servermsg_WorldRetGiveRoleAward(const uint8_t* msgData, uint32_t size)
{
    auto rev = reinterpret_cast<const PrivateRaw::WorldRetGiveRoleAward*>(msgData);
    m_awardingRole.erase(rev->roleId);
    if(rev->success && rev->awardtype != ShabakeAward::daily)
    {
        m_parttakeRoleAward[rev->roleId] |= static_cast<uint8_t>(rev->awardtype);
        refreshShabakeAwardInfo(rev->roleId);
    }

    LOG_DEBUG("沙巴克, role:{} 领取奖励{}, awardtype:{}", rev->roleId, rev->success ? "成功":"失败", rev->awardtype);
}

void ShaBaKe::servermsg_ReqShabakeKing(const uint8_t* msgData, uint32_t size, uint64_t pid)
{
    PrivateRaw::RetShabakeKing ret;
    std::memset(&ret, 0, sizeof(ret));
    for(const auto& it : m_shabakePositionRole)
    {
        if(it.second.position == ShabakePosition::king)
        {
            it.first.copy(ret.king, sizeof(ret.king)-1);
            break;
        }
    }

    ProcessIdentity remotePid(pid);
    Func::me().sendToPrivate(remotePid, RAWMSG_CODE_PRIVATE(RetShabakeKing), &ret, sizeof(ret));
}

void ShaBaKe::clientmsg_ReqShabakeInfo(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    PublicRaw::RetShabakeInfo ret;
    std::memset(&ret, 0, sizeof(ret));
    std::string faction = FactionManager::me().getFactionName(m_winFaction);
    if(faction != "")
        faction.copy(ret.factionName, sizeof(ret.factionName) - 1);

    for(const auto& it : m_shabakePositionRole)
    {
        if(it.second.position == ShabakePosition::king)
        {
            it.first.copy(ret.info[0].name, sizeof(ret.info[0].name) - 1);
            ret.info[0].job = it.second.job;
            ret.info[0].sex = it.second.sex;
        }
        else if(it.second.position == ShabakePosition::leftViceKing)
        {
            it.first.copy(ret.info[1].name, sizeof(ret.info[1].name) - 1);
            ret.info[1].job = it.second.job;
            ret.info[1].sex = it.second.sex;
        }
        else if(it.second.position == ShabakePosition::rightViceKing)
        {
            it.first.copy(ret.info[2].name, sizeof(ret.info[2].name) - 1);
            ret.info[2].job = it.second.job;
            ret.info[2].sex = it.second.sex;
        }
    }
    if(m_state == ShabakeState::started)
        ret.nextActivityTime = 0;
    else
    {
        std::pair<TimePoint, TimePoint> nexttime = nextActivityTime(Clock::now());
        ret.nextActivityTime = std::chrono::duration_cast<std::chrono::seconds>(nexttime.first - Clock::now()).count();
    }

    role->sendToMe(RAWMSG_CODE_PUBLIC(RetShabakeInfo), &ret, sizeof(ret));
}

void ShaBaKe::clientmsg_ReqGetShabakeAward(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    if(m_awardingRole.find(roleId) != m_awardingRole.end())
    {
        LOG_DEBUG("沙巴克, 正在领取奖励中, role:{}", role->name());
        return;
    }

    auto rev = reinterpret_cast<const PublicRaw::ReqGetShabakeAward*>(msgData);
    //首先城主是可以领取一切奖励的(前提是未曾领取过)
    auto kingIt = m_shabakePositionRole.find(role->name());
    if(kingIt != m_shabakePositionRole.end() && kingIt->second.position == ShabakePosition::king)
    {
        if(m_parttakeRoleAward.find(roleId) == m_parttakeRoleAward.end())
            m_parttakeRoleAward.insert({roleId, 0});
        if(0 != (m_parttakeRoleAward[roleId] & static_cast<uint8_t>(rev->awardtype)))
        {
            role->sendSysChat("已经领取过, 不能再领");
            return;
        }

        m_awardingRole.insert(roleId);
        notifyWorldGiveShabakeAward(role, m_openCount == 1, rev->awardtype);
        return;
    }

    switch(rev->awardtype)
    {
    case ShabakeAward::king:
        {
            role->sendSysChat("不是城主, 不能领取城主奖励");
            return;
        }
        break;
    case ShabakeAward::win:
        {
            auto awardIt = m_parttakeRoleAward.find(roleId);
            if(awardIt == m_parttakeRoleAward.end())
            {
                role->sendSysChat("只有坚持战斗到最后一个刻的英雄才可以领取奖励");
                return;
            }

            if(role->factionId() == 0 || role->factionId() != m_winFaction)
            {
                role->sendSysChat("不是占领帮派成员, 不能领取该奖励");
                return;
            }

            if(0 != (m_parttakeRoleAward[roleId] & static_cast<uint8_t>(rev->awardtype)))
            {
                role->sendSysChat("已经领取过, 不能再领");
                return;
            }
        }
        break;
    case ShabakeAward::parttake:
        {
            auto awardIt = m_parttakeRoleAward.find(roleId);
            if(awardIt == m_parttakeRoleAward.end())
            {
                role->sendSysChat("只有坚持战斗到最后一个刻的英雄才可以领取奖励");
                return;
            }

            if(0 != (m_parttakeRoleAward[roleId] & static_cast<uint8_t>(rev->awardtype)))
            {
                role->sendSysChat("已经领取过, 不能再领");
                return;
            }
        }
        break;
    case ShabakeAward::daily:
        {//daily是否可领取丢给world去检查
            if(inSetPositionTime())
            {
                role->sendSysChat("沙城争霸结束10分钟内为城主设定官职时间无法领取每日奖励");
                return;
            }

            if(role->factionId() == 0 || role->factionId() != m_winFaction)
            {
                role->sendSysChat("不是占领帮派成员, 不能领取该奖励");
                return;
            }
        }
        break;
    default:
        {
            LOG_DEBUG("沙巴克, 收到领取未知奖励type:{}, role:{}", rev->awardtype, role->name());
        }
        return;
    }

    m_awardingRole.insert(roleId);
    notifyWorldGiveShabakeAward(role, m_openCount == 1, rev->awardtype);
}

void ShaBaKe::clientmsg_ReqShabakaAwardInfo(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    refreshShabakeAwardInfo(roleId);
}

void ShaBaKe::clientmsg_ReqShabakeDailyAwardInfo(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    if(role->factionId() != m_winFaction)
    {
        PublicRaw::RetShabakeDailyAwardInfo ret;
        ret.canGet = false;
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetShabakeDailyAwardInfo), &ret, sizeof(ret));
        return;
    }

    PrivateRaw::FuncReqDailyAwardInfo info;
    auto it = m_shabakePositionRole.find(role->name());
    if(it != m_shabakePositionRole.end())
        info.position = it->second.position;
    info.roleId = roleId;
    role->sendToWorld(RAWMSG_CODE_PRIVATE(FuncReqDailyAwardInfo), &info, sizeof(info));
}

void ShaBaKe::clientmsg_ReqShabakePosition(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    refreshPosition(role);
}

void ShaBaKe::clientmsg_ReqSetShabakePosition(const uint8_t* msgData, uint32_t size, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto kingIt = m_shabakePositionRole.find(role->name());
    if(kingIt == m_shabakePositionRole.end() || kingIt->second.position != ShabakePosition::king)
    {
        LOG_DEBUG("沙巴克, 不是城主无法设置官职, role:{}", role->name());
        return;
    }

    if(!inSetPositionTime())
    {
        role->sendSysChat("已经过了设置官职时间, 不能设置");
        return;
    }

    auto rev = reinterpret_cast<const PublicRaw::ReqSetShabakePosition*>(msgData);
    if(rev->autoflag)
    {
        autoSetPosition(true);
        refreshPosition(role);
        return;
    }
    if(rev->position == ShabakePosition::none || rev->position == ShabakePosition::king)
        return;

    RoleInfo::Ptr roleInfo = RoleInfoManager::me().getRoleInfoById(rev->targetId);
    if(nullptr == roleInfo)
    {
        LOG_DEBUG("沙巴克, RoleInfoManager设置职位时获取不到玩家信息, id:{}", rev->targetId);
        return;
    }
    //把目标玩家原来所在职位剔除
    auto targetIt = m_shabakePositionRole.find(roleInfo->name);
    if(targetIt != m_shabakePositionRole.end())
        m_shabakePositionRole.erase(targetIt);

    //剔除设置职位上的玩家
    for(auto positionIt = m_shabakePositionRole.begin(); positionIt != m_shabakePositionRole.end(); ++positionIt)
    {
        if(positionIt->second.position == rev->position)
        {
            m_shabakePositionRole.erase(positionIt);
            break;
        }
    }

    RolePositionInfo info;
    info.job = roleInfo->job;
    info.sex = roleInfo->sex;
    info.position = rev->position;
    m_shabakePositionRole[roleInfo->name] = info;
    refreshPosition(role);
}

void ShaBaKe::notifyRoleShabakeOver(RoleId roleId, const std::string& king, const std::string& faction)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;
    PublicRaw::NotifyShabakeOver notify;
    std::memset(&notify, 0, sizeof(notify));
    king.copy(notify.king, sizeof(notify.king) - 1);
    faction.copy(notify.factionName, sizeof(notify.factionName) - 1);
    role->sendToMe(RAWMSG_CODE_PUBLIC(NotifyShabakeOver), &notify, sizeof(notify));
}

bool ShaBaKe::inSetPositionTime() const
{
    if(m_endtime + std::chrono::seconds{600} >= Clock::now())
        return true;
    return false;
}

/*
 *
 * 规则: 副帮主 -> 三职业首席 -> 等级高者
 *
 */
void ShaBaKe::autoSetPosition(bool client/*=false*/)
{
    if(m_shabakePositionRole.size() == 0 || m_shabakePositionRole.size() >= 6)
        return;

    if(client)
    {//玩家请求主动自动设置官职
        if(!inSetPositionTime())
            return;
    }
    else
    {
        if(m_autosetFlag)
            return;
        if(inSetPositionTime())
            return;

        m_autosetFlag = true;
    }

    Faction::Ptr faction = FactionManager::me().getById(m_winFaction);
    if(nullptr == faction)
    {
        LOG_DEBUG("沙巴克, 自动设置官职时,找不到帮派, factionId:{}", m_winFaction);
        return;
    }

    LOG_DEBUG("沙巴克, 自动设置官职");
    std::vector<FactionInfoOfRole> factionMembers; //帮派成员信息(已按等级排好序)
    std::set<uint8_t> existPosition;
    for(const auto& positionIt : m_shabakePositionRole)
    {
        existPosition.insert(static_cast<uint8_t>(positionIt.second.position));
    }
    for(uint8_t position = 2; position <= 6; ++position)
    {
        if(existPosition.find(position) == existPosition.end())
        {
            RolePositionInfo info;
            bool filled = false;
            std::unordered_set<RoleId> viceLeaders = faction->viceLeader();
            for(const auto& roleId : viceLeaders)
            {
                RoleInfo::Ptr roleInfo = RoleInfoManager::me().getRoleInfoById(roleId);
                if(nullptr == roleInfo)
                    continue;
                if(m_shabakePositionRole.find(roleInfo->name) == m_shabakePositionRole.end())
                {
                    info.job = roleInfo->job;
                    info.sex = roleInfo->sex;
                    info.position = static_cast<ShabakePosition>(position);
                    m_shabakePositionRole.insert({roleInfo->name, info});
                    filled = true;
                    break;
                }
            }
            if(filled)
                continue;
            //战士首席
            RoleId warriorLeader = faction->warriorLeader();
            if(0 != warriorLeader)
            {
                RoleInfo::Ptr roleInfo = RoleInfoManager::me().getRoleInfoById(warriorLeader);
                if(nullptr != roleInfo)
                {
                    if(m_shabakePositionRole.find(roleInfo->name) == m_shabakePositionRole.end())
                    {
                        info.job = roleInfo->job;
                        info.sex = roleInfo->sex;
                        info.position = static_cast<ShabakePosition>(position);
                        m_shabakePositionRole.insert({roleInfo->name, info});
                        continue;
                    }
                }
            }
            //法师首席
            RoleId magicLeader = faction->magicianLeader();
            if(0 != magicLeader)
            {
                RoleInfo::Ptr roleInfo = RoleInfoManager::me().getRoleInfoById(warriorLeader);
                if(nullptr != roleInfo)
                {
                    if(m_shabakePositionRole.find(roleInfo->name) == m_shabakePositionRole.end())
                    {
                        info.job = roleInfo->job;
                        info.sex = roleInfo->sex;
                        info.position = static_cast<ShabakePosition>(position);
                        m_shabakePositionRole.insert({roleInfo->name, info});
                        continue;
                    }
                }
            }
            //道士首席
            RoleId taoistLeader = faction->taoistLeader();
            if(0 != taoistLeader)
            {
                RoleInfo::Ptr roleInfo = RoleInfoManager::me().getRoleInfoById(warriorLeader);
                if(nullptr != roleInfo)
                {
                    if(m_shabakePositionRole.find(roleInfo->name) == m_shabakePositionRole.end())
                    {
                        info.job = roleInfo->job;
                        info.sex = roleInfo->sex;
                        info.position = static_cast<ShabakePosition>(position);
                        m_shabakePositionRole.insert({roleInfo->name, info});
                        continue;
                    }
                }
            }

            //全名遍历
            if(factionMembers.empty())
            {
                std::unordered_set<RoleId> members = faction->allMembers();
                for(const auto& member : members)
                {
                    FactionInfoOfRole roleInfo = FactionManager::me().getFactionRoleInfo(member);
                    factionMembers.push_back(roleInfo);
                }
                struct roleLevelCmp
                {
                    bool operator() (const FactionInfoOfRole& lhs, const FactionInfoOfRole& rhs) const
                    {
                        return lhs.level >= rhs.level;
                    }
                };
                std::sort(factionMembers.begin(), factionMembers.end(), roleLevelCmp());
            }
            for(const auto& role : factionMembers)
            {
                if(m_shabakePositionRole.find(role.name) == m_shabakePositionRole.end())
                {
                    RoleInfo::Ptr roleInfo = RoleInfoManager::me().getRoleInfoById(role.roleId);
                    if(nullptr == roleInfo)
                        continue;
                    info.job = roleInfo->job;
                    info.sex = roleInfo->sex;
                    info.position = static_cast<ShabakePosition>(position);
                    m_shabakePositionRole.insert({roleInfo->name, info});
                    break;
                }
            }
        }
    }
}

void ShaBaKe::refreshPosition(Role::Ptr role)
{
    using namespace std::chrono;
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PublicRaw::RefreshShabakePosition));
    auto msg = reinterpret_cast<PublicRaw::RefreshShabakePosition*>(buf.data());
    if(inSetPositionTime())
        msg->setLefttime = SAFE_SUB(600, duration_cast<seconds>(Clock::now() - m_endtime).count());
    else
        msg->setLefttime = 0;
    for(const auto& it : m_shabakePositionRole)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RefreshShabakePosition::PositionInfo));
        auto msg = reinterpret_cast<PublicRaw::RefreshShabakePosition*>(buf.data());
        msg->info[msg->size].position = it.second.position;
        msg->info[msg->size].job = it.second.job;
        msg->info[msg->size].sex = it.second.sex;

        std::memset(msg->info[msg->size].name, 0, sizeof(msg->info[msg->size].name));
        it.first.copy(msg->info[msg->size].name, sizeof(msg->info[msg->size].name)-1);
        ++msg->size;
    }

    role->sendToMe(RAWMSG_CODE_PUBLIC(RefreshShabakePosition), buf.data(), buf.size());
}

void ShaBaKe::notifyWorldGiveShabakeAward(Role::Ptr role, bool first, ShabakeAward awardtype)
{
    PrivateRaw::NotifyWorldGiveRoleAward notify;
    notify.roleId = role->id();
    notify.first = first;
    notify.awardtype = awardtype;
    auto it = m_shabakePositionRole.find(role->name());
    if(it == m_shabakePositionRole.end())
        notify.position = ShabakePosition::none;
    else
        notify.position = it->second.position;
    role->sendToWorld(RAWMSG_CODE_PRIVATE(NotifyWorldGiveRoleAward), &notify, sizeof(notify));
}

void ShaBaKe::refreshShabakeAwardInfo(RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;
    auto it = m_parttakeRoleAward.find(role->id());
    if(it == m_parttakeRoleAward.end())
    {
        LOG_DEBUG("沙巴克, 玩家不在参与列表, role:{}", role->name());
        return;
    }

    PublicRaw::RetShabakeAwardInfo ret;
    auto posIt = m_shabakePositionRole.find(role->name());
    if(posIt != m_shabakePositionRole.end() && posIt->second.position == ShabakePosition::king)
    {
        if(0 == (it->second & static_cast<uint8_t>(ShabakeAward::king)))
            ret.kingAward = true;
    }
    if(role->factionId() == m_winFaction && 0 == (it->second & static_cast<uint8_t>(ShabakeAward::win)))
        ret.winAward = true;
    if(0 == (it->second & static_cast<uint8_t>(ShabakeAward::parttake)))
       ret.parttake = true;

    role->sendToMe(RAWMSG_CODE_PUBLIC(RetShabakeAwardInfo), &ret, sizeof(ret));
}

}
