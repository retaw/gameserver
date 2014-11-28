#include "process_id.h"

#include <map>


namespace water{

std::string ProcessTypeToString(ProcessType type)
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

bool operator==(const ProcessIdentity& pid1, const ProcessIdentity& pid2)
{
    return pid1.value == pid2.value;
}

bool operator<(const ProcessIdentity& pid1, const ProcessIdentity& pid2)
{
    return pid1.value < pid2.value;
}



}
