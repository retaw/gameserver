/*
 * Description: 日志模块前端，example: 2014-07-27 09:41:23 DEBUG: hello shok
 */
#ifndef WATER_COMPONET_LOGGER_HPP
#define WATER_COMPONET_LOGGER_HPP

#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <time.h>
#include <map>
#include "writer.h"
#include "../class_helper.h"
#include "../format.h"

namespace water{
namespace componet{

typedef std::stringstream LogStream; //FIXME: move to a better one
extern thread_local LogStream m_stream;

enum class LogLevel : uint8_t
{
	LL_DEBUG,
	LL_TRACE,
	LL_ERROR,
	LL_MAX
};

class Logger
{
	NON_COPYABLE(Logger);
public:
    Logger();
    ~Logger();
public:
    void setWriter(std::shared_ptr<Writer> writer);
	std::shared_ptr<Writer> getWriter(const WriterType type) const;
	void restartWriter(const WriterType type) ;
	void stopWriter(const WriterType type) ;

	template<typename... Args>   
	void trace(const Args&... args)
	{
		return this->log(LogLevel::LL_TRACE, std::forward<const Args>(args)...);
	}

	template<typename... Args>   
	void debug(const Args&... args)
	{
		return this->log(LogLevel::LL_DEBUG, std::forward<const Args>(args)...);
	}

	template<typename... Args>   
	void error(const Args&... args)
	{
		return this->log(LogLevel::LL_ERROR, std::forward<const Args>(args)...);
	}
	
private:
    void formatTime();
    const char* getLevelStr(LogLevel l);

    template<typename... Args>
    void log(LogLevel level, const Args&... args)
    {
        formatTime();
        m_stream <<" "<< getLevelStr(level) << ": ";
        m_stream << format(args...);
        m_stream << "\n";
		for (const auto &item : writerMap)
		{
			if (item.second != nullptr)
				item.second->append(m_stream.str().data(), (uint32_t)(m_stream.str().size())); 
		}
		m_stream.str("");
        m_stream.clear();
    }
private:
	std::map<WriterType, std::shared_ptr<Writer>> writerMap; 
};



}}

#endif //#define WATER_COMPONET_LOGGER_HPP 
