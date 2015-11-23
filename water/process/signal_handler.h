/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-17 17:21 +0800
 *
 * Description:  信号处理
 */


#ifndef WATER_PROCESS_SIGNAL_HANDLER_H
#define WATER_PROCESS_SIGNAL_HANDLER_H


#include "componet/event.h"

#include <csignal>
#include <map>
#include <initializer_list>

namespace water{
namespace process{


class SignalHandler final
{
    using Handle = std::function<void ()>;
public:

    static void setSignalHandle(int signal, const Handle& handle);
    static void setSignalHandle(std::initializer_list<int> signalList, const Handle& handle);

    static void resetSignalHandle(int signal);
    static void resetSignalHandle(std::initializer_list<int> signalList);

private:
    static void handler(int signal);

private:
    static std::map<int, std::function<void ()>> m_signalHandles;
};

}}


#endif
