/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  场景逻辑处理服
 */

#include "func.h"
#include "water/process/shell_arg_parser.h"
#include "protocol/protobuf/proto_manager.h"

#include <csignal>

int main(int argc, char* argv[])
{
    using namespace func;

    water::process::ShellArgParser arg(argc, argv);

    protocol::protobuf::ProtoManager::me().loadConfig(arg.configDir());

    func::Func::init(arg.num(), arg.configDir(), arg.logDir());
    func::Func::me().start();
}
