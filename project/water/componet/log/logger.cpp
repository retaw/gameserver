#include "logger.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "stdout_writer.h"

water::componet::Logger gLogger;

namespace water {
namespace componet{

thread_local LogStream m_stream;
thread_local char t_time[18];
thread_local time_t t_lastSecond;

Logger::Logger()
{
	this->setWriter(std::make_shared<StdoutWriter>());
}

Logger::~Logger()
{
	for (const auto &item : writerMap)
	{
		if (item.second != nullptr)
			item.second->stop();
	}
	writerMap.clear();
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

//FIXME: change to use chrono 
void Logger::formatTime()
{
    time_t now;
    now = time(&now);
    struct tm vtm;
    localtime_r(&now, &vtm);

    if (t_lastSecond != now)
    {
        t_lastSecond = now;
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",vtm.tm_year+1990, vtm.tm_mon+1, vtm.tm_mday, vtm.tm_hour, vtm.tm_min, vtm.tm_sec);
        assert(len == 17);
    }
    m_stream << std::string(t_time);
}


void Logger::setWriter(std::shared_ptr<Writer> writer)
{
	if (writer == nullptr)
		return;
	if (this->getWriter(writer->getWriteType()) != nullptr)  //已经有一个文件写
		return;

	writerMap.insert({writer->getWriteType(), writer});
	writer->start();
}

std::shared_ptr<Writer> Logger::getWriter(const WriterType type) const
{
	auto iter = writerMap.find(type);
	if (iter != writerMap.end())
	{
		return iter->second;
	}
	return nullptr;
}

void Logger::restartWriter(const WriterType type)
{
	auto writer = this->getWriter(type);
	if (writer)
	{
		writer->stop();
		writer->start();
	}
}

void Logger::stopWriter(const WriterType type)
{
	auto writer = this->getWriter(type);
	if (writer)
		writer->stop();
}


}} 
