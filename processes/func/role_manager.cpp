#include "role_manager.h"
#include "all_role_info_manager.h"
#include "faction_manager.h"
#include "friend_manager.h"

#include "func.h"
#include "water/componet/logger.h"

#include "protocol/rawmsg/private/login.h"
#include "protocol/rawmsg/private/login.codedef.private.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"

#include "protocol/rawmsg/private/friend.h"
#include "protocol/rawmsg/private/friend.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "team_manager.h"


namespace func{



RoleManager& RoleManager::me()
{
    static RoleManager me;
    return me;
}

void RoleManager::regTimer()
{
    using namespace std::placeholders;
    Func::me().regTimer(std::chrono::milliseconds(50),
                        std::bind(&RoleManager::deleteofflnInTeamRole, this, _1));
}

void RoleManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(RoleOffline, std::bind(&RoleManager::servermsg_RoleOffline, this, _1, _2));
    REG_RAWMSG_PRIVATE(SyncOnlineRoleInfoToFunc, std::bind(&RoleManager::servermsg_SyncOnlineRoleInfoToFunc, this, _1, _2));
    REG_RAWMSG_PRIVATE(SessionSyncAllOnlineRoleInfoToFunc, std::bind(&RoleManager::servermsg_SessionSyncAllOnlineRoleInfoToFunc, this, _1, _2));
    REG_RAWMSG_PRIVATE(UpdateFuncAndSessionRoleLevel, std::bind(&RoleManager::servermsg_UpdateFuncRoleLevel, this, _1, _2));
}

void RoleManager::timerExec(const water::componet::TimePoint& now)
{
    trySyncFromSession();
}

std::vector<Role::Ptr> RoleManager::getSimilarMylevelAndUpOnelevel(uint32_t myLevel, uint32_t oneLevel, RoleId roleId)
{
    std::vector<Role::Ptr> roleVec;
    uint16_t smallLevel;
    for(auto it = begin(); it != end(); it++)
    {
        if((*it)->level() > oneLevel)
        {
            if(roleId == (*it)->id())
                continue;
            //是否在黑名单中
            if(FriendManager::me().isExistBlackList(roleId, (*it)->id()))
                continue;
            if(myLevel < 11)
                smallLevel = 0;
            else
                smallLevel = myLevel - 11;
            if(((*it)->level() > smallLevel) && ((*it)->level() < (myLevel + 11)))
                roleVec.push_back(*it);
        }
    }
    return roleVec;
}

void RoleManager::trySyncFromSession()
{
    if(m_synced)
        return;

    ProcessIdentity pid("session", 1);
    Func::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(FuncQuestSyncAllOnlineRoleInfo));
}

void RoleManager::updateOnlineRoleInfo(const PrivateRaw::SyncOnlineRoleInfoToFunc& roleData)
{
    bool online = false;
    auto role = getById(roleData.id);
    if(role == nullptr)
    {
        role = Role::create(roleData.id, roleData.name, roleData.account, roleData.level, roleData.job);
        if(!insert(role))
        {
            LOG_ERROR("func同步session的role信息, 插入RoleManager失败, role={}", *role);
            return;
        }
        online = true;
    }

    role->setGatewayId(roleData.gatewayId);
    role->setWorldId(roleData.worldId);
	role->setSceneId(roleData.sceneId);

    if(online)
    {
        role->online();
    }
    //跳转场景也需要同步帮派信息
    FactionManager::me().roleOnlineSet(role);

    //查找role是否在offlnInTeamRole中
    auto it = m_offlnInTeamRole.find(roleData.id);
    if(it != m_offlnInTeamRole.end())
    {
        auto teamId = it->second.role->teamId();
        LOG_DEBUG("有队伍中的角色掉线后上线, roleId = {}", roleData.id);
        //把team中的对应role替换为新的role
        TeamManager::me().renewRoleOnlnInteam(role, teamId);
        m_offlnInTeamRole.erase(it);
    }
    LOG_DEBUG("同步session的role信息成功, role={}", *role);
}

void RoleManager::servermsg_RoleOffline(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RoleOffline*>(msgData);
    LOG_DEBUG("角色下线, 收到下线消息, rid={}, loginId={}, type={}", rev->rid, rev->loginId, rev->type);
    auto role = RoleManager::me().getById(rev->rid);
    if(role == nullptr)
        return;

    role->offline();
    //帮派设置
    FactionManager::me().roleOfflineSet(role);
    //把有队伍的队员保存，一分钟后再去掉并离开队伍，这段时间内如果上线则把队伍内的该角色重置
    if(role->teamId() != 0)
    {
        auto now = water::componet::Clock::now();
        m_offlnInTeamRole.insert({role->id(), {role, now}});
        LOG_TRACE("有队员下线, 插入列表, roleId={}, teamId={}", role->id(), role->teamId());
    }
    eraseById(rev->rid);
    
}

void RoleManager::servermsg_SyncOnlineRoleInfoToFunc(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::SyncOnlineRoleInfoToFunc*>(msgData);
    updateOnlineRoleInfo(*rev);
}

void RoleManager::servermsg_SessionSyncAllOnlineRoleInfoToFunc(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::SessionSyncAllOnlineRoleInfoToFunc*>(msgData);
    LOG_DEBUG("批量同步session的role信息, size={}", rev->size);
    for(decltype(rev->size) i = 0; i < rev->size; ++i)
        updateOnlineRoleInfo(rev->roleDataList[i]);

    m_synced = true;
}

void RoleManager::servermsg_UpdateFuncRoleLevel(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateFuncAndSessionRoleLevel*>(msgData);
    LOG_DEBUG("角色级别同步, roleId = {}, level = {}",
              rev->roleId, rev->level);
    auto role = getById(rev->roleId);
    if(role == nullptr)
        return;
    //在线角色级别更新
    role->setLevel(rev->level);
    //friendManager中roleInfo缓存更新
    RoleInfoManager::me().setRoleInfoLevel(rev->roleId, rev->level);
    FactionManager::me().setRoleInfLevel(rev->roleId, rev->level); 
}

void RoleManager::deleteofflnInTeamRole(water::componet::TimePoint now)
{
    std::chrono::duration<int, std::ratio<60>> one_minute(1);
    for(auto it = m_offlnInTeamRole.begin(); it != m_offlnInTeamRole.end(); /**/)
    {
        if((it->second.time + one_minute) < now)
        {
            LOG_TRACE("队员下线超过一分钟, 离开队伍, roleId={}", it->first);
            TeamManager::me().leaveTeam(it->second.role);
            it = m_offlnInTeamRole.erase(it);
        }
        else
            it++;
    }
}

}
