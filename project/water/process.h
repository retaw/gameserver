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
#include "process_config.h"
#include "packet_connection_manager.h"

#include "componet/timer.h"
#include "componet/datetime.h"

namespace water{

class Process
{
public:
    Process(ProcessType type, int32_t id, const std::string& configFile, const std::string& processTypeStr);
    virtual ~Process() = default;

    void start();
    void terminate();
    componet::Timer& mainTimer();

private:
    void init();
    void handleMsgByTimer(const componet::TimePoint& now);

protected:
    virtual void packetHandler(PacketConnection::Ptr conn, Packet::Ptr packet, const componet::TimePoint& now);
    void newPrivateInConnection(net::TcpConnection::Ptr tcpConn);
    void newPrivateOutConnection(net::TcpConnection::Ptr tcpConn);
    void newPublicInConnection(net::TcpConnection::Ptr tcpConn);

protected:
    ProcessType m_type;
    int32_t m_id;

    ProcessConfig m_cfg;

    //私网，建立内部连接
    TcpServer::Ptr m_privateNetServer;
    TcpClient::Ptr m_privateNetClient;
    //公网，建立外部连接
    TcpServer::Ptr m_publicNetServer;

    //连接检查器
    //暂空

    //连接管理器，消息接收
    TcpConnectionManager m_conns;

    //主定时器，处理一切业务处理
    componet::Timer m_timer;

    std::unordered_map<ProcessType, std::vector<PacketConnection::Ptr>> m_ProcessConns; 
};

}

#endif

