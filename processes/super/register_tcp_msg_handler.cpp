/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-07 15:44 +0800
 *
 * Description:  统一注册注册消息处理
 */

#include "super.h"

#include "water/componet/logger.h"

#include "zone_manager.h"
#include "account_manager.h"

namespace super{

void Super::registerTcpMsgHandler()
{
    using namespace std::placeholders;
 
    ZoneManager::me().regMsgHandler();
    AccountManager::me().regMsgHandler();
}


}
