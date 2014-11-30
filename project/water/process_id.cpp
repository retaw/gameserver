#include "process_id.h"

#include "componet/format.h"

#include <map>


namespace water{

std::string processTypeToString(ProcessType type)
{
    static const std::string strs[] =
    {
        "router",
        "function",
        "gateway",
    };

    return strs[static_cast<int32_t>(type)];
}

ProcessType stringToProcessType(std::string& str)
{
    static const std::map<std::string, ProcessType> processTypeStrs = 
    {
        {"router", ProcessType::router}, 
        {"function", ProcessType::function},
        {"gateway", ProcessType::gateway},
    };

    auto it = processTypeStrs.find(str);
    if(it == processTypeStrs.end())
        return ProcessType::none;

    return it->second;
}

ProcessIdentity::ProcessIdentity(ProcessType type_, int32_t num_)
{
    type = type_;
    num = num_;
}

void ProcessIdentity::appendToString(std::string* str) const
{
    str->append(processTypeToString(type));
    str->append(":");
    componet::appendToString(str, num);
}


bool operator==(const ProcessIdentity& pid1, const ProcessIdentity& pid2)
{
    return pid1.value == pid2.value;
}

bool operator<(const ProcessIdentity& pid1, const ProcessIdentity& pid2)
{
    return pid1.value < pid2.value;
}



}
