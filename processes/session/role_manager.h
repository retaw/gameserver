/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-16 10:28 +0800
 *
 * Modified: 2015-04-16 10:28 +0800
 *
 * Description:  角色管理器
 */

#ifndef PROCESSES_SESSION_ROLE_MANAGER_H
#define PROCESSES_SESSION_ROLE_MANAGER_H


#include "role.h"
#include "water/common/role_container.h"

namespace session{


class RoleManager : public RoleContainer<Role::Ptr>
{
public:
    ~RoleManager() = default;
    void timerExec(const water::componet::TimePoint& now);

private:
    RoleManager() = default;

public:
    static RoleManager& me();
};


}

#endif
