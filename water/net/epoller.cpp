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

#include "socket.h"
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

void Epoller::regSocket(int32_t socketFD, Event et)
{
    struct epoll_event ev;
    ev.data.fd = socketFD;

    if(et == Event::read)
    {
        ev.events = EPOLLIN;
    }
    else if(et == Event::write)
    {
        ev.events = EPOLLOUT;
    }
    else if(et == Event::read_write)
    {
        ev.events = EPOLLIN | EPOLLOUT;
    }
    else
    {
        return;
    }

    if(::epoll_ctl(m_epollfd, EPOLL_CTL_ADD, socketFD, &ev) == -1)
            SYS_EXCEPTION(NetException, "::epoll_ctl, EPOLL_CTL_ADD");
}

void Epoller::modifySocket(int32_t socketFD, Event et)
{
    struct epoll_event ev;
    ev.data.fd = socketFD;

    if(et == Event::read)
    {
        ev.events = EPOLLIN;
    }
    else if(et == Event::write)
    {
        ev.events = EPOLLOUT;
    }
    else if(et == Event::read_write)
    {
        ev.events = EPOLLIN | EPOLLOUT;
    }
    else
    {
        return;
    }

    if(::epoll_ctl(m_epollfd, EPOLL_CTL_MOD, socketFD, &ev) == -1)
        SYS_EXCEPTION(NetException, "::epoll_ctl, EPOLL_CTL_MOD");
}

void Epoller::delSocket(int32_t socketFD)
{
    if(::epoll_ctl(m_epollfd, EPOLL_CTL_DEL, socketFD, nullptr) == -1)
    {
        if(errno == ENOENT || errno == EBADF) //删除时已经不存在了，不视为错误
            return;

        SYS_EXCEPTION(NetException, "::epoll_ctl, EPOLL_CTL_DEL");
    }
}

void Epoller::setEventHandler(const EventHandler& eventHanlder)
{
    m_eventHanlder = eventHanlder;
}

void Epoller::wait(std::chrono::milliseconds timeout)
{
    waitImpl(timeout.count());
}

void Epoller::wait()
{
    waitImpl(-1);
}

void Epoller::waitImpl(int32_t timeout)
{
    const uint32_t maxevents = 100;
    struct epoll_event events[maxevents];

    const int32_t eventSize = ::epoll_wait(m_epollfd, events, maxevents, timeout);
    if(eventSize == -1 && errno != EINTR)
        SYS_EXCEPTION(NetException, "::epoll_wait");

    for(int32_t i = 0; i < eventSize; ++i)
    {
        bool ok = false;
        int32_t socketFD = events[i].data.fd;
        if(events[i].events & EPOLLIN || (events[i].events & EPOLLHUP) )
        {
            m_eventHanlder(this, socketFD, Event::read);
            ok = true;
        }

        if(events[i].events & EPOLLOUT)
        {
            m_eventHanlder(this, socketFD, Event::write);
            ok = true;
        }

        if( !ok || (events[i].events & EPOLLERR) )
        {
            m_eventHanlder(this, socketFD, Event::error);
            //不再wait这个scoketFD
            delSocket(socketFD); 
        }

    }
}


}}
