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

#include <functional>
#include <chrono>


namespace water{
namespace net{

class Epoller
{
public:
    enum class Event : int32_t { error = 0, read = 1, write = 2, read_write = 3 };
    typedef std::function<void(Epoller* epoller, int32_t socketFD, Event event)>  EventHandler;
public:
    Epoller();
    ~Epoller();

    void regSocket(int32_t socketFD, Event et);
    void modifySocket(int32_t socketFD, Event et);
    void delSocket(int32_t socketFD);

    void setEventHandler(const EventHandler& eventHanlder);

    void wait(std::chrono::milliseconds timeout);
    void wait();

private:
    void waitImpl(int32_t timeout);

private:
    int32_t m_epollfd = -1;
    EventHandler m_eventHanlder;
};

}}

#endif
