#ifndef PROCESS_DBCACHED_HORSE_HPP
#define PROCESS_DBCACHED_HORSE_HPP


#include "water/common/roledef.h"

namespace dbcached{

class Horse
{
public:
    ~Horse() = default;
    static Horse& me();

    void regMsgHandler();

private:
    Horse() =default;
    void servermsg_ModifyHorseData(const uint8_t* msgData, uint32_t msgSize);

public:
    std::string load(RoleId roleId);
};

}
#endif
