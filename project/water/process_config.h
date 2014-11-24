/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-18 12:10 +0800
 *
 * Description:  进程配置，监听的端点，要连接的端点
 */

#ifndef WATER_PROCESS_CONFIG_H
#define WATER_PROCESS_CONFIG_H

#include <string>
#include <vector>
#include <set>
#include <map>

#include "componet/exception.h"
#include "net/endpoint.h"

namespace water{

DEFINE_EXCEPTION(LoadProcessConfigFailed, componet::ExceptionBase)
DEFINE_EXCEPTION(ProcessConfigNotExist, componet::ExceptionBase)

enum class ProcessType : int32_t
{
    router,
    function,
};

class ProcessConfig final
{
public:
    struct NetInfo
    {
        std::set<net::Endpoint> listen;
        std::set<net::Endpoint> connect;
    };

    ProcessConfig(const std::string& filePath);
    ~ProcessConfig() = default;

    bool load();

    const NetInfo& getInfo(ProcessType type, int const id);

private:
    std::map<ProcessType, std::vector<NetInfo>> m_data;
    const std::string m_filePath;
};

}

#endif
