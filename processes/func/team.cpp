#include "team.h"
#include "role_manager.h"
#include "faction_manager.h"
#include "protocol/rawmsg/public/team.h"
#include "protocol/rawmsg/public/team.codedef.public.h"
#include "protocol/rawmsg/private/team.h"
#include "protocol/rawmsg/private/team.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "water/componet/logger.h"
#include "func.h"


namespace func{

Team::Team(Role::Ptr captain, TeamId teamid)
: m_captain(captain), m_teamId(teamid)
{
    insertMember(captain);
}

std::vector<Role::Ptr> Team::membersVec() const
{
    std::vector<Role::Ptr> ret;
    for(auto it : m_member)
        ret.push_back(it.second);
    return ret;
}

Role::Ptr Team::captain() const
{
    return m_captain;
}

const uint16_t Team::num() const
{
    return uint16_t(m_member.size());
}

const TeamId Team::teamId() const
{
    return m_teamId;
}

bool Team::setCaptain(Role::Ptr captain)
{
    if(captain == nullptr)
        return false;
    m_captain = captain;
    return true;
}

bool Team::autoChangeCaptain(RoleId& captainId)
{
    auto oldCaptainIter = m_member.find(captainId);
    if(oldCaptainIter == m_member.end())
        return false;;

    m_member.erase(captainId);
    auto it = m_member.begin();
    for(uint16_t i = 1; i < m_member.size(); i++)
        it++;
    m_captain = it->second;
    captainId = m_captain->id();
    m_member.insert({oldCaptainIter->second->id(), oldCaptainIter->second});

    return true;
}

bool Team::insertMember(Role::Ptr role)
{
   if(role == nullptr)
       return false;
   role->setTeamId(m_teamId);
   updateTeamInfoToWorld(role, true);
   return (m_member.insert({role->id(), role})).second;
}

bool Team::eraseMember(Role::Ptr role)
{
    if(role == nullptr)
       return false;
    m_member.erase(role->id());
    role->setTeamId(0); 
    updateTeamInfoToWorld(role, false);
    return true;
}

bool Team::beFull() const
{
    if(m_member.size() < MAXMEMNUM)
        return false;
    return true;
}

uint16_t Team::fillMembersInfo(std::vector<uint8_t> &buf) const
{
    buf.resize(sizeof(PublicRaw::RetTeamMembers));
    uint16_t i = 0;
    for(auto& mem : m_member)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::TeamMember));
        auto msg = reinterpret_cast<PublicRaw::RetTeamMembers*>(buf.data());
        msg->data[i].roleId = mem.second->id();
        std::memset(msg->data[i].name, 0, NAME_BUFF_SZIE);
        mem.second->name().copy(msg->data[i].name, NAME_BUFF_SZIE);
        msg->data[i].level = mem.second->level();
        msg->data[i].job = mem.second->job();
        msg->data[i].mapId = (uint16_t)mem.second->sceneId();
        std::memset(msg->data[i].factionName, 0, NAME_BUFF_SZIE);
        if(mem.second->factionId() != 0)
        {
            auto factionName = FactionManager::me().getFactionName(mem.second->factionId());
            factionName.copy(msg->data[i].factionName, NAME_BUFF_SZIE);
        }
        msg->data[i].isCaptain = false;
        if(m_captain->id() == mem.second->id())
            msg->data[i].isCaptain = true;
        //msg->data[i].vipLevel = mem.second->vipLevel();
        msg->data[i].vipLevel = 0;
        i++;
    }
    auto msg = reinterpret_cast<PublicRaw::RetTeamMembers*>(buf.data());
    msg->size = m_member.size();
    return msg->size;
}

std::unordered_map<RoleId, Role::Ptr> Team::getMemberMap() const
{
    return m_member;
}

void Team::updateTeamInfoToWorld(Role::Ptr role, bool insertOrErase) const
{
    PrivateRaw::UpdateTeamInfo send;
    send.teamId = m_teamId;
    send.insertOrErase = insertOrErase;
    send.roleId = role->id();
    LOG_DEBUG("同步world队伍信息, roleId={}, teamId={}", 
              send.roleId, send.teamId);
    //发给所有world
    Func::me().broadcastToWorlds(RAWMSG_CODE_PRIVATE(UpdateTeamInfo), &send, sizeof(send));
}

void Team::updateBreakTeamInfoToWorld() const
{
    for(auto& it : m_member)
        updateTeamInfoToWorld(it.second, false);
}

void Team::renewRoleOnlnInteam(Role::Ptr role)
{
    if(role == nullptr)
        return;
    auto it = m_member.find(role->id());
    if(it == m_member.end())
    {
        LOG_DEBUG("队员掉线上线, 队伍中没有该队员, roleId = {}, teamId = {}",
                  role->id(), m_teamId);
        return;
    }
    role->setTeamId(m_teamId);
    updateTeamInfoToWorld(role, true);//world上的role已经下线重新上线，所以需要发送team信息同步
    m_member[role->id()] = role;
}


}
