/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  game server启动入口
 */

#include "http.h"
#include "water/process/shell_arg_parser.h"

int main(int argc, char* argv[])
{
    using namespace http;

    water::process::ShellArgParser arg(argc, argv);
    Http http(arg.num(), arg.configDir(), arg.logDir());
    http.start();
}
