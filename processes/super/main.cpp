/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  game server启动入口
 */

#include "super.h"
#include "water/process/shell_arg_parser.h"

#include <csignal>

int main(int argc, char* argv[])
{
    using namespace super;

    water::process::ShellArgParser arg(argc, argv);

    protocol::protobuf::ProtoManager::me().loadConfig(arg.configDir());

    super::Super::init(arg.num(), arg.configDir(), arg.logDir());
    super::Super::me().start();
}
