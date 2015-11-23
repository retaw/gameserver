/*
 * Description: 输出日志数据到文件
 */
#ifndef WATER_BASE_LOG_FILE_HPP
#define WATER_BASE_LOG_FILE_HPP

#include "../exception.h"

#include <string>
#include <fcntl.h>
#include <condition_variable>


namespace water {
namespace componet{

DEFINE_EXCEPTION(OpenLogFileFailed, ExceptionBase)
DEFINE_EXCEPTION(WriteLogFileFailed, ExceptionBase)

class WriterFile
{
public:
    WriterFile(const std::string name);
    ~WriterFile();

    void append(const char* msg, const size_t line);

private:
    bool load();

    ssize_t writeto(const char* msg, const size_t line);

    int32_t lock();

    int32_t unlock();

    std::string getFileNameBynow();

    void roll();

    tm timeNow();

private:
    std::string m_filename;
    std::string m_curFilename;
    int32_t m_fd;
    struct flock m_lock { F_WRLCK, SEEK_SET, 0, 0, 0 };
    int32_t m_logHour;
};

}}

#endif //#define WATER_BASE_LOG_FILE_HPP

