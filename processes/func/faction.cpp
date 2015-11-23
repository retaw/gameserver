#include "faction_manager.h"
#include "role_manager.h"

#include "water/componet/string_kit.h"
#include "water/componet/logger.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace func{

Faction::Faction(FactionId factionId,
                 std::string name,
                 uint32_t level,
                 uint64_t exp,
                 uint64_t resource,
                 RoleId leader,
                 //副帮主领处理
                 RoleId warriorLeader,
                 RoleId magicianLeader,
                 RoleId taoistLeader,
                 std::string notice)
: m_id(factionId),
  m_name(name),
  m_level(level),
  m_exp(exp),
  m_resource(resource),
  m_leader(leader),
  m_warriorLeader(warriorLeader),
  m_magicianLeader(magicianLeader),
  m_taoistLeader(taoistLeader),
  m_notice(notice)
{
}

void Faction::insertViceLeader(std::unordered_set<RoleId>& viceLeaderSet)
{
    m_viceLeaderSet = viceLeaderSet;
}

void Faction::addLog(const LogType type, const uint32_t value, const std::string& name)
{
    PublicRaw::FactionLog log;
    log.time = time(NULL);
    log.type = type;
    log.value = value;
    std::memset(log.name, 0, NAME_BUFF_SZIE);
    name.copy(log.name, NAME_BUFF_SZIE);
    m_logList.push_back(log);
    if(m_logList.size() > 200)
    m_logList.pop_front();
}

void Faction::setLeader(const RoleId roleId)
{
    m_leader = roleId;
}

void Faction::setViceLeader(const RoleId roleId)
{
    m_viceLeaderSet.insert(roleId);
}

bool Faction::isViceLeader(const RoleId roleId) const
{
    auto it = m_viceLeaderSet.find(roleId);
    if(it == m_viceLeaderSet.end())
        return false;
    return true;
}

void Faction::setWarriorLeader(const RoleId roleId)
{
    m_warriorLeader = roleId;
}

void Faction::setMagicianLeader(const RoleId roleId)
{
    m_magicianLeader = roleId;
}

void Faction::setTaoistLeader(const RoleId roleId)
{
    m_taoistLeader = roleId;
}

void Faction::setPosition(const RoleId roleId, const FactionPosition position)
{
    //调用时保证被设置的职位是可以设置的,即不考虑原职务是否有人，是否已经满员
    //不考虑角色本身职务，只管设置新职务，不撤销原有职务
    switch(position)
    {
    case FactionPosition::leader:
        m_leader = roleId;
        break;
    case FactionPosition::viceLeader:
        m_viceLeaderSet.insert(roleId);
        break;
    case FactionPosition::warriorLeader:
        m_warriorLeader = roleId;
        break;
    case FactionPosition::magicianLeader:
        m_magicianLeader = roleId;
        break;
    case FactionPosition::taoistLeader:
        m_taoistLeader = roleId;
        break;
    case FactionPosition::ordinary:
        break;
    case FactionPosition::none:
        return;
    }
    //不会有其它情况
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    role->setFactionPosition(position);
}

FactionPosition Faction::getPositionByRoleId(const RoleId roleId) const
{
    //是否帮内成员
    if(!existMember(roleId))
        return FactionPosition::none;
    //是否副帮主
    auto it = m_viceLeaderSet.find(roleId);
    if(it != m_viceLeaderSet.end())
        return FactionPosition::viceLeader;
    if(roleId == m_leader)
        return FactionPosition::leader;
    if(roleId == m_warriorLeader)
        return FactionPosition::warriorLeader;
    if(roleId == m_magicianLeader)
        return FactionPosition::magicianLeader;
    if(roleId == m_taoistLeader)
        return FactionPosition::taoistLeader;
    else
        return FactionPosition::ordinary;
}

bool Faction::canAddPosition(const FactionPosition position, const LevelItem& item) const
{
    switch(position)
    {
    case FactionPosition::viceLeader:
        if(m_viceLeaderSet.size() < item.viceLeaderNum)
            return true;
        return false;
    case FactionPosition::warriorLeader:
        return m_warriorLeader == 0;
    case FactionPosition::magicianLeader:
        return m_magicianLeader == 0;
    case FactionPosition::taoistLeader:
        return m_taoistLeader == 0;
    case FactionPosition::leader:
        return true;
    case FactionPosition::ordinary:
        return true;
    case FactionPosition::none:
        return false;
    }
    //不存在其它情况
    return true;
}

void Faction::donate(const uint64_t exp, const uint64_t resource)
{
    m_exp = exp;
    m_resource = resource;
}

FactionId Faction::factionId() const
{
    return m_id;
}

uint64_t Faction::exp() const
{
    return m_exp;
}

void Faction::setLevel(const uint32_t level)
{
    m_level = level;
}

uint32_t Faction::level() const
{
    return m_level;
}

std::unordered_set<RoleId> Faction::allMembers() const
{
    return m_membersSet;
}

RoleId Faction::leader() const
{
    return m_leader;
}

std::unordered_set<RoleId> Faction::viceLeader() const
{
    return m_viceLeaderSet;
}

void Faction::eraseViceLeader(const RoleId roleId)
{
    m_viceLeaderSet.erase(roleId);
}

RoleId Faction::warriorLeader() const
{
    return m_warriorLeader;
}

RoleId Faction::magicianLeader() const
{
    return m_magicianLeader;
}

RoleId Faction::taoistLeader() const
{
    return m_taoistLeader;
}

uint64_t Faction::resource() const
{
    return m_resource;
}

std::vector<RoleId> Faction::getMembers()
{
    std::vector<RoleId> ret(m_membersSet.begin(), m_membersSet.end());
    return ret;
}

std::string Faction::name() const
{
    return m_name;
}

uint32_t Faction::memberSize() const
{
    return  m_membersSet.size(); 
}

uint32_t Faction::onlineMemberSize() const
{
    return m_onlineMemberSize;
}

void Faction::setonlineMemberSize(const uint32_t size)
{
    m_onlineMemberSize = size;
}

void Faction::addOnlineNum()
{
    m_onlineMemberSize++;
}

void Faction::subOnlineNum()
{
    m_onlineMemberSize--;
}

uint32_t Faction::applySize() const
{
    return m_applyRecord.size();
}

bool Faction::insertMember(const RoleId roleId, const FactionPosition position)
{
    auto ret = m_membersSet.insert(roleId);
    if(ret.second == false)
        return false;
    //添加帮派日志
    auto info = FactionManager::me().getFactionRoleInfo(roleId);
    addLog(LogType::addMember, 0, info.name);
    //role设置factionId
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)//添加的角色不在线
        return true;
    role->setFactionPositionNotSysnc(position);
    role->setFactionId(factionId());
    return true;
}

bool Faction::eraseMember(const RoleId roleId)
{
    m_membersSet.erase(roleId);
    //删除其他职务
    cancelSpecialLeader(roleId);
    //添加帮派日志
    auto info = FactionManager::me().getFactionRoleInfo(roleId);
    addLog(LogType::subMember, 0, info.name);
    //role设置factionId
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return true;
    role->setFactionPositionNotSysnc(FactionPosition::none);
    role->setFactionId(0);
    //减少帮派在线人数
    subOnlineNum();
    return true;
}

void Faction::cancelSpecialLeader(const RoleId roleId)
{
    m_viceLeaderSet.erase(roleId);
    if(m_warriorLeader == roleId)
        m_warriorLeader = 0;
    if(m_magicianLeader == roleId)
        m_magicianLeader = 0;
    if(m_taoistLeader == roleId)
        m_taoistLeader = 0;
}

void Faction::saveNotice(const std::string& notice)
{
    m_notice = notice;
}

bool Faction::isLeader(const RoleId roleId) const
{
    return roleId == m_leader;
}

bool Faction::isOtherLeader(const RoleId roleId) const
{
    return (roleId == m_warriorLeader) || (roleId == m_magicianLeader) || (roleId == m_taoistLeader);
}

bool Faction::isAppliedByRoleId(const RoleId roleId) const
{
    auto it = m_applyRecord.find(roleId);
    if(it == m_applyRecord.end())
        return false;
    return true;
}

void Faction::insertApplyRecord(const RoleId roleId, const std::string& name)
{
    ApplyRecordMemberInfo record;
    record.name = name;
    record.time = time(NULL); 
    
    m_applyRecord.erase(roleId);

    if(uint16_t(m_applyRecord.size()) > (MAX_APPLY_RECORD_NUM - uint16_t(1)))
        m_applyRecord.erase(m_applyRecord.begin());
    m_applyRecord.insert({roleId, record});
}

void Faction::eraseApplyRecord(const RoleId roleId)
{
    m_applyRecord.erase(roleId);
}

bool Faction::canDealApply(const RoleId roleId) const
{
    auto it = m_viceLeaderSet.find(roleId);
    if(it != m_viceLeaderSet.end())
        return true;
    return (roleId == m_leader);
}

bool Faction::canInvitejoin(const RoleId roleId) const
{
    return canDealApply(roleId);
}

bool Faction::canKickOut(const RoleId roleId) const
{
    auto it = m_membersSet.find(roleId);
    if(it != m_membersSet.end())
        return true;
    return (roleId == m_leader);
}

bool Faction::canSaveNotice(const RoleId roleId) const
{
    return canKickOut(roleId);
}

void Faction::fillApplyRcord(std::vector<uint8_t*>& buf) const
{
    buf.resize(sizeof(PublicRaw::RetApplyList));
    auto msg = reinterpret_cast<PublicRaw::RetApplyList*>(buf.data());
    msg->size = m_applyRecord.size();
    uint16_t i = 0;
    for(auto iter = m_applyRecord.begin(); iter != m_applyRecord.end(); iter++)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetApplyList::Member));
        auto msg = reinterpret_cast<PublicRaw::RetApplyList*>(buf.data());
        msg->member[i].roleId = iter->first;
        msg->member[i].time = iter->second.time;
        iter->second.name.copy(msg->member[i].name, NAME_BUFF_SZIE);
        i++;
    }
    LOG_DEBUG("帮派列表申请, size={}", msg->size);
}

void Faction::fillFactionHall(std::vector<uint8_t*>& buf, LevelItem& item, const FactionInfoOfRole& roleinfo) const
{
    buf.resize(sizeof(PublicRaw::RetFactionHall));
    auto msg = reinterpret_cast<PublicRaw::RetFactionHall*>(buf.data());
    std::memset(msg->notice, 0, FACTION_NOTICE_SIZE);
    m_notice.copy(msg->notice, FACTION_NOTICE_SIZE);
    msg->level = m_level;
    msg->exp = m_exp;
    if(item.exp == 0)
        msg->addLevelexp = m_exp;
    else
        msg->addLevelexp = item.exp;
    std::memset(msg->name, 0, NAME_BUFF_SZIE);
    m_name.copy(msg->name, NAME_BUFF_SZIE);
    msg->resource = m_resource;
    msg->banggong = roleinfo.banggong;
    msg->maxMemberNum = item.memberNum;
    msg->memberNum = m_membersSet.size();
}

void Faction::fillFactionMembers(std::vector<uint8_t*>& buf) const
{
    buf.resize(sizeof(PublicRaw::RetFactionMembers));
    auto msg = reinterpret_cast<PublicRaw::RetFactionMembers*>(buf.data());

    msg->size = m_membersSet.size();
    uint16_t i = 0;
    for(auto& id : m_membersSet)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::FactionMember));
        msg = reinterpret_cast<PublicRaw::RetFactionMembers*>(buf.data());
        msg->members[i].roleId = id;
        msg->members[i].position = getPositionByRoleId(id);
        std::memset(msg->members[i].name, 0, NAME_BUFF_SZIE);
        auto factionRoleInfo = FactionManager::me().getFactionRoleInfo(id);
        factionRoleInfo.name.copy(msg->members[i].name, NAME_BUFF_SZIE);
        msg->members[i].level = factionRoleInfo.level;
        msg->members[i].job = factionRoleInfo.job;
        msg->members[i].banggong = factionRoleInfo.banggong;
        if(factionRoleInfo.offlnTime == 0)
            msg->members[i].offlnTime = 0;
        else
            msg->members[i].offlnTime = time(NULL) - factionRoleInfo.offlnTime;

        i++;
    }
}

void Faction::fillRetLog(std::vector<uint8_t*>& buf) const
{
    buf.resize(sizeof(PublicRaw::RetFactionLog));
    auto msg = reinterpret_cast<PublicRaw::RetFactionLog*>(buf.data());
    msg->size = m_logList.size();

    uint16_t i = 0;
    for(auto& it : m_logList)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::FactionLog));
        msg = reinterpret_cast<PublicRaw::RetFactionLog*>(buf.data());
        msg->data[i] = it;
        i++;
    }
    LOG_DEBUG("帮派日志请求, 发送给客户端, size={}", msg->size);
}

void Faction::sendAddMemToall(const PublicRaw::AddFactionMem& send) const
{
    for(auto& id : m_membersSet)
    {
        auto role = RoleManager::me().getById(id);
        if(role == nullptr)
            continue;

        role->sendToMe(RAWMSG_CODE_PUBLIC(AddFactionMem), (uint8_t*)&send, sizeof(PublicRaw::AddFactionMem));
    }
}

void Faction::sendSubMemToall(const PublicRaw::SubFactionMem& send) const
{
    for(auto& id : m_membersSet)
    {
        auto role = RoleManager::me().getById(id);
        if(role == nullptr)
            continue;

        role->sendToMe(RAWMSG_CODE_PUBLIC(SubFactionMem), (uint8_t*)&send, sizeof(PublicRaw::SubFactionMem));
    }
}

void Faction::sendAppointToAll(const PublicRaw::RetAppointLeader& send) const
{
    for(auto& id : m_membersSet)
    {
        auto role = RoleManager::me().getById(id);
        if(role == nullptr)
            continue;

        role->sendToMe(RAWMSG_CODE_PUBLIC(RetAppointLeader), (uint8_t*)&send, sizeof(PublicRaw::RetAppointLeader));
    }
}

bool Faction::existNewApply() const
{
    return !m_applyRecord.empty();
}

bool Faction::existMember(const RoleId roleId) const 
{
    auto it = m_membersSet.find(roleId);
    if(it == m_membersSet.end())
        return false;
    return true;
}

void Faction::sysnLevel()
{
    for(auto& id : m_membersSet)
    {
        auto role = RoleManager::me().getById(id);
        if(role == nullptr)
            continue;
        role->sysnFactionLevel(m_level);
    }
}

}
