/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:06 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_LISTENER_HPP
#define WATER_NET_LISTENER_HPP

#include "socket.h"
#include "connection.h"

namespace water{
namespace net{


class TcpListener : public TcpSocket
{
public:
    TYPEDEF_PTR(TcpListener);
public:
    CREATE_FUN_MAKE(TcpListener);
    explicit TcpListener() = default;
    //using TcpSocket::TcpSocket;

public:
    ~TcpListener() = default;

    void listen(int32_t backlog = 20);
    TcpConnection::Ptr accept();

private:
};


}}
#endif //#ifndef WATER_NET_LISTENER_HPP
