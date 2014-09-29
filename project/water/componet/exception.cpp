#include "exception.h"
#include <iostream>
#include <sstream>
#include <cxxabi.h>

#include <string.h>

#include <stdlib.h>
//#include <execinfo.h>


namespace water{

ExceptionBase::ExceptionBase(const std::string& msg, 
                             const std::string& file, 
                             const std::string& func, 
                             int32_t line,
                             int32_t sysErrno) noexcept
: m_msg(msg), m_file(file), m_func(func), m_line(line), m_sysErrno(sysErrno)
{
}

ExceptionBase::~ExceptionBase() noexcept
{
}

std::string ExceptionBase::exceptionName() const noexcept
{
    return "ExceptionBase";
}

const char* ExceptionBase::what() const noexcept
{
    if (!m_what.empty())
        return m_what.c_str();
    
    std::stringstream ss;
    ss << m_file << "+" << m_line << "," << exceptionName() << "," << errno << ":" << ::strerror(errno) << " {" << m_msg << "}";
    m_what = ss.str();
   
    return m_what.c_str();
}

}
