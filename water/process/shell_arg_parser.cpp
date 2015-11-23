#include "shell_arg_parser.h"

#include "componet/string_kit.h"


namespace water{
namespace process{

ShellArgParser::ShellArgParser(int32_t argc, char* argv[])
{
    if(argc > 1)
        m_num = componet::fromString<int32_t>(argv[1]);
    if(argc > 2)
        m_configDir = std::string(argv[2]);
    if(argc > 3)
        m_logDir = std::string(argv[3]);
}

int32_t ShellArgParser::num() const
{
    return m_num;
}

const std::string& ShellArgParser::configDir() const
{
    return m_configDir;
}

const std::string& ShellArgParser::logDir() const
{
    return m_logDir;
}

}}
