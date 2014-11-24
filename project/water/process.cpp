#include "process.h"

#include "componet/log.h"

#include <iostream>
#include <thread>


namespace water{

Process::Process(ProcessType type, int32_t id, const std::string& configFile)
:m_type(type), m_id(id), m_cfg(configFile)
{
}

void Process::start()
{
    try
    {
        m_switch.store(Switch::on);
        init();

        std::vector<std::thread> threads;

        threads.push_back(std::thread(std::mem_fn(&TcpServer::run), &m_server));
        threads.push_back(std::thread(std::mem_fn(&TcpClient::run), &m_client));
        threads.push_back(std::thread(std::mem_fn(&TcpConnectionManager::run), &m_tcm));
        threads.push_back(std::thread(std::mem_fn(&Process::runMainLoop, this));
        for(auto& th : threads)
            th.join();
    }
    catch (const componet::ExceptionBase& e)
    {
        LOG_ERROR("致命错误, 启动失败: [{}]", e.what());
        terminate();
    }
}

void Process::terminate()
{
    std::cout << "收到退出请求" << std::endl;
    m_switch.store(Switch::off);  
    m_server.stop();
    m_client.stop();
    m_tcm.stop();
}

void Process::init()
{
    m_cfg.load();
    const auto& cfg = m_cfg.getInfo(m_type, m_id);
    for(const net::Endpoint& ep : cfg.listen)
        m_server.addLocalEndpoint(ep);
    for(const net::Endpoint& ep : cfg.connect)
        m_client.addRemoteEndpoint(ep, std::chrono::seconds(5));

    using namespace std::placeholders;
    m_server.e_newConn.reg(std::bind(&TcpConnectionManager::addConnection, &m_tcm, 
                                     _1, TcpConnectionManager::ConnType::in));
    m_client.e_newConn.reg(std::bind(&TcpConnectionManager::addConnection, &m_tcm, 
                                     _1, TcpConnectionManager::ConnType::out));

    m_server.e_onClose.reg(std::bind(&Process::terminate, this));
    m_client.e_onClose.reg(std::bind(&Process::terminate, this));
    m_tcm.e_onClose.reg(std::bind(&Process::terminate, this));
}

void runMainLoop()
{
    while (m_switch.load() == Switch::on)
    {
        //处理消息事件
        net::TcpConnection::Ptr& conn, net::Packet::Ptr* packet
        if(m_tcm.getPacket(&conn, &packet))
        {
            LOG_DEBUG("recv packet: {}", packet->getMsg());
        }


        //处理定时器事件
    }
}


}

