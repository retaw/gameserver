/*
 * Author: LiZhaojia
 *
 * Created: 2015-02-26 16:39 +0800
 *
 * Modified: 2015-02-26 16:39 +0800
 *
 * Description:  进程命令行参数解析
 */

//#include "componet/exception.h"

#include <string>


namespace water{
namespace process{

//DEFINE_EXCEPTION(InvalidShellArgs, componet::ExceptionBase)


class ShellArgParser
{
public:
    ShellArgParser(int32_t argc, char* argv[]);
    ~ShellArgParser() = default;

    int32_t num() const;
    const std::string& configDir() const;
    const std::string& logDir() const;

private:
    int32_t m_num = 1;
    std::string m_configDir = "../config";
    std::string m_logDir = "../log";
};


}} //namespace end
