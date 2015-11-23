#ifndef PROCESSES_FUNC_ALL_ROLE_INFO_MANAGER_H
#define PROCESSES_FUNC_ALL_ROLE_INFO_MANAGER_H

#include "water/componet/class_helper.h"
#include <unordered_map> 
#include <string>
#include "water/common/roledef.h"
#include "water/common/commdef.h"

namespace func{

struct RoleInfo
{
    TYPEDEF_PTR(RoleInfo)
    CREATE_FUN_NEW(RoleInfo)
    RoleId roleId;
    std::string name;
    uint32_t level;
    Job job;
    Sex sex;
};

class RoleInfoManager
{
public:
    ~RoleInfoManager() = default;
    static RoleInfoManager& me();
    void init();
    void setRoleInfoLevel(RoleId roleId, uint32_t level);

    RoleInfo::Ptr getRoleInfoById(RoleId roleId);
    //直接获取roleInfo，尽量不要单独获得名字或者等级
private:
    std::unordered_map<RoleId, RoleInfo::Ptr> m_roleInfos;

};

}

#endif
