
#ifndef PROCESS_DBCACHED_OBJECT_MANAGER_H
#define PROCESS_DBCACHED_OBJECT_MANAGER_H
#include "dbcached.h"
#include "common/roledef.h"
#include "water/process/process_id.h" 
#include "protocol/rawmsg/private/package.h"

namespace dbcached{

//物品管理类
class ObjectManager
{
    friend class RoleManager;
public:
    ~ObjectManager() = default;
    static ObjectManager& me();

    void regMsgHandler();

private:
    ObjectManager() =default;
    void servermsg_RetModifyObjData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_RequestObjListByPackageType(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);

private:
    //object业务操作，包括对object表及缓存容器的操作
    void modify(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj, RoleId roleId);
    void erase(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj, RoleId roleId);
    void updateOrInsert(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj, RoleId roleId);
    
    //基本的object表操作
    std::vector<RoleObjData::ObjData> getObjListByPackageType(RoleId roleId, PackageType packageType);
    std::vector<RoleObjData::ObjData> getObjListByPackageTypeFromDB(RoleId roleId, PackageType packageType);
    std::vector<RoleObjData::ObjData> getObjDataByRoleId(RoleId roleId) const;

	uint64_t getCurObjIdByRoleId(RoleId roleId) const;

    bool updateOrInsertObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj, RoleId roleId);
    bool eraseObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj, RoleId roleId);

    //heroObject
    std::vector<RoleObjData::ObjData> getHeroObjData(RoleId roleId, Job job) const;
    
    PackageType getPacTypeByJob(Job job) const;
    PackageType getStonePacTypeByJob(Job job) const;
};

}
#endif
