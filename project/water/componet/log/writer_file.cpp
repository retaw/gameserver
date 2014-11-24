#include "writer_file.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

namespace water{
namespace componet{

WriterFile::WriterFile(const std::string filename)
    : m_filename(filename),
      m_curFilename(""),
      m_fd(1), //标准输出
      m_logHour(0)
{
    roll();
}

WriterFile::~WriterFile()
{
    ::close(m_fd);
}

bool WriterFile::load()
{
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    m_fd = ::open(m_curFilename.c_str(), O_WRONLY | O_APPEND | O_CREAT, mode);
    if (-1 == m_fd)
    {
        m_fd = 1; //重新定位到标准输出
        printf("can't open log file: %s\n", m_curFilename.c_str());
        printf("Error no is: %d\n", errno);
        printf("Error description is:%s\n", strerror(errno));
        return false;
    }
    ::unlink(m_filename.c_str());
    ::symlink(m_curFilename.c_str(), m_filename.c_str());
    return true;
}

ssize_t WriterFile::writeto(const char* msg, const size_t len)
{
	//lock();
    ssize_t size = ::write(m_fd, msg, len);
  //  unlock();
    return size;
}

int32_t WriterFile::lock()
{
    m_lock.l_type = F_WRLCK;
    return ::fcntl(m_fd, F_SETLK, &m_lock);
}

int32_t WriterFile::unlock()
{
    m_lock.l_type = F_UNLCK;
    return ::fcntl(m_fd, F_SETLK, &m_lock);
}

void WriterFile::append(const char* msg, const size_t len)
{
    if (m_logHour != timeNow().tm_hour)
        roll();

    ssize_t n = writeto(msg, len);
    if (-1 == n)
    {
        printf("append log file failed\n");
        printf("Error no is: %d\n", errno);
        printf("Error description is:%s\n", strerror(errno));
        return ;
    }
    size_t rest = len - n;
    while (rest)
    {
        ssize_t rt = writeto(msg + n, rest);
        if (-1 == rt)
        {
            printf("append log file failed\n");
            printf("Error no is: %d\n", errno);
            printf("Error description is:%s\n", strerror(errno));
            break;
        }
        rest -= rt;
    }
}

std::string WriterFile::getFileNameBynow()
{
    struct tm vtm = timeNow();
    char t_time[32];
    snprintf(t_time, sizeof(t_time), ".%4d%02d%02d-%02d", vtm.tm_year+1990, vtm.tm_mon+1, vtm.tm_mday, vtm.tm_hour);
    return m_filename + t_time;
}

void WriterFile::roll()
{
    m_curFilename = getFileNameBynow();
    load();

    m_logHour = timeNow().tm_hour;
}

tm WriterFile::timeNow()
{
    time_t now;
    now = time(&now);
    struct tm vtm;
    localtime_r(&now, &vtm);
    return vtm;
}

}}
