/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 19:06 +0800
 *
 * Description: 
 */

#include "epoller.h"

#include <sys/epoll.h>
#include <linux/version.h>

#include "net_exception.h"

namespace water{
namespace net{

Epoller::Epoller()
: m_epollfd(::epoll_create(10))
{
    if(m_epollfd == -1)
        SYS_EXCEPTION(NetException, "::epoll_create");
}

Epoller::~Epoller()
{
}

void Epoller::regSocket(TcpSocket* socket, EventType et)
{
    struct epoll_event ev;
    ev.data.ptr = socket;

    if(et == EventType::READ)
    {
        ev.events = EPOLLIN;
    }
    else if(et == EventType::WRITE)
    {
        ev.events = EPOLLOUT;
    }
    else
    {
        return;
    }

    if(::epoll_ctl(m_epollfd, EPOLL_CTL_ADD, socket->getFD(), &ev) == -1)
    {
        if(errno != EEXIST)
            SYS_EXCEPTION(NetException, "::epoll_ctl, EPOLL_CTL_ADD");

        if(::epoll_ctl(m_epollfd, EPOLL_CTL_MOD, socket->getFD(), &ev) == -1)
            SYS_EXCEPTION(NetException, "::epoll_ctl, EPOLL_CTL_MOD");
    }
}

void Epoller::delSocket(TcpSocket* socket)
{
    if(::epoll_ctl(m_epollfd, EPOLL_CTL_DEL, socket->getFD(), nullptr) == -1)
    {
        if(errno == ENOENT) //删除时已经不存在了，不视为错误
            return;

        SYS_EXCEPTION(NetException, "::epoll_ctl, EPOLL_CTL_DEL");
    }
}

void Epoller::wait(int32_t timeout)
{
    const uint32_t maxevents = 100;
    struct epoll_event events[maxevents];

    const int32_t eventSize = ::epoll_wait(m_epollfd, events, maxevents, timeout);
    if(eventSize == -1)
        SYS_EXCEPTION(NetException, "::epoll_wait");

    for(int32_t i = 0; i < eventSize; ++i)
    {
        TcpSocket* socket = reinterpret_cast<TcpSocket*>(events[i].data.ptr);
        if(events[i].events & EPOLLIN)
        {
            socket->m_epollReadCallback();
        }
        if(events[i].events & EPOLLOUT)
        {
            socket->m_epollWriteCallback();
        }
        if( (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) )
        {
            socket->m_epollErrorCallback();
        }
    }
}

}}
