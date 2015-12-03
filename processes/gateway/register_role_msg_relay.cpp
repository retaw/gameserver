#include "role_manager.h"


#include "protocol/rawmsg/rawmsg_manager.h"

#include "water/componet/logger.h"

namespace gateway{


#define CLIENT_MSG_TO_WORLD(clientMsgName) \
protocol::rawmsg::RawmsgManager::me().regHandler(RAWMSG_CODE_PUBLIC(clientMsgName), std::bind(&RoleManager::relayRoleMsgToWorld, this, RAWMSG_CODE_PUBLIC(clientMsgName), _1, _2, _3));


#define CLIENT_MSG_TO_FUNC(clientMsgName) \
protocol::rawmsg::RawmsgManager::me().regHandler(RAWMSG_CODE_PUBLIC(clientMsgName), std::bind(&RoleManager::relayRoleMsgToFunc, this, RAWMSG_CODE_PUBLIC(clientMsgName), _1, _2, _3));

void RoleManager::regRoleMsgRelay()
{
    using namespace std::placeholders;

    /************转发到world************/
//    CLIENT_MSG_TO_WORLD(RoleMoveToPos);

};

}
