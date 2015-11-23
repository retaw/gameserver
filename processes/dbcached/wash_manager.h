/*
 * Author: zhupengfei
 *
 * Created: 2015-08-07 16:16 +0800
 *
 * Modified: 2015-08-07 16:16 +0800
 *
 * Description: 洗练
 */

#ifndef PROCESS_DBCACHE_WASH_MANAGER_HPP
#define PROCESS_DBCACHE_WASH_MANAGER_HPP

#include "water/common/roledef.h"
#include "processes/world/pkdef.h"

#include <vector>

namespace dbcached{

class WashManager
{
    friend class RoleManager; 
public:
    ~WashManager() = default;
    static WashManager& me();
private:
	static WashManager m_me;  

public:
    void regMsgHandler();

private:
    void servermsg_UpdateOrInsertWashProp(const uint8_t* msgData, uint32_t msgSize);
    bool updateOrInsertDB(RoleId roleId, SceneItemType sceneItem, uint8_t washType, const std::string& propStr);

    //获取角色或英雄的洗练列表
    std::vector<WashPropInfo> getWashPropList(RoleId roleId, SceneItemType sceneItem) const;

};

}

#endif

