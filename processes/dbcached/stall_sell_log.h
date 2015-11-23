#ifndef PROCESS_DBCACHED_STALL_LOG_MGR_HPP
#define PROCESS_DBCACHED_STALL_LOG_MGR_HPP


#include "water/common/roledef.h"

namespace dbcached{

class StallLogMgr
{
    friend class RoleManager; 
public:
    ~StallLogMgr() = default;
    static StallLogMgr& me();

    void regMsgHandler();

private:
    StallLogMgr() =default;
    void servermsg_SaveStallLog(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_WorldReqStallLog(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);

private:
    std::string getStallLogById(RoleId roleId);
};

}
#endif
