/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:05 +0800
 *
 * Description: 
 *
 epoll 的封装， 底层epoll使用LT模式
 仅支持对TcpSocket*的操作， 不支持底层socketfd
 */

#ifndef WATER_NET_EPOLLER_H
#define WATER_NET_EPOLLER_H


#include "../componet/macro.h"
#include "socket.h"

#include <functional>

namespace water{
namespace net{


class Epoller
{
public:
    enum class EventType : int32_t { ERROR = 0, READ = 1, WRITE = 2 };

    TYPEDEF_PTR(Epoller)
    CREATE_FUN_MAKE(Epoller)
public:
    Epoller();
    ~Epoller();

    void regSocket(TcpSocket* socket, EventType et);
    void delSocket(TcpSocket* socket);

    void wait(int32_t timeout = -1);

private:
    int32_t m_epollfd = -1;
};

}}

#endif
