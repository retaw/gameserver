/*
 * Author: LiZhaojia - dantezhu@vip.qq.com
 *
 * Last modified: 2014-08-06 15:11 +0800
 *
 * Description: server的骨架
 */

#ifndef WATER_SERVERS_SERVER_H
#define WATER_SERVERS_SERVER_H

#include "componet/componet.h"
#include "net/net.h"

namespace water
{

enum class ProcessType
{
    CLIENT = 0,
    GATEWAY = 1,
    ROUTER = 2,
};

class ConnectionManager
{
public:
    enum class  
    {
        STOP, RUNNING, SUSPEND
    };

    ConnectionManager(ConnectionIdentity)

    void mainLoop();

private:
    std::map<uint32_t, TcpConnection::Ptr>
    std::vector<TcpConnection::Ptr> vec;
};


class Server
{
    enum class ServerStatus { ON, OFF };
public:
    Server(ProcessType type)
    : m_type(type)
    {
    }

    Server(const Server&) = delete;
    Server& operator = (const Server&) = delete;

    void start()
    {
        //读取配置
        loadConfig();

        m_threads.emplace_back(std::mem_fn(&Server::mainLoop, this));

        for(auto& thread : m_threads)
            m_threads.join();
    }

    virtual void mainLoop()
    {
        
    }
private:
    virtual void loadConfig()
    {
    }

private:
    ServerSwitch m_status ServerStatus::OFF;
    TcpConnector::Ptr m_connector;
    ConnectionGroup m_connectionGroup;

    ConfigData
    {
        struct RemoteListener
        {
            Endpoint remoteEndpoint;
        };
        std::vector<RemoteServer> m_remoteListeners;
    } m_Cfg;

    std::vector<std::thread> threads;
};

}

#endif
