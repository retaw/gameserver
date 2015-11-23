/*
 * Author: zhupengfei
 *
 * Created: 2015-08-26 10:03 +0800
 *
 * Modified: 2015-08-26 10:03 +0800
 *
 * Description: 龙珠
 */

#ifndef PROCESS_DBCACHE_DRAGON_BALL_MANAGER_HPP
#define PROCESS_DBCACHE_DRAGON_BALL_MANAGER_HPP

#include "water/common/roledef.h"

#include <vector>

namespace dbcached{

class DragonBallManager
{
    friend class RoleManager; 
public:
    ~DragonBallManager() = default;
    static DragonBallManager& me();
private:
	static DragonBallManager m_me;  

public:
    void regMsgHandler();

private:
    void servermsg_UpdateOrInsertDragonBallExp(const uint8_t* msgData, uint32_t msgSize);
    bool updateOrInsertDB(RoleId roleId, uint8_t dragonType, uint32_t exp);

    //获取角色或英雄的洗练列表
    std::vector<DragonBallInfo> getDragonBallList(RoleId roleId) const;

};

}

#endif

