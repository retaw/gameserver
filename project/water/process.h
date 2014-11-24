/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-19 21:45 +0800
 *
 * Description: 进程框架
 */

#ifndef WATER_PROCESS_H
#define WATER_PROCESS_H

#include <memory>
#include <atomic>

#include "tcp_server.h"
#include "tcp_client.h"
#include "tcp_connection_manager.h"
#include "process_config.h"
#include "componet/timer.h"

namespace water{

class Process
{
public:
    Process(ProcessType type, int32_t id, const std::string& configFile);
    virtual ~Process() = default;

    void start();
    void terminate();

private:
    void init();
    void runMainLoop();

protected:
    virtual void timeEventHandler(const componet::TimePoint& now);

protected:
    ProcessType m_type;
    int32_t m_id;

    ProcessConfig m_cfg;

    TcpServer m_server;
    TcpClient m_client;
    TcpConnectionManager m_tcm;

    enum class Switch : uint8_t {on, off};
    std::atomic<Switch> m_switch;
};

}

#endif

