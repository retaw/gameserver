#include "role_manager.h"
#include "object_manager.h"
#include "water/componet/logger.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"
#include "protocol/rawmsg/private/package.codedef.private.h"

#include "object_table_structure.h"
#include <vector>

namespace dbcached{

//public
ObjectManager& ObjectManager::me()
{
    static ObjectManager me;
    return me;
}
void ObjectManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(RetModifyObjData,std::bind(&ObjectManager::servermsg_RetModifyObjData,this,_1,_2,_3));
    REG_RAWMSG_PRIVATE(RequestObjListByPackageType,std::bind(&ObjectManager::servermsg_RequestObjListByPackageType,this,_1,_2,_3));
    //...
}

void ObjectManager::servermsg_RetModifyObjData(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::RetModifyObjData*>(msgData);
    ProcessIdentity pid(remoteProcessId);
    LOG_DEBUG("背包, 收到物品修改请求, remotePid={}, roleId={}",
              pid, rev->roleId);
    ArraySize objListSize = rev->objListSize;
    for(ArraySize i = 0; i < objListSize; i++)
    {
        modify(rev->data[i], rev->roleId);
    }

}

void ObjectManager::servermsg_RequestObjListByPackageType(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::RequestObjListByPackageType*>(msgData);
    LOG_DEBUG("背包, 收到指定背包类型的物品列表请求, roleId={}, packageTye={}", 
              rev->roleId, rev->packageType);
    std::vector<RoleObjData::ObjData> objDataVec;
    try
    {
        objDataVec = getObjListByPackageType(rev->roleId, rev->packageType);
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("背包, 收到指定背包类型的物品列表请求, roleId={}, packageTye={}, DB error:{}", 
				  rev->roleId, rev->packageType, er.what());
        return;
    }
    ProcessIdentity pid(remoteProcessId);
    std::vector<uint8_t> buf;
    buf.resize(sizeof(PrivateRaw::RetObjListByPackageType));
    uint16_t i = 0;
    for(auto& it : objDataVec)
    {
        buf.resize(buf.size() + sizeof(RoleObjData::ObjData));
        auto msg = reinterpret_cast<PrivateRaw::RetObjListByPackageType*>(buf.data());
        msg->data[i] = it;
        i++;
    }
    auto msg = reinterpret_cast<PrivateRaw::RetObjListByPackageType*>(buf.data());
    msg->roleId = rev->roleId;
    msg->packageType = rev->packageType;
    msg->size = objDataVec.size();
    DbCached::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetObjListByPackageType), buf.data(), buf.size());
}

void ObjectManager::modify(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj,RoleId roleId)
{
    switch(modifyObj.modifyType)
    {
    case ModifyType::modify:
        updateOrInsert(modifyObj,roleId);
        break;
    case ModifyType::erase:
        erase(modifyObj,roleId);
        break;
    default:
        LOG_ERROR("背包, 错误的ModifyType,不是insert,erase,update中的一种, roleId={}", roleId);
        break;
    }
}


void ObjectManager::updateOrInsert(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj, RoleId roleId)
{
    if(updateOrInsertObject(modifyObj, roleId))
    {
        if(!RoleManager::me().m_contrRoles.updateOrInsertObject(modifyObj, roleId))
        {
            LOG_ERROR("背包, 更新或插入物品, 缓存, 失败, roleId={}, objId={}, packageType={}, cell={}, tplId={}, item={}, strongLevel={}, luckyLevel={}",
                      roleId, modifyObj.objId, uint16_t(modifyObj.packageType),
                      modifyObj.cell, modifyObj.tplId, modifyObj.item, 
					  modifyObj.strongLevel, modifyObj.luckyLevel);
			return;
		}
        LOG_TRACE("背包, 更新或插入物品, 缓存, 成功, roleId={}, objId={}, packageType={}, cell={}, tplId={}, item={}, strongLevel={}, luckyLevel={}",
                  roleId, modifyObj.objId, uint16_t(modifyObj.packageType),
                  modifyObj.cell, modifyObj.tplId, modifyObj.item, 
				  modifyObj.strongLevel, modifyObj.luckyLevel);
        return;
    }
    LOG_ERROR("背包, 更新或插入物品, DB及缓存, 失败, roleId={}, objId={}, packageType={}, cell={}, tplId={}, item={}, strongLevel={}, luckyLevel={}",
              roleId, modifyObj.objId, uint16_t(modifyObj.packageType),
              modifyObj.cell, modifyObj.tplId, modifyObj.item,
			  modifyObj.strongLevel, modifyObj.luckyLevel);
}

void ObjectManager::erase(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj, RoleId roleId)
{
    if(eraseObject(modifyObj,roleId))
    {
        if(!RoleManager::me().m_contrRoles.eraseObject(modifyObj,roleId))
        {
            LOG_ERROR("背包, 删除物品, 缓存, 失败, roleId={}, objId={}, packageType={}, cell={}, tplId={}, item={}, strongLevel={}, luckyLevel={}",
                      roleId, modifyObj.objId, uint16_t(modifyObj.packageType),
                      modifyObj.cell, modifyObj.tplId, modifyObj.item, 
					  modifyObj.strongLevel, modifyObj.luckyLevel);
			return;
		}
        LOG_TRACE("背包, 删除物品, 缓存, 成功, roleId={}, objId={}, packageType={}, cell={}, tplId={}, item={}, strongLevel={}, luckyLevel={}",
                  roleId, modifyObj.objId, uint16_t(modifyObj.packageType),
                  modifyObj.cell, modifyObj.tplId, modifyObj.item,
				  modifyObj.strongLevel, modifyObj.luckyLevel);
        return;
    }
    LOG_ERROR("背包, 删除物品, DB及缓存, 失败, roleId={}, objId={}, packageType={}, cell={}, tplId={}, item={}, strongLevel={}, luckyLevel={}",
              roleId, modifyObj.objId, uint16_t(modifyObj.packageType),
			  modifyObj.cell, modifyObj.tplId, modifyObj.item, 
			  modifyObj.strongLevel, modifyObj.luckyLevel);
}

std::vector<RoleObjData::ObjData> ObjectManager::getObjListByPackageType(RoleId roleId, PackageType packageType)
{
    std::vector<RoleObjData::ObjData> ret;
    Role::Ptr role = RoleManager::me().m_contrRoles.getById(roleId);
    if(role == nullptr)
    {
        LOG_DEBUG("背包, 缓存中未找到该角色, 获取物品列表失败, roleId ={}",
                  roleId);
        ret = getObjListByPackageTypeFromDB(roleId, packageType);
        return ret;
    }
    else
    {
        for(auto& objData : role->getObjDataByKeyMap())
        {
            if(objData.second.packageType == packageType)
                ret.push_back(objData.second);
        }
        return ret;
    }

}

//基本object表操作

std::vector<RoleObjData::ObjData> ObjectManager::getObjListByPackageTypeFromDB(RoleId roleId, PackageType packageType)
{
    //上层处理异常
    std::vector<RoleObjData::ObjData> ObjDataVec;
    mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
    std::string sql = "select * from object where roleId = ";
    query << sql << roleId << " and packageType = " << uint16_t(packageType);
    std::vector<RowOfObject> res;
    query.storein(res);
    if(res.empty())
    {
        LOG_DEBUG("背包, 获取物品列表，限定背包类型, 列表为空, roleId={}, packageType={}", 
				  roleId, packageType);
        ObjDataVec.clear();
        return ObjDataVec;
    }

    for(auto iter = res.begin(); iter != res.end(); iter++)
    {
        RoleObjData::ObjData objData;
        objData.objId = iter->objId;
        objData.packageType = (PackageType)iter->packageType;
        objData.cell = iter->cell;
        objData.tplId = iter->tplId;
        objData.item = iter->item;
        objData.skillId = iter->skillId;
        objData.bind = Bind(iter->bind);
		objData.strongLevel = iter->strongLevel; 
		objData.luckyLevel = iter->luckyLevel;

        ObjDataVec.emplace_back(objData);
    }
    return ObjDataVec;
}

std::vector<RoleObjData::ObjData> ObjectManager::getObjDataByRoleId(RoleId roleId) const
{
    std::vector<RoleObjData::ObjData> ObjDataVec;
    LOG_DEBUG("背包, 获取物品列表, 限定角色背包集, roleId={}",roleId);
    //异常给上层处理
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select * from object where roleId = ";
        query << sql << roleId
        << " and packageType < " << uint16_t(PackageType::equipOfWarrior);
        std::vector<RowOfObject> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("背包, 获取物品列表, 限定角色背包集, 列表为空, roleId={},", roleId);
            ObjDataVec.clear();
            return ObjDataVec;
        }

        for(auto iter = res.begin(); iter != res.end(); iter++)
        {
            RoleObjData::ObjData objData;
            objData.objId = iter->objId;
            objData.packageType = (PackageType)iter->packageType;
            objData.cell = iter->cell;
            objData.tplId = iter->tplId;
            objData.item = iter->item;
            objData.skillId = iter->skillId;
            objData.bind = Bind(iter->bind);
            objData.sellTime = iter->sellTime;
			objData.strongLevel = iter->strongLevel;
			objData.luckyLevel = iter->luckyLevel;

            ObjDataVec.emplace_back(objData);
        }
        return ObjDataVec;
    }
}

uint64_t ObjectManager::getCurObjIdByRoleId(RoleId roleId) const
{
	try
	{
		mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
		std::string sql = "select * from object where roleId = ";
		query << sql << roleId
		<< " order by objId desc limit 1 ";
		std::vector<RowOfObject> res;
		query.storein(res);
		if(res.empty())
		{
			LOG_TRACE("背包, 获取curObjId, 成功, 物品列表为空, roleId={}, curObjId=0", roleId);
			return 0;
		}

		LOG_TRACE("背包, 获取curObjId, 成功, roleId={}, curObjId={}", roleId, res[0].objId);
		return res[0].objId;
	}
	catch(const mysqlpp::Exception& er)
	{
		LOG_ERROR("背包, 获取curObjId, 错误, roleId={}, error={}", roleId, er.what());
		return 0;
	}
}

//role插入不成功或者没有role，那object自动不会成功，这点不考虑
bool ObjectManager::updateOrInsertObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj,RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfObject objectRow(modifyObj.objId, roleId, (uint8_t)modifyObj.packageType, modifyObj.tplId, modifyObj.item, modifyObj.cell, modifyObj.skillId, (uint8_t)modifyObj.bind, modifyObj.sellTime, modifyObj.strongLevel, modifyObj.luckyLevel);
        query.replace(objectRow);
        query.execute();
        LOG_DEBUG("背包, 更新或插入物品, DB, 成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("背包, 更新或插入物品, DB error:{}", er.what());
        return false;
    }
}

bool ObjectManager::eraseObject(const PrivateRaw::RetModifyObjData::ModifyObj& modifyObj, RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from object where roleId = " << roleId 
        << " and objId = " << modifyObj.objId;
        query.execute();

        LOG_DEBUG("背包, 删除物品, DB, 成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("背包, 删除物品, DB error:{}",er.what());
        return false;
    }

}

std::vector<RoleObjData::ObjData> ObjectManager::getHeroObjData(RoleId roleId, Job job) const
{

    std::vector<RoleObjData::ObjData> ObjDataVec;
    LOG_DEBUG("背包, 获取英雄的物品列表, roleId={}, job={}", roleId, job);
    //异常给上层处理
    {
        PackageType type = getPacTypeByJob(job);
        PackageType stoneType = getStonePacTypeByJob(job);
        if(type == PackageType::none || stoneType == PackageType::none)
            return ObjDataVec;
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select * from object where roleId = ";
        query << sql << roleId 
        << " and (packageType = " << uint16_t(type)
        << " or packageType = " << uint16_t(stoneType)
        << ")";
        std::vector<RowOfObject> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("背包, 获取英雄的物品列表, 列表为空, roleId={}, job={}", roleId, job);
            ObjDataVec.clear();
            return ObjDataVec;
        }

        for(auto iter = res.begin(); iter != res.end(); iter++)
        {
            RoleObjData::ObjData objData;
            objData.objId = iter->objId;
            objData.packageType = (PackageType)iter->packageType;
            objData.cell = iter->cell;
            objData.tplId = iter->tplId;
            objData.item = iter->item;
            objData.skillId = iter->skillId;
            objData.bind = Bind(iter->bind);
            objData.sellTime = iter->sellTime;
			objData.strongLevel = iter->strongLevel;
			objData.luckyLevel = iter->luckyLevel;

            ObjDataVec.emplace_back(objData);
        }
        return ObjDataVec;
    }
}

PackageType ObjectManager::getPacTypeByJob(Job job) const
{
    if(job == Job::warrior)
        return PackageType::equipOfWarrior;
    else if(job == Job::magician)
        return PackageType::equipOfMagician;
    else if(job == Job::taoist)
        return PackageType::equipOfTaoist;
    else
        return PackageType::none;
}

PackageType ObjectManager::getStonePacTypeByJob(Job job) const
{
    if(job == Job::warrior)
        return PackageType::stoneOfWarrior;
    else if(job == Job::magician)
        return PackageType::stoneOfMagician;
    else if(job == Job::taoist)
        return PackageType::stoneOfTaoist;
    else
        return PackageType::none;
}

}
