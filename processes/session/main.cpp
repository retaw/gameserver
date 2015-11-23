/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  game server启动入口
 */

#include "session.h"
#include "water/process/shell_arg_parser.h"
#include "protocol/protobuf/proto_manager.h"

#include <csignal>

int main(int argc, char* argv[])
{
    using namespace session;

    water::process::ShellArgParser arg(argc, argv);

    protocol::protobuf::ProtoManager::me().loadConfig(arg.configDir());

    session::Session::init(arg.num(), arg.configDir(), arg.logDir());
    session::Session::me().start();
}
