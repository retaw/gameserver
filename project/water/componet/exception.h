#ifndef WATER_BASE_EXCEPTION_HPP
#define WATER_BASE_EXCEPTION_HPP

#include <exception>
#include <string>

namespace water{
namespace componet{

class ExceptionBase : public std::exception
{
public:
    ExceptionBase(const std::string& msg = "", 
                  const std::string& file = "<unknown file>", 
                  const std::string& func = "<unknown func>", 
                  int32_t line = 0,
                  int32_t sysErrno = 0) noexcept;
    virtual ~ExceptionBase() noexcept;


    virtual std::string exceptionName() const noexcept;
    std::string msg() const noexcept;
    const char* what() const noexcept override;

protected:
    const std::string m_msg;
    const std::string m_file;
    const std::string m_func;
    const int32_t m_line;
    const int32_t m_sysErrno;
    mutable std::string m_what;
};

#define DEFINE_EXCEPTION(ExceptionClass, Base)\
class ExceptionClass : public Base\
{\
public:\
    ExceptionClass(const std::string& msg = "",\
            const std::string& file = "<unknown file>", \
            const std::string& func = "<unknown func>", \
            int32_t line = 0,\
            int32_t sysErrno = 0) noexcept\
    : Base(msg, file, func, line, sysErrno)\
    {\
    }\
    ~ExceptionClass() noexcept {}\
    std::string exceptionName() const noexcept override\
    {\
        return #ExceptionClass;\
    }\
};

#define EXCEPTION(ExceptionType, msg)\
    throw ExceptionType(msg, __FILE__, __PRETTY_FUNCTION__, __LINE__);


#define SYS_EXCEPTION(ExceptionType, msg)\
    throw ExceptionType(msg, __FILE__, __PRETTY_FUNCTION__, __LINE__, errno);

}}

#endif //#ifndef WATER_EXCEPTION_HPP
