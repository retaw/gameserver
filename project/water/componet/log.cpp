#include "log.h"



namespace water{
namespace componet{

GlobalLogger* GlobalLogger::getInstance() 
{
    static std::atomic<GlobalLogger*> m_instance;
    static std::mutex m_mutex;
    GlobalLogger* tmp = m_instance.load(std::memory_order_acquire);
    if (tmp == nullptr) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        tmp = m_instance.load(std::memory_order_acquire);
        if (tmp == nullptr) 
        {
            tmp = new GlobalLogger;
            m_instance.store(tmp, std::memory_order_release);
        }
    }

    return tmp;
}

}}
