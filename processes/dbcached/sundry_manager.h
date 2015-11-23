#ifndef PROCESS_DBCACHED_SUNDRY_MANAGER_HPP
#define PROCESS_DBCACHED_SUNDRY_MANAGER_HPP


#include "dbcached.h"
#include "water/common/roledef.h"
#include "protocol/rawmsg/private/sundry.h"
#include "protocol/rawmsg/private/sundry.codedef.private.h"

namespace dbcached{

class SundryManager
{
public:
    ~SundryManager() = default;
    static SundryManager& me();

    void regMsgHandler();

    void fillsundry(std::string& sundry, const RoleId roleId);
    void fillTimerSundry(std::string& sundry, const RoleId roleId);

private:
    SundryManager() = default;
    void servermsg_SundryToDB(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_TimerSundryToDB(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);

    void modify(RoleId roleId, std::string& buf);
    void modifyTimer(RoleId roleId, std::string& buf);
};

}
#endif
