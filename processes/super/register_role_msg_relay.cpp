#include "zone_manager.h"

#include "water/componet/logger.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/protobuf/proto_manager.h"


// #include "protocol/protobuf/public/client.codedef.h"
// 
// namespace super{
// 
// //using protocol::protobuf::ProtoMsgPtr;
// 
// #define CLIENT_RAW_TO_WORLD(clientMsgName) protocol::rawmsg::RawmsgManager::me().regHandler(RAWMSG_CODE_PUBLIC(clientMsgName), std::bind(&UserManager::relayUserMsgToWorld, this, RAWMSG_CODE_PUBLIC(clientMsgName), _1, _2, _3));
// 
// #define CLIENT_PROTO_TO_WORLD(clientMsgName) protocol::protobuf::ProtoManager::me().regHandler(PROTO_CODE_PUBLIC(clientMsgName), std::bind(&UserManager::relayUserMsgToWorldProto, this, PROTO_CODE_PUBLIC(clientMsgName), _1, _2));
// 
// 
// void ZoneManager::regZoneMsgRelay()
// {
//     using namespace std::placeholders;
// 
//     /************转发到world************/
// //    CLIENT_PROTO_TO_WORLD(C_SetUpName);
// //    CLIENT_PROTO_TO_WORLD(C_MoveTo);
// //    CLIENT_PROTO_TO_WORLD(C_Fission);
// //    CLIENT_PROTO_TO_WORLD(C_Expal);
// };
// 
// }
