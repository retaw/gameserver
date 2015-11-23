#ifndef PROCESS_DBCACHED_BUFF_MANAGER_HPP
#define PROCESS_DBCACHED_BUFF_MANAGER_HPP


#include "dbcached.h"
#include "water/common/roledef.h"
#include "protocol/rawmsg/private/buff.h"

namespace dbcached{

class BuffManager
{
    friend class RoleManager; 
public:
    ~BuffManager() = default;
    static BuffManager& me();

    void regMsgHandler();

private:
//public://test
    BuffManager() = default;
    void servermsg_ModifyBuffData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);

private:
//buff
    //业务
    void updateOrInsert(const BuffData& data, RoleId roleId);
    void erase(const BuffData& data, RoleId roleId);
    //基本的表操作
    std::vector<BuffData> getBuffDataByRoleId(RoleId roleId);
    //bool isExist(const BuffData& data,RoleId roleId);
    bool updateOrInsertBuff(const BuffData& data, RoleId roleId);
    bool eraseBuff(const BuffData& data, RoleId roleId);

//herobuff
    //业务
    void updateOrInsertHero(const BuffData& data, RoleId roleId, Job job);
    void eraseHero(const BuffData& data, RoleId roleId, Job job);
    //基本的表操作
    std::vector<BuffData> getHeroBuffData(RoleId roleId, Job job);
    bool updateOrInsertHeroBuff(const BuffData& data, RoleId roleId, Job job);
    bool eraseHeroBuff(const BuffData& data, RoleId roleId, Job job);

};

}
#endif
