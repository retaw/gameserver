/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:06 +0800
 *
 * Description: 
 */
#ifndef WATER_NET_SOCKET_HPP
#define WATER_NET_SOCKET_HPP

#include "../componet/macro.h"
#include "../componet/event.h"
#include "endpoint.h"
namespace water{
namespace net{

class TcpSocket
{
    friend class Epoller;
public:
    enum class BlockingStatus {BLOCKING, NON_BLOCKING };

public:
    TYPEDEF_PTR(TcpSocket)
protected:
    explicit TcpSocket();
    explicit TcpSocket(int32_t fd);

public:
    virtual ~TcpSocket();

    //non-copyable
    TcpSocket(const TcpSocket&) = delete;
    TcpSocket(TcpSocket&&) = delete;
    TcpSocket& operator = (const TcpSocket&) = delete;

    void bind(uint16_t port);
    void bind(const std::string& strIp, uint16_t port);
    void bind(const Endpoint& endPoint);

    void setNonBlocking();
    bool isNonBlocking() const;

    int32_t getFD() const;

    bool isAvaliable() const;

    void close();

public:
    typedef componet::Event<void (TcpSocket*)> OnClosedEvent;
    OnClosedEvent onClosed;

public:
    typedef std::function<void ()> EpollCallback;

    void setEpollReadCallback(EpollCallback cb);
    void setEpollWriteCallback(EpollCallback cb);
    void setEpollErrorCallback(EpollCallback cb);

private:
    EpollCallback m_epollReadCallback;
    EpollCallback m_epollWriteCallback;
    EpollCallback m_epollErrorCallback;

private:
    int32_t m_fd;
    bool m_autoClose;

public:
    static bool isNonBlockingFD(int32_t fd);
};

}}
#endif //#ifndef WATER_NET_SOCKET_H
