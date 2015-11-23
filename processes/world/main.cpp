/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  场景逻辑处理服
 */

#include "world.h"
#include "water/process/shell_arg_parser.h"
#include "protocol/protobuf/proto_manager.h"

#include <csignal>

int main(int argc, char* argv[])
{
    using namespace world;

    water::process::ShellArgParser arg(argc, argv);

    protocol::protobuf::ProtoManager::me().loadConfig(arg.configDir());

    world::World::init(arg.num(), arg.configDir(), arg.logDir());
    world::World::me().start();
}
