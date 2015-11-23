#include "exception.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdlib.h>

//#include <cxxabi.h>
//#include <execinfo.h>

//#include "format.h"

namespace water{
namespace componet{

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

std::string ExceptionBase::msg() const noexcept
{
    return m_msg;
}

const char* ExceptionBase::what() const noexcept
{
    if (!m_what.empty())
        return m_what.c_str();

    if(m_sysErrno != 0)
        m_what = format("[{execptionName}], msg:[{m_msg}], file:[{m_file}+{m_line}], fun:[{m_func}], errno:[{m_sysErrno} {strerr}]",
                        exceptionName(), m_msg, m_file, m_line, m_func, m_sysErrno, ::strerror(m_sysErrno));
    else
        m_what = format("[{execptionName}], msg:[{m_msg}], file:[{m_file}+{m_line}], fun:[{m_func}]",
                        exceptionName(), m_msg, m_file, m_line, m_func);
   
    return m_what.c_str();
}

}}
