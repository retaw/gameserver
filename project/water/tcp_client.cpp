#include "tcp_client.h"

#include <iostream>
#include <thread>

#include "net/connector.h"
#include "tcp_connection_manager.h"

namespace water{

TcpClient::TcpClient()
: m_switch(Switch::off)
{
}

void TcpClient::addRemoteEndpoint(net::Endpoint ep, std::chrono::seconds retryInterval)
{
    if(m_remoteEndpoints.find(ep) == m_remoteEndpoints.end())
    {
        RemoteEndpointInfo info;
        info.retryInterval = retryInterval;
        info.retryTimepoint = componet::Clock::now();
        m_remoteEndpoints.insert(std::make_pair(ep, info));
    }
    else
    {
        m_remoteEndpoints[ep].retryInterval = retryInterval;
    }
}

void TcpClient::run()
{
    m_switch.store(Switch::on, std::memory_order_release);
    //while(m_switch.load(std::memory_order_relaxed) == Switch::on)
    {
        const auto now = componet::Clock::now();
        for(auto it = m_remoteEndpoints.begin(); it != m_remoteEndpoints.end(); ++it)
        {
            if(m_switch.load(std::memory_order_relaxed) == Switch::off)
                break;

            const net::Endpoint& ep = it->first;
            const std::string epStr = ep.toString();
            RemoteEndpointInfo& info = it->second;

            if(info.retryTimepoint > now) //未到重试时间
                continue;

            if(!info.conn.expired())
                continue;

            try
            {
                std::cout << "连接:[" << ep.ip << ":" << ep.port << "]" << std::endl;

                auto connector = net::TcpConnector::create(ep);
                net::TcpConnection::Ptr conn = connector->connect(std::chrono::milliseconds(1000 * 5)); //5s超时
                if(conn != nullptr)
                {
                    std::cout << "主动新建连接成功:{" << epStr << "}" << std::endl;
                    it->second.conn = conn;
                    conn->e_onClose.reg([epStr](net::TcpSocket*)
                                        {std::cout << "out连接已断开" << epStr << std::endl;});
                    e_newConn(conn);
                }
                else
                {
                    std::cout << "连接超时 " << ep.toString() << std::endl;
                    continue;
                }
            }
            catch (const net::NetException& ex)
            {
                std::cerr << "连接操作异常 [" << epStr << "] " << info.retryInterval.count() << "秒后重试";
                std::cerr << ex.what() << std::endl;
                info.retryTimepoint = now + info.retryInterval;
            }
        }

        //暂停1秒
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "connector thread terminated" << std::endl;
}

void TcpClient::stop()
{
    Switch s = Switch::on;
    if(m_switch.compare_exchange_strong(s, Switch::off, std::memory_order_release))
        e_onClose(this);
}

}
