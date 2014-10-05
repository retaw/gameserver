#include "logger.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

water::componet::Logger gLogger;

namespace water {
namespace componet{

thread_local LogStream m_stream;
thread_local char t_time[18];
thread_local time_t t_lastSecond;

void defaultOutput(const char* msg, uint32_t len)
{
    fwrite(msg, 1, len, stdout);
}

using std::placeholders::_1;
using std::placeholders::_2;
Logger::Logger()
    : m_level(LogLevel::LL_DEBUG),
      m_appendcb(std::bind(&defaultOutput,_1,_2))
{
}

const char* Logger::getLevelStr(LogLevel l)
{
    switch (l)
    {
        case LogLevel::LL_DEBUG:
            return "DEBUG";
        case LogLevel::LL_TRACE:
            return "TRACE";
        case LogLevel::LL_WARN:
            return "WARN";
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

}} 
