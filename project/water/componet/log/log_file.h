/*
 * Author: HongXiaoqiang  - prove.hxq@gmail.com
 *
 * Last modified: 2014-10-05 20:17 +0800
 *
 * Description: 输出日志数据到文件
 */
#ifndef WATER_BASE_LOG_FILE_HPP
#define WATER_BASE_LOG_FILE_HPP

#include <string>
#include <fcntl.h>
#include <condition_variable>

namespace water {
namespace componet{

class LogFile
{
public:
    LogFile(const std::string name);
    ~LogFile();

    bool load();

    void append(const char* msg, const size_t line);

private:
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
    std::mutex m_mutex;
    std::condition_variable m_cond;
    int32_t m_logHour;
};

}}

#endif //#define WATER_BASE_LOG_FILE_HPP

