#include "tcp_server.h"

#include "componet/log.h"

#include <iostream>
#include <functional>

namespace water{

TcpServer::TcpServer()
{
}

void TcpServer::addLocalEndpoint(const net::Endpoint& ep)
{
    m_localEndpoints.insert(ep);
}

bool TcpServer::exec()
{
    try 
    {
        {//绑定事件处理器
            using namespace std::placeholders;
            m_epoller.setEventHandler(std::bind(&TcpServer::epollEventHandler, this, _2, _3));
        }

        //开始监听
        for(const net::Endpoint& ep : m_localEndpoints)
        {
            auto listener = net::TcpListener::create();
            listener->bind(ep);
            listener->setNonBlocking();
            listener->listen();
            m_epoller.regSocket(listener->getFD(), net::Epoller::Event::READ);
            m_listeners.insert({listener->getFD(), listener});
            LOG_TRACE("start listening on {}", ep.toString());
        }

        //开始epoll循环
        while(checkSwitch())
        {   
            m_epoller.wait(std::chrono::milliseconds(50));
        }
    }
    catch (const net::NetException& ex) 
    {   
        LOG_ERROR("监听失败, {}", ex.what());
        return false;
    }

    LOG_TRACE("listener thread stoped");
    return true;
}

void TcpServer::epollEventHandler(int32_t socketFD, net::Epoller::Event event)
{
    auto it = m_listeners.find(socketFD);
    if(it == m_listeners.end())
    {
        LOG_ERROR("不存在的listener");
        m_epoller.delSocket(socketFD);
        return;
    }
    net::TcpListener::Ptr listener = it->second;

    switch (event)
    {
    case net::Epoller::Event::ERROR:
        {
            LOG_ERROR("listener的收到error事件");
            m_epoller.delSocket(socketFD);
        }
        break;
    case net::Epoller::Event::WRITE:
        {
            LOG_ERROR("listener的收到write事件");
        }
        break;
    case net::Epoller::Event::READ:
        {
            try 
            {
                net::TcpConnection::Ptr conn = listener->accept();
                if(conn == nullptr)
                    return;

                LOG_TRACE("新呼入连接: {}", conn->getRemoteEndpoint().toString());
                e_newConn(conn);
            }
            catch (const net::NetException& ex) 
            {   
                LOG_TRACE("accept失败, {}", ex.what());
            }
        }
        break;
    default:
        break;
    }
}

}

