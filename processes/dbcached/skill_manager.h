#ifndef PROCESS_DBCACHED_SKILL_MANAGER_HPP
#define PROCESS_DBCACHED_SKILL_MANAGER_HPP


#include "dbcached.h"
#include "common/roledef.h"
#include "protocol/rawmsg/private/skill.h"

namespace dbcached{

class SkillManager
{
    friend class RoleManager; 
public:
    ~SkillManager() = default;
    static SkillManager& me();

    void regMsgHandler();

private:
//public://test
    SkillManager() =default;
    void servermsg_ModifySkillData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_SavePKCdStatus(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    

private:
    void erase(const SkillData& modifySkill,RoleId roleId);
    void updateOrInsert(const SkillData& modifySkill,RoleId roleId);

    void eraseHero(const SkillData& modifySkill, RoleId roleId, Job job);
    void updateOrInsertHero(const SkillData& modifySkill, RoleId roleId, Job job);

    //基本的skill表操作
    std::vector<SkillData> getSkillDataByRoleId(RoleId roleId);

    bool updateOrInsertSkill(const SkillData& modifySkill,RoleId roleId);
    bool eraseSkill(const SkillData& modifySkill,RoleId roleId);

    //基本的heroSkill表操作
    std::vector<SkillData> getHeroSkillData(RoleId roleId, Job job);
    bool updateOrInsertHeroSkill(const SkillData& modifySkill, RoleId roleId, Job job);
    bool eraseHeroSkill(const SkillData& modifySkill, RoleId roleId, Job job);

};

}
#endif
