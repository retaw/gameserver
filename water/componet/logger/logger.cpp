#include "logger.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "stdout_writer.h"

//water::componet::Logger gLogger;

namespace water {
namespace componet{

Logger::Logger()
{
    this->setWriter(std::make_shared<StdoutWriter>());
}

Logger::~Logger()
{
    for (const auto &item : m_writerMap)
    {
        if (item.second != nullptr)
            item.second->stop();
    }
    m_writerMap.clear();
}

const char* Logger::getLevelStr(LogLevel l)
{
    switch (l)
    {
    case LogLevel::LL_DEBUG:
        return "DEBUG";
    case LogLevel::LL_TRACE:
        return "TRACE";
    case LogLevel::LL_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

std::string Logger::formatTime()
{
    time_t now;
    now = time(&now);
    struct tm vtm;
    localtime_r(&now, &vtm);

    static thread_local time_t lastSecond = 0;
    static thread_local char result[18];

    if (lastSecond != now)
    {
        lastSecond = now;
        int len = snprintf(result, sizeof(result), "%4d%02d%02d %02d:%02d:%02d", vtm.tm_year + 1990, vtm.tm_mon + 1, vtm.tm_mday, vtm.tm_hour, vtm.tm_min, vtm.tm_sec);
        assert(len == 17);
    }
    return result;
}


void Logger::setWriter(std::shared_ptr<Writer> writer)
{
    if(writer->getWriteType() == WriterType::stdOut)
        std::cout << "DEBUG:终端日志设置setWriter()" << std::endl;
    if(writer->getWriteType() == WriterType::fileOut)
        std::cout << "DEBUG:文件日志设置setWriter()"<<std::endl;
    if (writer == nullptr)
    {
        std::cout << "ERROR:日志设置指针为空setWriter()"<<std::endl;
        return;
    }
    m_writerMapLock.lock();
    if (this->getWriter(writer->getWriteType()) != nullptr)  //已经有一个文件写
    {
        if(writer->getWriteType() == WriterType::stdOut)
            std::cout << "ERROR:已经存终端类型的日志setWriter()"<<std::endl;
        if(writer->getWriteType() == WriterType::fileOut)
            std::cout << "ERROR:已经存在文件类型的日志setWriter()"<<std::endl;
        return;
    }
    m_writerMap.insert({writer->getWriteType(), writer});
    m_writerMapLock.unlock();
    writer->start();
}

std::shared_ptr<Writer> Logger::getWriter(const WriterType type) const
{
    auto iter = m_writerMap.find(type);
    if (iter != m_writerMap.end())
    {
        return iter->second;
    }
    return nullptr;
}

void Logger::clearWriter(const WriterType type)
{
    m_writerMapLock.lock();
    auto iter = m_writerMap.find(type);
    if(iter != m_writerMap.end())
    {
        auto &writer = iter->second;
        if(writer)
        {
            writer->stop();
            m_writerMap.erase(iter);
        }
    }
    m_writerMapLock.unlock();

}

}} 
