#include "http_connection_manager.h"

#include <iostream>
#include <mutex>

#include "componet/other_tools.h"
#include "componet/logger.h"
#include "http_packet.h"

namespace water{
namespace process{


HttpConnectionManager::HttpConnectionManager()
{
}

void HttpConnectionManager::addPrivateConnection(net::PacketConnection::Ptr conn)
{
    conn->setNonBlocking();

    auto item = std::make_shared<ConnectionHolder>();
    item->id = uniqueID++;
    item->conn = conn;

    std::lock_guard<componet::Spinlock> lock(m_lock);

    auto insertAllRet = m_allConns.insert({conn->getFD(), item});
    if(insertAllRet.second == false)
    {
        LOG_ERROR("ConnectionManager, insert httpConn to m_allConns failed, remoteEp={}", 
                  conn->getRemoteEndpoint());
    }
    auto insertPrivateRet = m_privateConns.insert({item->id, item});
    if(insertPrivateRet.second == false)
    {
        LOG_ERROR("ConnectionManager, insert httpConn to m_privateConns failed, remoteEp={}", 
                  conn->getRemoteEndpoint());
        m_allConns.erase(insertAllRet.first);
    }

    try
    {
        m_epoller.regSocket(conn->getFD(), net::Epoller::Event::read);
        conn->setRecvPacket(HttpPacket::create());
        LOG_DEBUG("add conn to epoll read");
    }
    catch (const componet::ExceptionBase& ex)
    {
        LOG_ERROR("ConnectionManager, insert httpConn to epoller failed, ex={}, remoteEp={}",
                  ex.what(), conn->getRemoteEndpoint());
        m_allConns.erase(insertAllRet.first);
        m_privateConns.erase(insertPrivateRet.first);
    }
}

void HttpConnectionManager::delConnection(net::PacketConnection::Ptr conn)
{
    std::lock_guard<componet::Spinlock> lock(m_lock);

    auto it = m_allConns.find(conn->getFD());
    if(it == m_allConns.end())
        return;

    m_privateConns.erase(it->second->id);
    m_allConns.erase(it);
    m_epoller.delSocket(conn->getFD());
}

bool HttpConnectionManager::exec()
{
    try
    {
        if(!checkSwitch())
            return true;

        //绑定epoll消息处理器
        using namespace std::placeholders;
        m_epoller.setEventHandler(std::bind(&HttpConnectionManager::epollerEventHandler, this, _2, _3));

        while(checkSwitch())
        {
            m_epoller.wait(std::chrono::milliseconds(10)); //10 milliseconds 一轮
        }
    }
    catch (const net::NetException& ex)
    {
        LOG_ERROR("connManager 异常退出 , {}", ex.what());
        return false;
    }

    return true;
}

void HttpConnectionManager::epollerEventHandler(int32_t socketFD, net::Epoller::Event event)
{
    ConnectionHolder::Ptr connHolder;
    {
        std::lock_guard<componet::Spinlock> lock(m_lock);
        auto connsIter = m_allConns.find(socketFD);
        if(connsIter == m_allConns.end())
        {
            LOG_ERROR("epoll报告一个socket事件，但该socket不在manager中");
            return;
        }
        connHolder = connsIter->second;
    }
    net::PacketConnection::Ptr conn = connHolder->conn;
    try
    {
        switch (event)
        {
        case net::Epoller::Event::read:
            {
                while(conn->tryRecv())
                {
					e_packetrecv(connHolder, conn->getRecvPacket());
					this->delConnection(conn); //关闭连接
					break;
                }
            }
            break;
        case net::Epoller::Event::write:
            {
                componet::LockFreeCircularQueueSPSC<net::Packet::Ptr>::Ptr queue = connHolder->sendQueue;

                while(conn->trySend())
                {
                    if(queue == nullptr)
                    {
                        m_epoller.modifySocket(socketFD, net::Epoller::Event::read);
                        break;
                    }

                    net::Packet::Ptr packet;
                    if(queue->pop(&packet))
                        conn->setSendPacket(packet);
                    else
                        queue = nullptr; //队列已空，不再需要存在
                }
            }
            break;
        case net::Epoller::Event::error:
            {
                LOG_ERROR("epoll error, {}", conn->getRemoteEndpoint());
                delConnection(conn);
            }
            break;
        default:
            LOG_ERROR("epoll, unexcept event type, {}", conn->getRemoteEndpoint());
            break;
        }
    }
    catch (const net::ReadClosedConnection& ex)
    {
        LOG_TRACE("对方断开连接, {}", conn->getRemoteEndpoint());
        delConnection(conn);
    }
    catch (const net::NetException& ex)
    {
        LOG_TRACE("连接异常, {}", conn->getRemoteEndpoint());
        delConnection(conn);
    }
}

bool HttpConnectionManager::sendPacket(ConnectionHolder::Ptr connHolder, net::Packet::Ptr packet)
{
    if(connHolder->conn->setSendPacket(packet))
        return true;

    LOG_TRACE("socket 发送过慢, ep={}", 
              connHolder->conn->getRemoteEndpoint());

    if(connHolder->sendQueue == nullptr)
    {
        m_epoller.modifySocket(connHolder->conn->getFD(), net::Epoller::Event::read_write);
        connHolder->sendQueue = std::make_shared<componet::LockFreeCircularQueueSPSC<net::Packet::Ptr>>(9);
    }
    return connHolder->sendQueue->push(packet);
}


}}
