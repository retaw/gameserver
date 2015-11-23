/*
 * Author: zhupengfei
 *
 * Created: 2015-07-27 14:53 +0800
 *
 * Modified: 2015-07-27 14:53 +0800
 *
 * Description: 称号
 */

#ifndef PROCESS_DBCACHE_TITLE_MANAGER_HPP
#define PROCESS_DBCACHE_TITLE_MANAGER_HPP

#include "water/common/roledef.h"

#include "protocol/rawmsg/private/title.h"
#include "protocol/rawmsg/private/title.codedef.private.h"

#include <vector>

namespace dbcached{

class TitleManager
{
    friend class RoleManager; 
public:
    ~TitleManager() = default;
    static TitleManager& me();
private:
	static TitleManager m_me;  

public:
    void regMsgHandler();

private:
    void servermsg_UpdateOrInsertTitle(const uint8_t* msgData, uint32_t msgSize);
    bool updateOrInsertDB(RoleId roleId, const TitleInfo& data);

    //获取角色的称号列表
    std::vector<TitleInfo> getTitleListByRoleId(RoleId roleId) const;

};

}

#endif

