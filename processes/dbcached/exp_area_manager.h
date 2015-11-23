/*
 * Author: zhupengfei
 *
 * Created: 2015-09-16 15:27 +0800
 *
 * Modified: 2015-09-16 15:27 +0800
 *
 * Description: 经验区
 */

#ifndef PROCESS_DBCACHE_EXP_AREA_MANAGER_HPP
#define PROCESS_DBCACHE_EXP_AREA_MANAGER_HPP

#include "water/common/roledef.h"

#include <unordered_map>

namespace dbcached{

class ExpAreaManager
{
    friend class RoleManager; 
public:
    ~ExpAreaManager() = default;
    static ExpAreaManager& me();
private:
	static ExpAreaManager m_me;  

public:
    void regMsgHandler();

private:
    void servermsg_UpdateOrInsertExpSec(const uint8_t* msgData, uint32_t msgSize);
    bool updateOrInsertDB(RoleId roleId, uint8_t type, uint32_t sec);

    //获取角色的经验时间列表
    std::unordered_map<uint8_t, uint32_t> getExpAreaSecList(RoleId roleId) const;

};

}

#endif

