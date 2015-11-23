#include "tcp_client.h"

#include <iostream>
#include <thread>

#include "net/connector.h"
#include "componet/logger.h"

namespace water{
namespace process{

TcpClient::TcpClient()
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

bool TcpClient::exec()
{
    while(checkSwitch())
    {
        const auto now = componet::Clock::now();
        for(auto it = m_remoteEndpoints.begin(); it != m_remoteEndpoints.end(); ++it)
        {
            if(!checkSwitch())
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
                LOG_TRACE("发起到{}的tcp连接", ep);

                auto connector = net::TcpConnector::create(ep);
                net::TcpConnection::Ptr conn = connector->connect(std::chrono::milliseconds(1000 * 5)); //5s超时
                if(conn != nullptr)
                {
                    LOG_TRACE("已建立到{}的tcp连接", ep);
                    conn->e_close.reg([ep](net::TcpSocket*)
                                      {LOG_TRACE("主动tcp连接已断开, remoteEp={}", ep);});
                    auto packetConn = net::PacketConnection::create(std::move(*conn));
                    e_newConn(packetConn);
                    info.conn = packetConn;
                }
                else
                {
                    LOG_TRACE("到{}的tcp连接超时", ep);
                    continue;
                }
            }
            catch (const net::NetException& ex)
            {
                LOG_TRACE("发起到{}的tcp连接出错, {}秒后重试, {}", 
                          ep, info.retryInterval.count(), ex.what());
                info.retryTimepoint = now + info.retryInterval;
            }
        }

        //暂停1秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return true;
}

uint32_t TcpClient::readyNum() const
{
    uint32_t ret = 0;
    for(const auto& item : m_remoteEndpoints)
    {
        if(item.second.conn.expired())
            ++ret;
    }
    return ret;
}



}}
