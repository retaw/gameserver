#include "all_role_info_manager.h"
#include "water/componet/logger.h"
#include "role_table_structure.h"
#include "friend_table_structure.h"

namespace func{
using namespace water::dbadaptcher;

RoleInfoManager& RoleInfoManager::me()
{
    static RoleInfoManager me;
    return me;
}

RoleInfo::Ptr RoleInfoManager::getRoleInfoById(RoleId roleId)
{
    auto it = m_roleInfos.find(roleId);
    //缓存中有直接赋值返回
    if(it != m_roleInfos.end())
    {
        return it->second;
    }

    //缓存中没有则库内查询并插入缓存
    std::vector<RowOfRoleInfo> res;
    try
    {
        mysqlpp::Query query = water::dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select name,job,sex,level from roleRarelyUp,roleOftenUp where roleOftenUp.id = roleRarelyUp.id and roleOftenUp.id = " << roleId;
        query.storein(res);
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("func角色缓存从数据库中加载新角色失败, roleId={}", roleId);
        return false;
    }
    if(res.size() == 0)
    {
        LOG_DEBUG("func角色缓存从数据库中加载新角色失败, 没有该角色roleId={}", roleId);
        return false;
    }
    //赋值roleInfo
    RoleInfo::Ptr roleInfo = RoleInfo::create();
    roleInfo->roleId = roleId;
    roleInfo->name = res[0].name;
    roleInfo->level = res[0].level;
    roleInfo->job = Job(res[0].job);
    roleInfo->sex = static_cast<Sex>(res[0].sex);
    //加入缓存
    m_roleInfos[roleId] = roleInfo;
    return roleInfo;
}

void RoleInfoManager::init()
{
    std::vector<RowOfRoleInfo> res;
    try
    {
        mysqlpp::Query query = water::dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select roleRarelyUp.id,name,level,job,sex from roleRarelyUp,roleOftenUp where roleRarelyUp.id=roleOftenUp.id and roleRarelyUp.id > 0";
        query.storein(res);
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("func角色缓存初始化失败");
    }
    for(auto it = res.begin(); it != res.end(); it++)
    {
        RoleInfo::Ptr roleInfo = RoleInfo::create();
        roleInfo->roleId = it->id;
        roleInfo->name = it->name;
        roleInfo->level = it->level;
        roleInfo->job = Job(it->job);
        roleInfo->sex = static_cast<Sex>(it->sex);
        m_roleInfos[it->id] = roleInfo;
    }
}

void RoleInfoManager::setRoleInfoLevel(RoleId roleId, uint32_t level)
{
    auto it = m_roleInfos.find(roleId);
    if(it == m_roleInfos.end())
        return;
    it->second->level = level;
}

}
