#ifndef PROCESS_DBCACHED_COUNTER_MANAGER_HPP
#define PROCESS_DBCACHED_COUNTER_MANAGER_HPP


#include "water/common/roledef.h"

namespace dbcached{

class CounterManager
{
    friend class RoleManager; 
public:
    ~CounterManager() = default;
    static CounterManager& me();

    void regMsgHandler();

private:
    CounterManager() =default;
    void servermsg_ModifyCounterInfo(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);

private:
    //基本的表操作
    std::string getCounterInfoByRoleId(RoleId roleId);
};

}
#endif
