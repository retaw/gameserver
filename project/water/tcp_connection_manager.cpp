#include "tcp_connection_manager.h"

#include <iostream>
#include <mutex>

#include "componet/other_tools.h"
#include "componet/log.h"

namespace water{


/****************inner class*********************/
bool TcpConnectionManager::ConnectionWithEpoller::insert(net::TcpConnection::Ptr conn)
{
    std::lock_guard<componet::Spinlock> lock(m_lock);

    if(!m_datas.insert({conn->getFD(), conn}).second)
        return false;
    regSocket(conn->getFD(), net::Epoller::Event::READ);
    return true;
}

void TcpConnectionManager::ConnectionWithEpoller::erase(int32_t socketFD)
{
    std::lock_guard<componet::Spinlock> lock(m_lock);

    //这里要最后调用m_datas.erase
    //保证先执行epoll_ctl_del，然后才可能会执行 close(socketFD)
    delSocket(socketFD);
    m_datas.erase(socketFD);
}

net::TcpConnection::Ptr TcpConnectionManager::ConnectionWithEpoller::find(int32_t socketFD)
{
    auto it = m_datas.find(socketFD);
    if(it == m_datas.end())
        return nullptr;
    return it->second;
}

/***********************************/


TcpConnectionManager::TcpConnectionManager()
: m_switch(Switch::off)
{
}

void TcpConnectionManager::addConnection(net::TcpConnection::Ptr conn, ConnType ct)
{
    conn->setNonBlocking();
    if(!m_conns.insert(conn))
    {
        std::cerr << "连接放入管理器 {" <<  conn->getRemoteEndpoint().toString() << "}" << std::endl;
    }
}

void TcpConnectionManager::delConnection(net::TcpConnection::Ptr conn)
{
    m_conns.erase(conn->getFD());
    conn->close();
}

void TcpConnectionManager::run()
{
    m_switch.store(Switch::on, std::memory_order_release);

    try
    {
        {//绑定epoll消息处理器
            using namespace std::placeholders;
            m_conns.setEventHandler(std::bind(&TcpConnectionManager::epollerEventHandler, this, _1, _2, _3));
        }

        while(m_switch.load(std::memory_order_relaxed) == Switch::on)
        {
            m_conns.wait(std::chrono::milliseconds(10)); //10 milliseconds 一轮
        }
    }
    catch (const net::NetException& ex)
    {
        std::cerr << "connManager 异常 " << ex.what() << std::endl;
        stop();
    }
}

void TcpConnectionManager::stop()
{
    Switch s = Switch::on;
    if(m_switch.compare_exchange_strong(s, Switch::off, std::memory_order_release))
        e_onClose(this);
}

void TcpConnectionManager::epollerEventHandler(net::Epoller* epoller, int32_t socketFD, net::Epoller::Event event)
{
    net::TcpConnection::Ptr conn = m_conns.find(socketFD);
    if(conn == nullptr)
    {
        LOG_ERROR("epoll报告一个socket事件，但该socket不在manager中");
        return;
    }

    try
    {
        switch (event)
        {
        case net::Epoller::Event::READ:
            {
                conn->recvAll();
                net::Packet::Ptr packet = conn->getPacket();
                if(packet == nullptr)
                    return;
                std::pair<net::TcpConnection::Ptr, net::Packet::Ptr> tmp{conn, packet};
                if(!m_recvMsgQueue.push(tmp))
                {
                    LOG_TRACE("消息接收队列满，消息处理系统速度过慢");
                    //while保证不丢消息
                    do
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    } while(!m_recvMsgQueue.push(tmp));
                }
            }
            break;
        case net::Epoller::Event::WRITE:
            if(conn->sendAll()) //socket已可写，不用再监听write事件
            {
                m_conns.modifySocket(socketFD, net::Epoller::Event::READ);
            }
            break;
        case net::Epoller::Event::ERROR:
            {
                std::cerr << "epoll报告连接异常: {" << conn->getRemoteEndpoint().toString() << "}" << std::endl;
                delConnection(conn);
            }
            break;
        }
    }
    catch (const net::ReadClosedConnection& ex)
    {
        std::cerr << "对方断开连接: " << ex.msg() << std::endl;
        delConnection(conn);
    }
}

bool TcpConnectionManager::getPacket(net::TcpConnection::Ptr* conn, net::Packet::Ptr* packet)
{
    std::pair<net::TcpConnection::Ptr, net::Packet::Ptr> ret;
    if(!m_recvMsgQueue.pop(&ret))
        return false;

    *conn = ret.first;
    *packet = ret.second;
    return true;
}

}
