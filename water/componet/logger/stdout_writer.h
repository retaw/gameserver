/*
 * Description: 日志模块后台处理，输出日志数据到标准输出
 */
#ifndef WATER_COMPONET_STDOUT_WRITER_HPP
#define WATER_COMPONET_STDOUT_WRITER_HPP
#include "writer.h"
#include "../spinlock.h"

namespace water {
namespace componet{

class StdoutWriter : public Writer 
{
public:
    StdoutWriter();
    ~StdoutWriter();
public:
    //interface from base
    void append(const char* msg, const uint32_t len);
    void start();
    void stop();

private:
    bool m_running = true;
    Spinlock m_writeLock;
};

}}
#endif
