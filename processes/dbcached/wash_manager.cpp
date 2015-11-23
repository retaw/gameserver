#include "wash_manager.h"
#include "wash_table_structure.h"
#include "role_table_structure.h"
#include "role_manager.h"

#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

#include "protocol/rawmsg/private/wash.h"
#include "protocol/rawmsg/private/wash.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include <unordered_map>  

namespace dbcached{

using namespace std::placeholders;

WashManager WashManager::m_me;

WashManager& WashManager::me()
{
    return m_me;
}


void WashManager::regMsgHandler()
{
    REG_RAWMSG_PRIVATE(UpdateOrInsertWashProp, std::bind(&WashManager::servermsg_UpdateOrInsertWashProp, this, _1, _2));
}

void WashManager::servermsg_UpdateOrInsertWashProp(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateOrInsertWashProp*>(msgData);
	if(!rev)
		return;

	std::string propStr;
	std::vector<WashPropInfo> washPropVec;
	for(ArraySize i =0; i < rev->size; ++i)
	{
		propStr += toString(rev->data[i].group);
		propStr += "-";
		propStr += toString(rev->data[i].quality);
		propStr += "-";
		propStr += toString(static_cast<uint16_t>(rev->data[i].propType));
		propStr += "-";
		propStr += toString(rev->data[i].prop);
		propStr += ",";

		washPropVec.push_back(rev->data[i]);
	}

	if(!updateOrInsertDB(rev->roleId, rev->sceneItem, rev->washType, propStr))
	{
		LOG_ERROR("洗练, 更新或插入, DB, 失败, roleId={}, sceneItem={}, washType={}, propStr={}",
				  rev->roleId, rev->sceneItem, rev->washType, propStr); 
		return;
	}

	if(rev->sceneItem == SceneItemType::role)
	{
		if(!RoleManager::me().m_contrRoles.updateOrInsertWashProp(rev->roleId, rev->washType, washPropVec))
		{
			LOG_TRACE("洗练, 更新或插入, 缓存, 角色, 失败, roleId={}, sceneItem={}, washType={}, propStr={}",
					  rev->roleId, rev->sceneItem, rev->washType, propStr); 
			return;
		}
	}
	else if(rev->sceneItem == SceneItemType::hero)
	{
		if(!RoleManager::me().m_contrRoles.updateOrInsertHeroWashProp(rev->roleId, rev->washType, washPropVec))
		{
			LOG_TRACE("洗练, 更新或插入, 缓存, 英雄, 失败, roleId={}, sceneItem={}, washType={}, propStr={}",
					  rev->roleId, rev->sceneItem, rev->washType, propStr); 
			return;
		}
	}
	
	LOG_TRACE("洗练, 更新或插入, DB及缓存, 成功, roleId={}, sceneItem={}, washType={}, propStr={}",
			  rev->roleId, rev->sceneItem, rev->washType, propStr); 
	return;
}


bool WashManager::updateOrInsertDB(RoleId roleId, SceneItemType sceneItem, uint8_t washType, const std::string& propStr)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfWash washRow(roleId, static_cast<uint8_t>(sceneItem), washType, propStr);
        query.replace(washRow);
		query.execute();
        LOG_DEBUG("洗练, 更新或插入, DB, 成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_ERROR("洗练, 更新或插入, DB, 失败, DB error:{}", er.what());
         return false;
    }
}


//获取角色或英雄的洗练列表
std::vector<WashPropInfo> WashManager::getWashPropList(RoleId roleId, SceneItemType sceneItem) const
{
    std::vector<WashPropInfo> propVec;
    LOG_DEBUG("洗练, 获取属性列表, roleId={}, sceneItem={}", roleId, sceneItem);
    
	try
	{
		mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query(); 
		std::string sql = "select * from wash where roleId=";   
		query << sql << roleId << " and sceneItem=" << static_cast<uint16_t>(sceneItem);
		std::vector<RowOfWash> res;
		query.storein(res); 
        if(res.empty())
        {
            LOG_DEBUG("洗练, 获取属性列表, 列表为空, roleId={}, sceneItem={}", roleId, sceneItem);
            propVec.clear();
            return propVec;
        }
        
		for(auto iter = res.begin(); iter != res.end(); iter++)
        {
			std::vector<std::string> strItems = splitString(iter->propStr, ",");  
			for(const std::string& item : strItems)
			{
				std::vector<uint32_t> propItems;
				componet::fromString(&propItems, item, "-");  
				if(propItems.size() != 4)
					continue;

				WashPropInfo temp;
				temp.washType = iter->washType;
				temp.group = propItems[0];
				temp.quality = propItems[1];
				temp.propType = static_cast<PropertyType>(propItems[2]);
				temp.prop = propItems[3];

				propVec.push_back(temp);
			}
        }
        return propVec;
    }

	catch(const mysqlpp::Exception& er) 
	{
		LOG_DEBUG("洗练, 获取属性列表, 失败, roleId={}, sceneItem={}, error={}", 
				  roleId, sceneItem, er.what());
	}

	return propVec;
}



}
