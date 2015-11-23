/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-14 14:59 +0800
 *
 * Modified: 2015-04-14 14:59 +0800
 *
 * Description: 
 */

#ifndef PROCESS_WORLD_ROLE_MANAGER_H
#define PROCESS_WORLD_ROLE_MANAGER_H

#include "water/common/role_container.h"

#include "role.h"

namespace world{

class RoleManager : public RoleContainer<Role::Ptr>
{
public:
    RoleManager() = default;
    ~RoleManager() = default;

    void regTimer();

private:
    void timerLoop(StdInterval interval, const componet::TimePoint& now);

public:
    static RoleManager& me();
};

}


#endif

