/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  game server启动入口
 */

#include "gateway.h"
#include "water/process/shell_arg_parser.h"

#include <csignal>

int main(int argc, char* argv[])
{
    using namespace gateway;

    water::process::ShellArgParser arg(argc, argv);

    //protocol::protobuf::ProtoManager::me().loadConfig(arg.configDir());

    gateway::Gateway::init(arg.num(), arg.configDir(), arg.logDir());
    gateway::Gateway::me().start();
}
