#include "team_manager.h"
#include "role_manager.h"
#include "faction_manager.h"
#include "water/componet/logger.h"
#include "world_boss.h"
#include "water/componet/xmlparse.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/team.codedef.public.h"

namespace func{

TeamManager& TeamManager::me()
{
    static TeamManager me;
    return me;
}

void TeamManager::renewRoleOnlnInteam(Role::Ptr role, const TeamId teamId)
{
    auto it = m_teams.find(teamId);
    if(it == m_teams.end())
    {
        LOG_DEBUG("队员掉线上线, 但是队伍已经解散, roleId = {}, teamId = {}", role->id(), teamId);
                  return;
    }
    it->second->renewRoleOnlnInteam(role);
}

std::vector<Role::Ptr> TeamManager::getTeamMembers(const RoleId roleId)
{
    std::vector<Role::Ptr> ret;
    ret.clear();
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return ret;
    if(role->teamId() == 0)
        return ret;

    auto teamPair = m_teams.find(role->teamId());
    if(teamPair == m_teams.end())
        return ret;
    return teamPair->second->membersVec();
}

void TeamManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(CreateTeam, std::bind(&TeamManager::clientmsg_CreateTeam, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(BreakTeam, std::bind(&TeamManager::clientmsg_BreakTeam, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(LeaveTeam, std::bind(&TeamManager::clientmsg_LeaveTeam, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(KickOutTeam, std::bind(&TeamManager::clientmsg_KickOutTeam, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ChangeCaptain, std::bind(&TeamManager::clientmsg_ChangeCaptain, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ApplyJoinToS, std::bind(&TeamManager::clientmsg_ApplyJoinToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RetApplyJoinToS, std::bind(&TeamManager::clientmsg_RetApplyJoinToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(InviteJoinToS, std::bind(&TeamManager::clientmsg_InviteJoinToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RetInviteJoinToS, std::bind(&TeamManager::clientmsg_RetInviteJoinToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(TeamMembers, std::bind(&TeamManager::clientmsg_TeamMembers, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(NearbyTeams, std::bind(&TeamManager::clientmsg_NearbyTeams, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(FormTeam, std::bind(&TeamManager::clientmsg_FormTeam, this, _1, _2, _3));
}

void TeamManager::loadConfig(const std::string& cfgDir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgDir + "/relation.xml";

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + "parse root node failed");

    m_teamStartLevel = root.getChildNodeText<uint32_t>("teamStartLevel");
}

//消息
void TeamManager::clientmsg_CreateTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = 0;
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("创建队伍请求, 消息错误, msgSize = {}", msgSize);
        return;
    }
    LOG_DEBUG("创建队伍, 收到创建队伍请求, captainId = {}", roleId);
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->level() < m_teamStartLevel)
    {
        role->sendSysChat("你的等级不足, 无法使用组队功能");
        return;
    }
    createTeam(role);
}

void TeamManager::clientmsg_BreakTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = 0;
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("解散队伍请求, 消息错误, msgSize = {}", msgSize);
        return;
    }
    LOG_DEBUG("解散队伍, 收到解散队伍请求, captainId = {}", roleId);
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    breakTeam(role);
}

void TeamManager::clientmsg_LeaveTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = 0;
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("离开队伍请求, 消息错误, msgSize = {}", msgSize);
        return;
    }
    LOG_DEBUG("离开队伍, 收到离开队伍请求, roleId = {}", roleId);
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    leaveTeam(role);
}

void TeamManager::clientmsg_KickOutTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::KickOutTeam);
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("踢出队伍请求, 消息错误, msgSize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::KickOutTeam*>(msgData);
    LOG_DEBUG("踢出队伍, 收到踢出队伍请求, roleId = {}, kickRoleId = {}", 
              roleId, rev->roleId);
    auto role = RoleManager::me().getById(roleId);
    auto kickRole = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    kickOutTeam(role, kickRole);
}

void TeamManager::clientmsg_ChangeCaptain(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::ChangeCaptain);
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("转移队长请求, 消息错误, msgSize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::ChangeCaptain*>(msgData);
    LOG_DEBUG("转移队长, 收到转移队长请求, oldCaptainId = {}, newCaptainId = {}", 
              roleId, rev->newCaptainId);
    auto role = RoleManager::me().getById(roleId);
    auto newCaptain = RoleManager::me().getById(rev->newCaptainId);
    if((role == nullptr) || (newCaptain == nullptr))
        return;
    changeCaptain(role, newCaptain);
}

void TeamManager::clientmsg_ApplyJoinToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::ApplyJoinToS);
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("申请入队请求, 消息错误, msgSize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::ApplyJoinToS*>(msgData);
    LOG_DEBUG("申请入队, 收到申请入队请求, roleId = {}, teamId = {}", 
              roleId, rev->teamId);
    auto role = RoleManager::me().getById(roleId);
    if((role == nullptr))
        return;
    if(role->level() < m_teamStartLevel)
    {
        role->sendSysChat("你的等级不足, 无法使用组队功能");
        return;
    }
    applyJoinToC(role, rev->teamId);
}

void TeamManager::clientmsg_InviteJoinToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::InviteJoinToS);
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("邀请入队请求, 消息错误, msgSize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::InviteJoinToS*>(msgData);
    LOG_DEBUG("邀请入队请求, 收到邀请入队请求, captainId = {}, roleId = {}", 
              roleId, rev->roleId);
    auto captain = RoleManager::me().getById(roleId);
    auto role = RoleManager::me().getById(rev->roleId);
    if((captain == nullptr) || (role == nullptr))
        return;
    if(role->level() < m_teamStartLevel)
    {
        captain->sendSysChat("你邀请的玩家等级不足, 无法使用组队功能");
        return;
    }
    inviteJoinToC(captain, role);
}

void TeamManager::clientmsg_RetApplyJoinToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::RetApplyJoinToS);
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("申请入队回复请求, 消息错误, msgSize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::RetApplyJoinToS*>(msgData);
    LOG_DEBUG("申请入队回复, 收到申请入队回复请求, roleId = {}, acpt = {}", 
              roleId, rev->acpt);
    auto captain = RoleManager::me().getById(roleId);
    auto role = RoleManager::me().getById(rev->roleId);
    if((captain == nullptr) || (role == nullptr))
        return;
    retApplyJoinToC(captain, role, rev->acpt);
}

void TeamManager::clientmsg_RetInviteJoinToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::RetInviteJoinToS);
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("邀请入队回馈, 消息错误, msgSize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::RetInviteJoinToS*>(msgData);
    LOG_DEBUG("邀请入队回馈, 收到邀请回馈, teamId = {}, roleId = {}, acpt = {}", 
              rev->teamId, roleId, rev->acpt);
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    retInviteJoinToC(role, rev->teamId, rev->acpt);
}

void TeamManager::clientmsg_TeamMembers(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = 0;
    if(sizeCount != msgSize)
    {
        LOG_DEBUG("请求我的队伍, 消息错误, msgSize = {}", msgSize);
        return;
    }
    LOG_DEBUG("请求我的队伍, 收到请求, roleId = {}", 
              roleId);
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    retTeamMembers(role);
}

void TeamManager::clientmsg_NearbyTeams(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::NearbyTeams);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("请求附近队伍, 消息错误, msgSize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::NearbyTeams*>(msgData);
    sizeCount += (rev->size) * sizeof(TeamId); 
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("请求附近队伍, 消息错误, msgSize = {}", msgSize);
        return;
    }
    LOG_DEBUG("请求附近队伍, 收到请求, roleId = {}, teamSize = {}", 
              roleId, rev->size);
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    retNearbyTeams(role, rev);
}

void TeamManager::clientmsg_FormTeam(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->level() < m_teamStartLevel)
    {
        role->sendSysChat("你的等级不足, 无法使用组队功能");
        return;
    }
    uint16_t sizeCount = sizeof(PublicRaw::FormTeam);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("最对请求, 消息错误, msgsize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::FormTeam*>(msgData);
    dealFormTeam(roleId, rev->roleId);
}
//业务
void TeamManager::createTeam(Role::Ptr captain)
{
    if(captain == nullptr)
        return;
    if(captain->sceneId() == WorldBoss::me().sceneId())
    {
        captain->sendSysChat("世界boss活动中不能创建队伍");
        return;
    }

    //是否已经存在某个队伍
    if(captain->teamId() != 0)
    {
        LOG_DEBUG("创建队伍, 收到创建者已经属于一个队伍, roleId = {}, teamId ={}",
                  captain->id(), captain->teamId());
        return;
    }
    //创建队伍并插入队伍列表
    auto team = Team::create(captain, m_teamId);
    m_teams[m_teamId] = team;
    m_teamId++;
    //回复客户端
    std::vector<uint8_t> buf;
    uint16_t size = team->fillMembersInfo(buf);
    LOG_DEBUG("创建队伍成功, teamId = {},  向客户端发送队员列表, size = {}", 
              team->teamId(), size);
    captain->sendToMe(RAWMSG_CODE_PUBLIC(RetTeamMembers), buf.data(), buf.size());
    captain->sendSysChat("创建队伍成功");
}

void TeamManager::breakTeam(Role::Ptr captain)
{
    if(captain == nullptr)
        return;
    //是否是队长
    if(!isCaptain(captain))
    {
        LOG_DEBUG("解散队伍, 请求者不是队长, roleId = {}, teamId = {}",
                  captain->id(), captain->teamId());
        return;
    }
    auto it = m_teams.find(captain->teamId());
    std::unordered_map<RoleId, Role::Ptr> memberMap = it->second->getMemberMap();
    //同步worl上的角色队伍信息
    it->second->updateBreakTeamInfoToWorld();
    //队伍内成员的teamId清零
    auto mebers = it->second->getMemberMap();
    for(auto& it : mebers)
        it.second->setTeamId(0);
    m_teams.erase(it);
    //回复客户端，队伍解散
    PublicRaw::RetBreakTeam sendToC;
    for(auto& role : memberMap)
    {
        if(role.second == nullptr)
            continue;
        role.second->sendToMe(RAWMSG_CODE_PUBLIC(RetBreakTeam), (uint8_t*)&sendToC, sizeof(sendToC));
    }
    LOG_DEBUG("解散队伍, 给客户端回复解散消息, memberNum = {}", memberMap.size());
}

void TeamManager::leaveTeam(Role::Ptr role)
{
    if(role == nullptr)
        return;
    //是否是队长
    bool IsCaptain = isCaptain(role);
    //是队长并且不是一个人，即需要更换队长
    if(IsCaptain && (m_teams[role->teamId()]->num() > 1))
    {
        automicChangeCaptain(role); 
        LOG_DEBUG("离开队伍, 角色是队长且不只一个人, 需要更换队长, roleId = {}", role->id());
    }
    //回复客户端，离开一个队员，如果离开的是自己则客户端应该删除队伍
    auto it = m_teams.find(role->teamId());
    if(it == m_teams.end())
        return;
    std::unordered_map<RoleId, Role::Ptr> memberMap = it->second->getMemberMap();
    if(!it->second->eraseMember(role))
        return;
    role->sendSysChat("离开队伍成功");
    PublicRaw::RetLeaveTeam sendToC;
    sendToC.roleId = role->id();
    for(auto& role : memberMap)
    {
        if(role.second == nullptr)
            continue;
        role.second->sendToMe(RAWMSG_CODE_PUBLIC(RetLeaveTeam), (uint8_t*)&sendToC, sizeof(sendToC));
        //role.second->sendSysChat("{}离开了队伍", role.second->name());
    }
    LOG_DEBUG("离开队伍, 给客户端发送离开消息, roleId = {}", role->id());
    if(memberMap.size() == 1)
    {
        m_teams.erase(it);
        LOG_DEBUG("队伍已经没有人, 删除队伍");
    }

}

void TeamManager::kickOutTeam(Role::Ptr role, Role::Ptr kickRole)
{
    if(role == nullptr)
        return;
    if(!isCaptain(role))
    {
        LOG_DEBUG("踢出队伍, 请求者不是队长, roleId = {}", role->id());
        return;
    }
    auto it = m_teams.find(role->teamId());
    std::unordered_map<RoleId, Role::Ptr> memberMap = it->second->getMemberMap();
    if(!it->second->eraseMember(kickRole))
        return;
    PublicRaw::BeKickOutTeam sendToC;
    sendToC.roleId = kickRole->id();
    for(auto& role : memberMap)
    {
        if(role.second == nullptr)
            continue;
        role.second->sendToMe(RAWMSG_CODE_PUBLIC(BeKickOutTeam), (uint8_t*)&sendToC, sizeof(sendToC));
    }
    LOG_DEBUG("踢出队伍, 给客户端发送踢出消息, roleId = {}, kickRoleId = {}", 
              role->id(), kickRole->id());
}

void TeamManager::changeCaptain(Role::Ptr role, Role::Ptr newCaptain)
{
    if(role == nullptr || newCaptain == nullptr)
        return;
    if(!isCaptain(role))
    {
        LOG_DEBUG("转移队长, 请求者不是队长, roleId = {}", role->id());
        return;
    }
    auto it = m_teams.find(role->teamId());
    std::unordered_map<RoleId, Role::Ptr> memberMap = it->second->getMemberMap();
    if(!it->second->setCaptain(newCaptain))
        return;
    PublicRaw::RetChangeCaptain sendToC;
    sendToC.newCaptainId = newCaptain->id();
    for(auto& role : memberMap)
    {
        if(role.second == nullptr)
            continue;
        role.second->sendToMe(RAWMSG_CODE_PUBLIC(RetChangeCaptain), (uint8_t*)&sendToC, sizeof(sendToC));
    }
    LOG_DEBUG("转移队长, 给客户端发送转移队长消息, oldCaptainId = {}, newCaptainId = {}", 
              role->id(), newCaptain->id());
}

void TeamManager::automicChangeCaptain(Role::Ptr role)
{
    if(role == nullptr)
        return;
    if(!isCaptain(role))
    {
        LOG_DEBUG("自动转移队长, 请求者不是队长, roleId = {}", role->id());
        return;
    }
    auto it = m_teams.find(role->teamId());
    if(it == m_teams.end())
        return;
    //找到当前队长迭代器位置,让之后的一位作为队长
    auto newCaptainId = role->id();
    if(!(it->second->autoChangeCaptain(newCaptainId)))
        return;
    std::unordered_map<RoleId, Role::Ptr> memberMap = it->second->getMemberMap();
    PublicRaw::RetChangeCaptain sendToC;
    sendToC.newCaptainId = newCaptainId;
    for(auto& it : memberMap)
    {
        it.second->sendToMe(RAWMSG_CODE_PUBLIC(RetChangeCaptain), (uint8_t*)&sendToC, sizeof(sendToC));
        LOG_DEBUG("自动转移队长, 给客户端发送自动转移队长消息, oldCaptainId = {}, newCaptainId = {}", 
                  it.second->id(), newCaptainId);
    }
}

void TeamManager::applyJoinToC(Role::Ptr role, const TeamId teamId)
{
    if(role == nullptr)
        return;
    auto it = m_teams.find(teamId);
    if(it == m_teams.end())
    {
        LOG_TRACE("TeamManager::applyJoinToC, 无效的teamId, roleId={}, teamId={}", role->id(), teamId); 
        return;
    }
    //role是否已经存在队伍
    if(role->teamId() != 0)
    {
        LOG_DEBUG("申请入队, teamId = {}, roleId = {}",
                  teamId, role->id());
        role->sendSysChat("你已经拥有一个队伍,不能加入其它队伍", ""); 
        return;
    }
    //队伍是否已经满员
    if(it->second->beFull())
    {
        LOG_DEBUG("申请入队, teamId = {}, roleId = {}",
                  teamId, role->id());
        PublicRaw::RetApplyJoinToC sendToC;
        sendToC.status = false;
        sendToC.type = ApplyJoinTeamFailType::full;
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetApplyJoinToC), (uint8_t*)&sendToC, sizeof(sendToC));
        return;
    }
    //找到队长
    if(it == m_teams.end())
        return;
    auto captain = it->second->captain();
    if(captain == nullptr)
        return;
    PublicRaw::ApplyJoinToC sendToC;
    sendToC.roleId = role->id();
    std::memset(sendToC.name, 0, NAME_BUFF_SZIE);
    role->name().copy(sendToC.name, NAME_BUFF_SZIE);
    captain->sendToMe(RAWMSG_CODE_PUBLIC(ApplyJoinToC), (uint8_t*)&sendToC, sizeof(sendToC));
    role->sendSysChat("队伍申请已发送");
    LOG_DEBUG("申请入队, 向客户端发送入队申请, teamId = {}, roleId = {}, name = {}", 
              teamId, role->id(), role->name());
}

void TeamManager::retApplyJoinToC(Role::Ptr captain, Role::Ptr role, const bool acpt)
{
    if(role == nullptr || captain == nullptr)
        return;
    auto it = m_teams.find(captain->teamId());
    //role是否已经存在队伍
    if(role->teamId() != 0)
    {
        LOG_DEBUG("队长回馈申请入队, 申请者已经加入了队伍, captainId = {}, roleId = {}",
                  captain->id(), role->id());
        captain->sendSysChat("{}已经拥有一个队伍,不能加入你的队伍", role->name()); 
        role->sendSysChat("你经拥有一个队伍,不能加入其它队伍", ""); 
        return;
    }
    //给申请人发送反馈
    PublicRaw::RetApplyJoinToC sendToC;
    sendToC.status = true;
    //队长拒绝
    if(!acpt)
    {
        LOG_DEBUG("队长回馈申请入队, 队长拒绝, captainId = {}, roleId = {}",
                  captain->id(), role->id());
        sendToC.status = false;
        sendToC.type = ApplyJoinTeamFailType::refuse;
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetApplyJoinToC), (uint8_t*)&sendToC, sizeof(sendToC));
        return;
    }
    //队伍满员
    if(it->second->beFull())
    {
        LOG_DEBUG("队长回馈申请入队, 队伍已经满员, captainId = {}, roleId = {}",
                  captain->id(), role->id());
        sendToC.status = false;
        sendToC.type = ApplyJoinTeamFailType::full;
        captain->sendSysChat("队伍已经满员, 你的入队批准已经无效", "");
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetApplyJoinToC), (uint8_t*)&sendToC, sizeof(sendToC));
        return;
    }
    //给所有原有队员发送addmember
    std::unordered_map<RoleId, Role::Ptr> memberMap = it->second->getMemberMap();
    PublicRaw::AddMember send;
    {
    send.data.roleId = role->id();
    std::memset(send.data.name, 0, NAME_BUFF_SZIE);
    role->name().copy(send.data.name, NAME_BUFF_SZIE);
    send.data.level = role->level();
    send.data.job = role->job();
    send.data.mapId = role->sceneId();
    std::memset(send.data.factionName, 0, NAME_BUFF_SZIE);
    if(role->factionId() != 0)
    {
        auto factionName = FactionManager::me().getFactionName(role->factionId());
        factionName.copy(send.data.factionName, NAME_BUFF_SZIE);
    }
    send.data.isCaptain = false;
    send.data.vipLevel = 0;
    }
    for(auto& mem : memberMap)
    {
        mem.second->sendToMe(RAWMSG_CODE_PUBLIC(AddMember), (uint8_t*)&send, sizeof(send));
    }
    //入队
    it->second->insertMember(role);
    //给申请人发送成功回馈
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetApplyJoinToC), (uint8_t*)&sendToC, sizeof(sendToC));
    captain->sendSysChat("{}, 加入了你的队伍", role->name());
    LOG_DEBUG("队长回馈申请入队, 入队成功, captainId = {}, roleId = {}",
              captain->id(), role->id());

}

void TeamManager::inviteJoinToC(Role::Ptr captain, Role::Ptr role)
{
    if(role == nullptr || captain == nullptr)
        return;
    auto it = m_teams.find(captain->teamId());
    if(it == m_teams.end())
        return;
    //role是否已经存在队伍
    if(role->teamId() != 0)
    {
        LOG_DEBUG("组队邀请, 对方已经有队伍, captainId = {}, roleId = {}",
                  captain->id(), role->id());
        captain->sendSysChat("{}已经拥有一个队伍,不能对其发出邀请", role->name()); 
        return;
    }
    //队伍是否已经满员
    if(it->second->beFull())
    {
        captain->sendSysChat("你的队伍已经满员,不能在发送邀请", "");
        return;
    }
    captain->sendSysChat("你对{}的组队邀请已经发送", role->name()); 
    PublicRaw::InviteJoinToC sendToC;
    sendToC.teamId = captain->teamId();
    std::memset(sendToC.captainName, 0, NAME_BUFF_SZIE);
    captain->name().copy(sendToC.captainName, NAME_BUFF_SZIE);
    role->sendToMe(RAWMSG_CODE_PUBLIC(InviteJoinToC), (uint8_t*)&sendToC, sizeof(sendToC));
    LOG_DEBUG("组队邀请, 向客户端发送邀请, teamId = {}, captainName = {}",
              sendToC.teamId, sendToC.captainName);
}

void TeamManager::retInviteJoinToC(Role::Ptr role, const TeamId teamId, const bool acpt)
{
    if(role == nullptr)
        return;
    auto it = m_teams.find(teamId);
    auto captain = it->second->captain();
    if(captain == nullptr)
        return;
    //role是否已经存在队伍
    if(role->teamId() != 0)
    {
        LOG_DEBUG("组队邀请回馈, 被邀请者已经加入了其它队伍, captainId = {}, roleId = {}",
                  captain->id(), role->id());
        captain->sendSysChat("{}已经拥有一个队伍,不能加入你的队伍", role->name()); 
        role->sendSysChat("你经拥有一个队伍,不能加入其它队伍", ""); 
        return;
    }
    //被邀请人拒绝
    if(!acpt)
    {
        //给邀请者反馈
        PublicRaw::RetInviteJoinToAcctiveToC sendToC;
        LOG_DEBUG("组队邀请回馈, 被邀请者拒绝, captainId = {}, roleId = {}",
                  captain->id(), role->id());
        sendToC.status = false;
        sendToC.type = ApplyJoinTeamFailType::refuse;
        captain->sendToMe(RAWMSG_CODE_PUBLIC(RetInviteJoinToAcctiveToC), (uint8_t*)&sendToC, sizeof(sendToC));
        return;
    }

    if(role->sceneId() == WorldBoss::me().sceneId())
    {
        role->sendSysChat("世界boss活动中不能加入队伍");
        return;
    }

    //队伍满员
    PublicRaw::RetInviteJoinToPassiveToC sendToC;
    if(it->second->beFull())
    {
        LOG_DEBUG("组队邀请回馈, 队伍已经满员, captainId = {}, roleId = {}",
                  captain->id(), role->id());
        sendToC.status = false;
        sendToC.type = ApplyJoinTeamFailType::full;
        captain->sendSysChat("队伍已经满员, {}已经无法加入你的队伍", role->name());
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetInviteJoinToPassiveToC), (uint8_t*)&sendToC, sizeof(sendToC));
        return;
    }
    //给被邀请者发送成功反馈，让对方拉自己的队伍信息
    sendToC.status = true;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetInviteJoinToPassiveToC), (uint8_t*)&sendToC, sizeof(sendToC));
    //给所有原有队员发送addmember
    std::unordered_map<RoleId, Role::Ptr> memberMap = it->second->getMemberMap();
    PublicRaw::AddMember send;
    {
    send.data.roleId = role->id();
    std::memset(send.data.name, 0, NAME_BUFF_SZIE);
    role->name().copy(send.data.name, NAME_BUFF_SZIE);
    send.data.level = role->level();
    send.data.job = role->job();
    send.data.mapId = role->sceneId();
    std::memset(send.data.factionName, 0, NAME_BUFF_SZIE);
    if(role->factionId() != 0)
    {
        auto factionName = FactionManager::me().getFactionName(role->factionId());
        factionName.copy(send.data.factionName, NAME_BUFF_SZIE);
    }
    send.data.isCaptain = false;
    send.data.vipLevel = 0;
    }
    for(auto& mem : memberMap)
    {
        mem.second->sendToMe(RAWMSG_CODE_PUBLIC(AddMember), (uint8_t*)&send, sizeof(send));
    }
    //入队
    it->second->insertMember(role);
    captain->sendSysChat("{}加入你的队伍", role->name());
    role->sendSysChat("你成功加入队伍", "");
    LOG_DEBUG("邀请入队成功");
    
}

void TeamManager::retTeamMembers(Role::Ptr role)
{
    if(role == nullptr)
        return;
    std::vector<uint8_t> buf;
    if(role->teamId() == 0)
    {
        PublicRaw::RetTeamMembers send;
        send.size = 0;
        LOG_DEBUG("我的队伍请求回馈, 向客户端发送队员列表, size = {}", send.size);
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetTeamMembers), (uint8_t*)&send, sizeof(send));
        return;
    }
    auto it = m_teams.find(role->teamId());
    if(it == m_teams.end())
    {
        LOG_DEBUG("我的队伍请求回馈, 无效的teamId, roleId = {}, teamId = {}",
                  role->id(), role->teamId());
        return;
    }
    auto team = it->second;
    if(team == nullptr)
    {
        LOG_ERROR("我的队伍请求回馈, 队伍id对应的teamId没有找到, roleId = {}, teamId = {}",
                  role->id(), role->teamId());
        return;
    }
    uint16_t size = team->fillMembersInfo(buf);
    LOG_DEBUG("我的队伍请求回馈, 向客户端发送队员列表, size = {}", size);
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetTeamMembers), buf.data(), buf.size());
    
}

void TeamManager::retNearbyTeams(Role::Ptr role, const PublicRaw::NearbyTeams* rev)
{
    if(role == nullptr)
        return;
    std::vector<uint8_t> buf;
    buf.resize(sizeof(PublicRaw::RetNearbyTeams));
    uint16_t i = 0;
    for(i = 0; i < rev->size; i++)
    {
        auto it = m_teams.find(rev->data[i]);
        if(it == m_teams.end())
        {
            LOG_DEBUG("无效的teamId = {}", rev->data[i]);
            continue;
        }
        auto captain = it->second->captain();
        buf.resize(buf.size() + sizeof(PublicRaw::RetNearbyTeams::NearbyTeam));
        auto msg = reinterpret_cast<PublicRaw::RetNearbyTeams*>(buf.data());
        msg->data[i].teamId = it->first;
        msg->data[i].captainJob = captain->job();
        msg->data[i].memNum = it->second->num();
        msg->data[i].captainLevel = captain->level();
        std::memset(msg->data[i].captainName, 0, NAME_BUFF_SZIE);
        captain->name().copy(msg->data[i].captainName, NAME_BUFF_SZIE);
        //队长帮会
        std::memset(msg->data[i].captainFaction, 0, NAME_BUFF_SZIE);
        if(role->factionId() != 0)
        {
            auto factionName = FactionManager::me().getFactionName(role->factionId());
            factionName.copy(msg->data[i].captainFaction, NAME_BUFF_SZIE);
        }
    }
    auto msg = reinterpret_cast<PublicRaw::RetNearbyTeams*>(buf.data());
    msg->size = i;
    LOG_DEBUG("给客户端发送附近队伍, roleIe = {}, size = {}", 
              role->id(), msg->size);    
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetNearbyTeams), buf.data(), buf.size());
}

void TeamManager::dealFormTeam(const RoleId AcRoleId, const RoleId PasRoleId)
{
    auto AcRole = RoleManager::me().getById(AcRoleId);
    auto PasRole = RoleManager::me().getById(PasRoleId);
    if(AcRole == nullptr)
        return;
    if(PasRole == nullptr)
    {
        AcRole->sendSysChat("对方{}已离线", "");
        return;
    }
    if(AcRole->teamId() == 0)   //发起者无队伍
    {
        if(PasRole->teamId() == 0)  //对方也无队伍
        {
             AcRole->sendSysChat("你还没有创建队伍");
             return;
        }
        //对方有队伍，申请入队即可
        applyJoinToC(AcRole, PasRole->teamId());
    }
    else    //发起者有队伍
    {
        if(isCaptain(AcRole))  //是队长，邀请
        {
            inviteJoinToC(AcRole, PasRole);
        }
        else    //是队员，结束
            AcRole->sendSysChat("只有队长才能邀请入队", "");
    }
}

bool TeamManager::isCaptain(Role::Ptr role)
{
    if(role == nullptr)
        return false;
    auto it = m_teams.find(role->teamId());
    if(it == m_teams.end())
    {
        LOG_ERROR("判断是否为队长, 逻辑错误, 没有对应的teamdId, roleId = {}, teamId = {}",
                  role->id(), role->teamId());
        return false;
    }
    if(it->second->captain()->id() == role->id())
        return true;
    return false;

}


}
