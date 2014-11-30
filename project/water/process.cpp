#include "process.h"

#include "componet/log.h"
#include "msg/msg.h"

#include <iostream>
#include <thread>


namespace water{

Process::Process(ProcessType type, int32_t num, const std::string& configFile)
: m_processId.type(type), m_processId.num(num), m_cfg(m_processId)
{
}

void Process::start()
{
    try
    {
        init();

        std::vector<std::thread> threads;

        threads.push_back(std::thread(std::mem_fn(&TcpServer::run), m_server));
        threads.push_back(std::thread(std::mem_fn(&TcpClient::run), m_client));
        threads.push_back(std::thread(std::mem_fn(&PacketConnectionManager::run), m_conns));
        threads.push_back(std::thread(std::mem_fn(&componet::Timer::run), &m_timer));
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
    m_server.stop();
    m_client.stop();
    m_conns.stop();
    m_timer.stop();
}

void Process::init()
{
    {//配置解析
        m_cfg.load();
        const ProcessConfig::ProcessInfo& cfg = m_cfg.getInfo();

        const auto& privateNet = cfg.privateNet;
        //私网监听
        if(!privateNet.listen.empty())
        {
            m_privateNetServer = TcpServer::create();
            for(const net::Endpoint& ep : privateNet.listen)
                m_privateNetServer->addLocalEndpoint(ep);
        }
        //私网接入过滤
        m_connChecker.setPrivateWhiteList(cfg.privateNet.acceptWhiteList);
        //私网连出
        if(!privateNet.connect.empty())
        {
            m_privateNetClient = TcpClient::create();
            for(const net::Endpoint& ep : privateNet.connect)
                m_privateNetClient->addLocalEndpoint(ep);
        }

        const auto& publicNet& = cfg.publicNet;
        //公网监听
        if(!publicNet.listen.empty())
        {
            for(const net::Endpoint& ep : cfg.connect)
                m_publicNetClient.addRemoteEndpoint(ep, std::chrono::seconds(5));
        }
    }

    {//绑定各种事件的处理函数
        using namespace std::placeholders;
        //私网的新连接
        if(m_privateNetServer)
            m_privateNetServer->e_newConn.reg(std::bind(&Process::newPrivateInConnection, this, _1));
        if(m_publicNetClient)
            m_privateNetClient.e_newConn.reg(std::bind(&Process::newPrivateOutConnection, this, _1));

        //检查通过的连接
        {
        }

        //定时执行消息处理
        m_timer.regEventHandler(std::chrono::milliseconds(20),
                                std::bind(&Process::handleMsgByTimer, this, _1));

        //成员结束时自动结束整个process
        m_server.e_close.reg(std::bind(&Process::terminate, this));
        m_client.e_close.reg(std::bind(&Process::terminate, this));
        m_conns.e_close.reg(std::bind(&Process::terminate, this));
    }
}

void Process::newPrivateInConnection(net::TcpConnection::Ptr tcpConn)
{
    PacketConnection::Ptr conn = PacketConnection::create(std::move(*tcpConn), 8, 11);
    m_connChecker.addUncheckedPrivateConnection(conn, ConnectionChecker::ConnType::in);
}

void Process::newPrivateOutConnection(net::TcpConnection::Ptr tcpConn)
{
    PacketConnection::Ptr conn = PacketConnection::create(std::move(*tcpConn), 8, 11);
    m_connChecker.addUncheckedPrivateConnection(conn, ConnectionChecker::ConnType::out);
}

void Process::newPublicInConnection(net::TcpConnection::Ptr tcpConn)
{
    //msg::ProcessInendityNum idMsg;
    PacketConnection::Ptr conn = PacketConnection::create(std::move(*tcpConn), 8, 11);
    if(m_conns.addConnection(conn))
        LOG_TRACE("public in connection: {}", conn->getRemoteEndpoint().toString());
}

void Process::handleMsgByTimer(const componet::TimePoint& now)
{
    PacketConnection::Ptr conn;
    Packet::Ptr packet;
    while(m_conns.getPacket(&conn, &packet))
    {
        packetHandler(conn, packet, now);
    }
}

void Process::packetHandler(PacketConnection::Ptr conn, Packet::Ptr packet, const componet::TimePoint& now)
{
    LOG_DEBUG("recv packet, from {}, length = {}", 
              conn->getRemoteEndpoint().toString(), packet->contentSize());
}

}

