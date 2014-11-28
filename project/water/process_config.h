/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-18 12:10 +0800
 *
 * Description:  进程配置，监听的端点，要连接的端点，可接入的processType
 */

#ifndef WATER_PROCESS_NET_CONFIG_H
#define WATER_PROCESS_NET_CONFIG_H

#include <string>
#include <vector>
#include <set>
#include <map>

#include "componet/exception.h"
#include "net/endpoint.h"
#include "process_id.h"

namespace water{

DEFINE_EXCEPTION(LoadProcessConfigFailed, componet::ExceptionBase)
DEFINE_EXCEPTION(ProcessConfigNotExist, componet::ExceptionBase)

class ProcessConfig final
{
public:

    struct ProcessInfo
    {
        struct
        {
            std::set<net::Endpoint> listen;

            std::set<ProcessIdentity> acceptWhiteList;
            std::set<net::Endpoint> connect;
        } privateNet;

        struct
        {
            std::set<net::Endpoint> listen;
        } publicNet;
    };

    ProcessConfig(const std::string& filePath, ProcessIdentity processId);
    ~ProcessConfig() = default;

    void load();

    const ProcessInfo& getInfo();

private:
    bool parseEndpointListStr(std::set<net::Endpoint>* ret, const std::string& epsStr);
    bool parseProcessList(std::set<ProcessIdentity>* ret, const std::string& epsStr);

private:
    std::vector<net::Endpoint> m_eps;
    ProcessInfo m_processInfo; //下标表示num

    const std::string m_filePath;
    const ProcessIdentity m_processId;
};

}

#endif
