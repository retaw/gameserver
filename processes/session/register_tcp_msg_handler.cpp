/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-07 15:44 +0800
 *
 * Modified: 2015-03-07 15:44 +0800
 *
 * Description:  统一注册注册消息处理
 */

#include "session.h"

#include "water/componet/logger.h"

#include "roles_and_scenes.h"
#include "scene_dispenser.h"

namespace session{

void Session::registerTcpMsgHandler()
{
    using namespace std::placeholders;
    RolesAndScenes::me().regMsgHandler();
    SceneDispenser::me().regMsgHandler();
}


}
