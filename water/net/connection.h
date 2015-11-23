/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:04 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_CONNECTION_HPP
#define WATER_NET_CONNECTION_HPP

#include "socket.h"
#include "net_exception.h"
#include "endpoint.h"
#include "componet/lock_free_circular_queue_ss.h"


namespace water{ 
namespace net{


class TcpConnection : public TcpSocket
{
    friend class TcpListener;
    friend class TcpConnector;

public:
    TYPEDEF_PTR(TcpConnection)

private:
    CREATE_FUN_NEW(TcpConnection)
    explicit TcpConnection(const Endpoint& remoteEndpoint);
    explicit TcpConnection(int32_t socketFD, const Endpoint& remoteEndpoint);

public:
    ~TcpConnection();

    //move able
    TcpConnection(TcpConnection&& other);
    TcpConnection& operator=(TcpConnection&& other);

public:
    enum class ConnState : uint8_t 
    {
        closed         = 0, 
        read           = 1, 
        write          = 2, 
        read_nad_write = 3
    };

    const Endpoint& getRemoteEndpoint() const;

    void shutdown(ConnState state = ConnState::read_nad_write);

    ConnState getState() const;

public://发送与接收
    //返回-1时, 表示noblocking的socket, 返回EAGAIN或EWOULDBLOCK, 即socket忙
    int32_t send(const void* data, int32_t dataLen);
    //返回-1时, 表示noblocking的socket, 返回EAGAIN或EWOULDBLOCK, 即socket忙
    //返回0时, 表示socket不可写, 即发送已被关闭
    int32_t recv(void* data, int32_t dataLen);

private:
    Endpoint m_remoteEndpoint;
    ConnState m_state;
};

}}

#endif

