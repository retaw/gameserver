/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-07 15:44 +0800
 *
 * Description:  统一注册注册消息处理
 */

#include "gateway.h"

#include "water/componet/logger.h"

#include "login_processor.h"
#include "role_manager.h"

namespace gateway{

void Gateway::registerTcpMsgHandler()
{
    using namespace std::placeholders;
 
    LoginProcessor::me().regMsgHandler();
    RoleManager::me().regMsgHandler();
}


}
