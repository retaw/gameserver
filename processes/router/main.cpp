/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 11:25 +0800
 *
 * Description: router 启动入口
 */

#include "router.h"
#include "water/process/shell_arg_parser.h"

#include <csignal>
#include <iostream>

int main(int argc, char* argv[])
{
    water::process::ShellArgParser arg(argc, argv);

    router::Router::init(arg.num(), arg.configDir(), arg.logDir());

    router::Router::me().start();
}

