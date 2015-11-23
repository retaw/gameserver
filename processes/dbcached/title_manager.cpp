#include "title_manager.h"
#include "title_table_structure.h"
#include "role_table_structure.h"
#include "role_manager.h"

#include "water/componet/logger.h"
#include "water/componet/datetime.h"
#include "water/componet/string_kit.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace dbcached{

using namespace std::placeholders;

TitleManager TitleManager::m_me;

TitleManager& TitleManager::me()
{
    return m_me;
}


void TitleManager::regMsgHandler()
{
    REG_RAWMSG_PRIVATE(UpdateOrInsertTitle, std::bind(&TitleManager::servermsg_UpdateOrInsertTitle, this, _1, _2));
}

void TitleManager::servermsg_UpdateOrInsertTitle(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateOrInsertTitle*>(msgData);
	if(!rev)
		return;

	for(ArraySize i = 0; i < rev->size; ++i)
    {
		if(updateOrInsertDB(rev->roleId, rev->data[i]))
		{
			if(!RoleManager::me().m_contrRoles.updateOrInsertTitle(rev->roleId, rev->data[i]))
			{
				LOG_TRACE("称号, 更新或插入, 缓存, 失败, roleId={}, titleId={}, titleType={}, createTime={}, disableTime={}, used={}",
						  rev->roleId, rev->data[i].titleId, rev->data[i].titleType, 
						  rev->data[i].createTime, rev->data[i].disableTime, rev->data[i].used);
				continue;
			}
			LOG_TRACE("称号, 更新或插入, DB及缓存, 成功, roleId={}, titleId={}, titleType={}, createTime={}, disableTime={}, used={}",
					  rev->roleId, rev->data[i].titleId, rev->data[i].titleType, 
					  rev->data[i].createTime, rev->data[i].disableTime, rev->data[i].used);
		}
		else
		{
			LOG_ERROR("称号, 更新或插入, DB, 失败, roleId={}, titleId={}, titleType={}, createTime={}, disableTime={}, used={}",
			rev->roleId, rev->data[i].titleId, rev->data[i].titleType, 
			rev->data[i].createTime, rev->data[i].disableTime, rev->data[i].used);
		}
	}

	return;
}


bool TitleManager::updateOrInsertDB(RoleId roleId, const TitleInfo& data)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfTitle titleRow(roleId,data.titleId, (uint8_t)data.titleType, data.createTime, data.disableTime, data.used);
        query.replace(titleRow);
		query.execute();
        LOG_DEBUG("称号, 更新或插入, DB, 成功");
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
         LOG_ERROR("称号, 更新或插入, DB, 失败, DB error:{}", er.what());
         return false;
    }
}


//获取角色的称号列表
std::vector<TitleInfo> TitleManager::getTitleListByRoleId(RoleId roleId) const
{
    std::vector<TitleInfo> titleVec;
    LOG_DEBUG("称号, 获取角色的称号列表, roleId={}", roleId);
    
	try
	{
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        std::string sql = "select * from title where roleId = ";
        query << sql << roleId;
        std::vector<RowOfTitle> res;
        query.storein(res);
        if(res.empty())
        {
            LOG_DEBUG("称号, 获取角色的称号列表, 列表为空, roleId={}", roleId);
            titleVec.clear();
            return titleVec;
        }
        
		for(auto iter = res.begin(); iter != res.end(); iter++)
        {
            TitleInfo temp;
			temp.titleId = iter->titleId;
			temp.titleType = static_cast<TitleType>(iter->titleType);
			temp.createTime = iter->createTime;
			temp.disableTime = iter->disableTime;
			temp.used = iter->used;

			titleVec.emplace_back(temp);
        }
        return titleVec;
    }

	catch(const mysqlpp::Exception& er) 
	{
		LOG_DEBUG("称号, 获取角色的称号列表, 失败, roleId={}, error={}", roleId, er.what());
	}

	return titleVec;
}



}
