#include "logger.h"



namespace water{
namespace componet{

GlobalLogger* GlobalLogger::getMe() 
{
    static std::atomic<GlobalLogger*> m_me;
    static std::mutex m_mutex;
    GlobalLogger* tmp = m_me.load(std::memory_order_acquire);
    if (tmp == nullptr) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        tmp = m_me.load(std::memory_order_acquire);
        if (tmp == nullptr) 
        {
            tmp = new GlobalLogger;
            m_me.store(tmp, std::memory_order_release);
        }
    }

    return tmp;
}

}}
