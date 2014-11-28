#include "packet_connection_manager.h"

#include <iostream>
#include <mutex>

#include "componet/other_tools.h"
#include "componet/log.h"

namespace water{


/****************inner class*********************/
bool PacketConnectionManager::ConnectionWithEpoller::insert(PacketConnection::Ptr conn)
{
    std::lock_guard<componet::Spinlock> lock(m_lock);

    if(!m_conns.insert({conn->getFD(), conn}).second)
        return false;
    regSocket(conn->getFD(), net::Epoller::Event::READ);
    return true;
}

void PacketConnectionManager::ConnectionWithEpoller::erase(int32_t socketFD)
{
    std::lock_guard<componet::Spinlock> lock(m_lock);

    //这里要最后调用m_conns.erase
    //保证先执行epoll_ctl_del，然后才可能会执行 close(socketFD)
    delSocket(socketFD);
    m_conns.erase(socketFD);
}

PacketConnection::Ptr PacketConnectionManager::ConnectionWithEpoller::find(int32_t socketFD)
{
    auto it = m_conns.find(socketFD);
    if(it == m_conns.end())
        return nullptr;
    return it->second;
}

/***********************************/


PacketConnectionManager::PacketConnectionManager()
: m_switch(Switch::off)
{
}

bool PacketConnectionManager::addConnection(PacketConnection::Ptr conn)
{
    conn->setNonBlocking();
    return m_connsEpoller.insert(conn);
}

void PacketConnectionManager::delConnection(PacketConnection::Ptr conn)
{
    m_connsEpoller.erase(conn->getFD());
    conn->close();
}

PacketConnection::Ptr PacketConnectionManager::getConnection() const
{
    return nullptr;
}

void PacketConnectionManager::run()
{
    m_switch.store(Switch::on, std::memory_order_release);

    try
    {
        {//绑定epoll消息处理器
            using namespace std::placeholders;
            m_connsEpoller.setEventHandler(std::bind(&PacketConnectionManager::epollerEventHandler, this, _1, _2, _3));
        }

        while(m_switch.load(std::memory_order_relaxed) == Switch::on)
        {
            m_connsEpoller.wait(std::chrono::milliseconds(10)); //10 milliseconds 一轮
        }
    }
    catch (const net::NetException& ex)
    {
        std::cerr << "connManager 异常 " << ex.what() << std::endl;
        stop();
    }
}

void PacketConnectionManager::stop()
{
    Switch s = Switch::on;
    if(m_switch.compare_exchange_strong(s, Switch::off, std::memory_order_release))
        e_close(this);
}

void PacketConnectionManager::epollerEventHandler(net::Epoller* epoller, int32_t socketFD, net::Epoller::Event event)
{
    PacketConnection::Ptr conn = m_connsEpoller.find(socketFD);
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
                while(Packet::Ptr packet = conn->getPacket())
                {
                    std::pair<PacketConnection::Ptr, Packet::Ptr> tmp{conn, packet};
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
            }
            break;
        case net::Epoller::Event::WRITE:
            if(conn->sendAll()) //socket已可写，不用再监听write事件
            {
                m_connsEpoller.modifySocket(socketFD, net::Epoller::Event::READ);
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
    catch (const ReadClosedConnection& ex)
    {
        std::cerr << "对方断开连接: " << ex.msg() << std::endl;
        delConnection(conn);
    }
}

bool PacketConnectionManager::getPacket(PacketConnection::Ptr* conn, Packet::Ptr* packet)
{
    std::pair<PacketConnection::Ptr, Packet::Ptr> ret;
    if(!m_recvMsgQueue.pop(&ret))
        return false;

    *conn = ret.first;
    *packet = ret.second;
    return true;
}

}
