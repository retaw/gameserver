#include "exp_area_manager.h"
#include "expArea_table_structure.h"
#include "role_table_structure.h"
#include "role_manager.h"

#include "water/componet/logger.h"

#include "protocol/rawmsg/private/exp_area.h"
#include "protocol/rawmsg/private/exp_area.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include <unordered_map>  

namespace dbcached{

using namespace std::placeholders;

ExpAreaManager ExpAreaManager::m_me;

ExpAreaManager& ExpAreaManager::me()
{
    return m_me;
}


void ExpAreaManager::regMsgHandler()
{
    REG_RAWMSG_PRIVATE(UpdateOrInsertExpSec, std::bind(&ExpAreaManager::servermsg_UpdateOrInsertExpSec, this, _1, _2));
}

void ExpAreaManager::servermsg_UpdateOrInsertExpSec(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateOrInsertExpSec*>(msgData);
	if(!rev)
		return;

	if(!updateOrInsertDB(rev->roleId, rev->type, rev->sec))
	{
		LOG_ERROR("经验区, 更新或插入, DB, 失败, roleId={}, type={}, sec={}",
				  rev->roleId, rev->type, rev->sec);
		return;
	}

	if(!RoleManager::me().m_contrRoles.updateOrInsertExpAreaSec(rev->roleId, rev->type, rev->sec))
	{
		LOG_TRACE("经验区, 更新或插入, 缓存, 角色, 失败, roleId={}, type={}, sec={}",
				  rev->roleId, rev->type, rev->sec); 
		return;
	}
	
	LOG_TRACE("经验区, 更新或插入, DB及缓存, 成功, roleId={}, type={}, sec={}",
			  rev->roleId, rev->type, rev->sec); 
	return;
}


bool ExpAreaManager::updateOrInsertDB(RoleId roleId, uint8_t type, uint32_t sec)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfExpArea expAreaRow(roleId, type, sec);
        query.replace(expAreaRow);
		query.execute();
        LOG_DEBUG("经验区, 更新或插入, DB, 成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_ERROR("经验区, 更新或插入, DB, 失败, DB error:{}", er.what());
         return false;
    }
}


//获取角色的经验时间列表
std::unordered_map<uint8_t, uint32_t> ExpAreaManager::getExpAreaSecList(RoleId roleId) const
{
    std::unordered_map<uint8_t, uint32_t> expSecMap;
	expSecMap.clear();
    LOG_DEBUG("经验区, 获取时间列表, roleId={}", roleId);
    
	try
	{
		mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query(); 
		std::string sql = "select * from expArea where roleId=";   
		query << sql << roleId;
		std::vector<RowOfExpArea> res;
		query.storein(res); 
        if(res.empty())
        {
            LOG_DEBUG("经验区, 获取时间列表, 列表为空, roleId={}", roleId);
            return expSecMap;
        }
        
		for(auto iter = res.begin(); iter != res.end(); iter++)
        {
			expSecMap.insert(std::make_pair(iter->expType, iter->sec));
		}
        
		return expSecMap;
    }
	catch(const mysqlpp::Exception& er) 
	{
		LOG_DEBUG("经验区, 获取时间列表, 失败, roleId={}, error={}", roleId, er.what());
	}

	return expSecMap;
}

}
