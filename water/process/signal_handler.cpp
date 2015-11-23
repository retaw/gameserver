#include "signal_handler.h"

#include "componet/logger.h"



namespace water{
namespace process{

std::map<int, std::function<void ()>> SignalHandler::m_signalHandles;

void SignalHandler::setSignalHandle(int signal, const Handle& handle)
{
    std::signal(signal, &SignalHandler::handler);
    m_signalHandles[signal] = handle;
}

void SignalHandler::setSignalHandle(std::initializer_list<int> signalList, const Handle& handle)
{
    for(int signal : signalList)
        setSignalHandle(signal, handle);
}

void SignalHandler::resetSignalHandle(int signal)
{
    std::signal(signal, SIG_DFL);
    m_signalHandles.erase(signal);
}

void SignalHandler::resetSignalHandle(std::initializer_list<int> signalList)
{
    for(int signal : signalList)
        resetSignalHandle(signal);
}

void SignalHandler::handler(int signal)
{
    LOG_DEBUG("收到信号{}", signal);
    auto it = m_signalHandles.find(signal);
    if(it == m_signalHandles.end())
        return;

    it->second();
}


}}

