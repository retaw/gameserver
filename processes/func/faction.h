#ifndef PROCESSES_FUNC_FACTION_H
#define PROCESSES_FUNC_FACTION_H

#include "water/common/factiondef.h"
#include "water/common/commdef.h"
#include "water/componet/class_helper.h"
#include "water/componet/fast_travel_unordered_map.h"
#include "water/componet/fast_travel_unordered_map.h"

#include "protocol/rawmsg/public/faction.h"
#include "protocol/rawmsg/public/faction.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "role_manager.h"
#include "faction_struct.h"
#include <unordered_set>


namespace func{

class FactionManager;

class Faction
{
struct ApplyRecordMemberInfo
{
    std::string name;
    uint32_t time;
};

const uint16_t MAX_APPLY_RECORD_NUM = 200;

public:
    TYPEDEF_PTR(Faction);
    CREATE_FUN_MAKE(Faction);
    Faction(FactionId factionId,
            std::string name,
            uint32_t level = 1,
            uint64_t exp = 0,
            uint64_t resource = 0,
            RoleId leader = 0, 
            RoleId warriorLeader = 0,
            RoleId magicianLeader = 0,
            RoleId taoistLeader = 0,
            std::string notice = "这个帮主很懒,什么都没留下");
    ~Faction() = default;
    void insertViceLeader(std::unordered_set<RoleId>& viceLeaderSet);
public:
    FactionId factionId() const; 
    std::string name() const;
    uint64_t exp() const;
    void setLevel(const uint32_t level);
    uint32_t level() const;
    std::unordered_set<RoleId> allMembers() const;

    void setLeader(const RoleId roleId);
    RoleId leader() const;
    void setViceLeader(const RoleId roleId);
    bool isViceLeader(const RoleId roleId) const;
    std::unordered_set<RoleId> viceLeader() const;
    void eraseViceLeader(const RoleId roleId);
    void setWarriorLeader(const RoleId roleId);
    RoleId warriorLeader() const;
    void setMagicianLeader(const RoleId roleId);
    RoleId magicianLeader() const;
    void setTaoistLeader(const RoleId roleId);
    RoleId taoistLeader() const;

    uint64_t resource() const;


    void setPosition(const RoleId roleId, const FactionPosition position);//调用时保证被设置的职位是可以设置的,考虑角色本身职务，只管设置新职务，不撤销原有职务
    FactionPosition getPositionByRoleId(const RoleId roleId) const;
    bool canAddPosition(const FactionPosition position, const LevelItem& item) const;//某些职务有多个人，所以不能单纯判断是否该职位是否有人

    void donate(const uint64_t exp, const uint64_t resource);

    std::vector<RoleId> getMembers();
    uint32_t memberSize() const;
    uint32_t onlineMemberSize() const;
    void setonlineMemberSize(const uint32_t size);
    void addOnlineNum();
    void subOnlineNum();

    uint32_t applySize() const;

    bool insertMember(const RoleId roleId, const FactionPosition position = FactionPosition::ordinary);
    bool eraseMember(const RoleId roleId);
    void cancelSpecialLeader(const RoleId roleId);//取消除了帮主外的其他领导
    void saveNotice(const std::string& notice);

    bool isLeader(const RoleId roleId) const;
    bool isOtherLeader(const RoleId roleId) const;  //首席
    bool isAppliedByRoleId(const RoleId roleId) const;
    void insertApplyRecord(const RoleId roleId, const std::string& name);
    void eraseApplyRecord(const RoleId roleId);
    bool canDealApply(const RoleId roleId) const;
    bool canInvitejoin(const RoleId roleId) const;
    bool canKickOut(const RoleId roleId) const;
    bool canSaveNotice(const RoleId roleId) const;
    bool existMember(const RoleId roleId) const;
    bool existNewApply() const;

    void fillApplyRcord(std::vector<uint8_t*>& buf) const;
    void fillFactionHall(std::vector<uint8_t*>& buf, LevelItem& item, const FactionInfoOfRole& roleinfo) const;
    void fillFactionMembers(std::vector<uint8_t*>& buf) const;
    void fillRetLog(std::vector<uint8_t*>& buf) const;

    void sendAddMemToall(const PublicRaw::AddFactionMem& send) const;
    void sendSubMemToall(const PublicRaw::SubFactionMem& send) const;
    void sendAppointToAll(const PublicRaw::RetAppointLeader& send) const;

    void sysnLevel();


private:

    void addLog(const LogType type, const uint32_t value, const std::string& name);


private:
    FactionId m_id;
    std::string m_name;
    uint32_t m_level = 1;
    uint64_t m_exp = 0;
    uint64_t m_resource = 0;
    uint32_t m_onlineMemberSize = 0;

    RoleId m_leader;
    std::unordered_set<RoleId> m_viceLeaderSet;
    RoleId m_warriorLeader = 0;
    RoleId m_magicianLeader = 0;
    RoleId m_taoistLeader = 0;
    std::string m_notice;
    
    std::unordered_set<RoleId> m_membersSet;
    std::unordered_map<RoleId, ApplyRecordMemberInfo> m_applyRecord;
    std::list<PublicRaw::FactionLog> m_logList;
};

}

#endif
