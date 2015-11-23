#include "role_manager.h"

namespace session{



RoleManager& RoleManager::me()
{
    static RoleManager me;
    return me;
}

void RoleManager::timerExec(const water::componet::TimePoint& now)
{
}


}
