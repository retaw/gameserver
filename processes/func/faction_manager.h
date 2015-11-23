#ifndef PROCESSES_FUNC_FACTION_MANAGER_H
#define PROCESSES_FUNC_FACTION_MANAGER_H

#include "faction.h"
#include <unordered_set>
#include "protocol/rawmsg/public/faction.h"
#include "protocol/rawmsg/private/faction.h"
#include "faction_struct.h"

namespace func{

DEFINE_EXCEPTION(FactionIdNotExist, componet::ExceptionBase)

class FactionManager
{
public:
    ~FactionManager() = default;

    bool loadConfig(const std::string& cfgDir);

    Faction::Ptr getById(FactionId factionId) const;

    void regMsgHandler();
    static FactionManager& me();
    FactionId getFactionId(RoleId roleId);
    std::string getFactionName(FactionId factionId);
    FactionInfoOfRole getFactionRoleInfo(RoleId roleId);

    void roleOnlineSet(Role::Ptr role);
    void roleOfflineSet(Role::Ptr role);
    void setRoleInfLevel(RoleId roleId, uint32_t level);
    uint32_t getFactionLevel(FactionId factionId) const;

    std::unordered_set<RoleId> getFactionMembers(RoleId roleId) const;
    std::string getFactionLeaderName(FactionId factionId) const;
    RoleId getFactionLeaderRoleId(FactionId factionId) const;

public:
    void loadFaction(); //加载所有的帮派
    void loadRoleInFaction();  //加载所有有帮派的角色的帮会属性

private:
    void fillRoleInfo(RoleId roleId, std::string& name, uint32_t& level, Job& job);
private:
    void clientmsg_ReqFactionHall(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ReqFactionList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_CreateFaction(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ApplyJoinFactionToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ApplyList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_DealApplyRecord(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_CancelApplyRecord(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_AppointLeader(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_InviteJoinFactionToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RetInviteJoinFactionToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_KickOutFromFaction(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_LeaveFaction(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_SaveNotice(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ReqFactionLog(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_FactionLevelUp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_ReqFactionMembers(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    void servermsg_ObjectDonate(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_CreateFaction(const uint8_t* msgData, uint32_t msgSize);
    void servermsg_SynBanggong(const uint8_t* msgData, uint32_t msgSize);

private:
    void factionPanel(const RoleId roleId);
    void insertMember(const FactionId factionId, RoleId roleId);
    void canCreateFaction(const std::string name, const RoleId leader);
    void createFaction(const std::string name, const RoleId leader);
    void eraseMember(Faction::Ptr fac, const RoleId roleId);//删除普通成员
    void breakFaction(Faction::Ptr fac);//删除帮主
    void eraseViceLeader(Faction::Ptr fac, const std::string& viceLeader, const RoleId roleId);
    void eraseEspecialMember(Faction::Ptr fac, const RoleId roleId);//删除除了帮主外的其他领导
    void applyJoinFactionToS(const FactionId factionId, const RoleId roleId);
    void applyList(const RoleId roleId);
    void dealApplyRecord(const RoleId dealerId, const RoleId roleId, const bool accept);
    void cancelApplyRecord(const RoleId roleId, const FactionId factionId);
    void appointLeader(const RoleId leaderId, const RoleId viceLeaderId, FactionPosition position); //此处是任命所有职位，包括取消职位
    void inviteJoinFactionToS(const RoleId leaderId, const RoleId roleId);
    void retInviteJoinFactionToS(const RoleId roleId, const PublicRaw::RetInviteJoinFactionToS* rev);
    void kickOutFromFaction(const RoleId leaderId, const RoleId roleId);
    void leaveFaction(const RoleId roleId);
    void saveNotice(const RoleId roleId, const PublicRaw::SaveNotice* rev);
    void retFactionLog(const RoleId roleId);
    void factionLevelUp(const RoleId roleId);

    uint32_t factionLevelShouldBe(const uint64_t exp, uint32_t nowLevel);

    bool existFactionName(const std::string& name);
    std::string getNameByPosition(const FactionPosition position);//汉语职位名
    std::string toPositionName(const FactionPosition position);//英文名
    
    //DB
    bool insertMemberToDB(const FactionId factionId, const RoleId roleId);
    bool eraseMemberFromDB(const RoleId roleId);
    bool createFactionToDB(const FactionId factionId, const std::string& name, const RoleId leader);

    bool changeViceLeaderToDB(const FactionId factionId, const std::string* viceLeader);//任命和取消都一样，都是整个修改viceLeader字段
    bool appointPositionToDB(const FactionId factionId, const FactionPosition position, const RoleId roleId);
    bool cancelPositionToDB(const FactionId factionId, const FactionPosition position, const RoleId roleId);

    bool breakFactionFromDB(const RoleId roleId, const FactionId factionId);
    bool eraseViceLeaderFromDB(const FactionId factionId, const std::string& viceLeader, const RoleId roleId);
    bool eraseEspecialMemberFromDB(const FactionId factionId, const FactionPosition position, const RoleId roleId);
    bool saveNoticeToDB(const FactionId factionId, const std::string& notice);
    bool factionLevelUpToDB(const FactionId factionId, const uint32_t level);
    bool donateToDB(const uint64_t exp, const uint64_t resource, const uint64_t banggong, const RoleId roleId, const FactionId factionId);
    bool changeBanggongToDB(const uint64_t banggong, const RoleId roleId);

    FactionId createFactionId();
    uint32_t getIdCounter(const FactionId id);
    
    bool canUseFaction(RoleId roleId);
    void setLeaveFactionTime(RoleId roleId);
private:
    FactionManager() = default;

    FactionCfg m_cfg;
    uint32_t m_lastId; //记录最后使用的factionId
    std::unordered_map<RoleId, uint32_t> m_leaveFactionTime;    //离开帮派的时间
    water::componet::FastTravelUnorderedMap<FactionId, Faction::Ptr> m_factions;    //所有的帮会
    std::unordered_set<std::string> m_factionNames; //所有帮派的名字
    std::unordered_map<RoleId, FactionInfoOfRole> m_factionInfoOfRole;
    std::unordered_map<RoleId, std::unordered_set<FactionId>> m_applyRecord; //每个角色的申请记录
    //std::unordered_map<RoleId, std::unordered_set<RoleId>> m_beInvitedRecord;
};

}


#endif
