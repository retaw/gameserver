#include "dragon_ball_manager.h"
#include "dragonBall_table_structure.h"
#include "role_table_structure.h"
#include "role_manager.h"

#include "water/componet/logger.h"

#include "protocol/rawmsg/private/dragon_ball.h"
#include "protocol/rawmsg/private/dragon_ball.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include <unordered_map>  

namespace dbcached{

using namespace std::placeholders;

DragonBallManager DragonBallManager::m_me;

DragonBallManager& DragonBallManager::me()
{
    return m_me;
}


void DragonBallManager::regMsgHandler()
{
    REG_RAWMSG_PRIVATE(UpdateOrInsertDragonBallExp, std::bind(&DragonBallManager::servermsg_UpdateOrInsertDragonBallExp, this, _1, _2));
}

void DragonBallManager::servermsg_UpdateOrInsertDragonBallExp(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateOrInsertDragonBallExp*>(msgData);
	if(!rev)
		return;

	if(!updateOrInsertDB(rev->roleId, rev->dragonType, rev->exp))
	{
		LOG_ERROR("龙珠, 更新或插入, DB, 失败, roleId={}, dragonType={}, exp={}",
				  rev->roleId, rev->dragonType, rev->exp); 
		return;
	}

	if(!RoleManager::me().m_contrRoles.updateOrInsertDragonBallExp(rev->roleId, rev->dragonType, rev->exp))
	{
		LOG_TRACE("龙珠, 更新或插入, 缓存, 角色, 失败, roleId={}, dragonType={}, exp={}",
				  rev->roleId, rev->dragonType, rev->exp); 
		return;
	}
	
	LOG_TRACE("龙珠, 更新或插入, DB及缓存, 成功, roleId={}, dragonType={}, exp={}",
			  rev->roleId, rev->dragonType, rev->exp); 
	return;
}


bool DragonBallManager::updateOrInsertDB(RoleId roleId, uint8_t dragonType, uint32_t exp)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfDragonBall dragonBallRow(roleId, dragonType, exp);
        query.replace(dragonBallRow);
		query.execute();
        LOG_DEBUG("龙珠, 更新或插入, DB, 成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_ERROR("龙珠, 更新或插入, DB, 失败, DB error:{}", er.what());
         return false;
    }
}


//获取角色或英雄的龙珠列表
std::vector<DragonBallInfo> DragonBallManager::getDragonBallList(RoleId roleId) const
{
    std::vector<DragonBallInfo> dragonVec;
	dragonVec.clear();
    LOG_DEBUG("龙珠, 获取属性列表, roleId={}", roleId);
    
	try
	{
		mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query(); 
		std::string sql = "select * from dragonBall where roleId=";   
		query << sql << roleId;
		std::vector<RowOfDragonBall> res;
		query.storein(res); 
        if(res.empty())
        {
            LOG_DEBUG("龙珠, 获取属性列表, 列表为空, roleId={}", roleId);
            return dragonVec;
        }
        
		for(auto iter = res.begin(); iter != res.end(); iter++)
        {
			DragonBallInfo temp;
			temp.type = iter->dragonType;
			temp.exp = iter->exp;

			dragonVec.push_back(temp);
        }
        
		return dragonVec;
    }

	catch(const mysqlpp::Exception& er) 
	{
		LOG_DEBUG("龙珠, 获取属性列表, 失败, roleId={}, error={}", roleId, er.what());
	}

	return dragonVec;
}



}
