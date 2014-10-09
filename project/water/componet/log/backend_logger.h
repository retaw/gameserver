/*
 * Author: HongXiaoqiang  - prove.hxq@gmail.com
 *
 * Last modified: 2014-10-05 20:11 +0800
 *
 * Description: 日志模块后台处理，输出日志数据到指定位置
 */
#ifndef WATER_BASE_BACKEND_LOGGER_HPP
#define WATER_BASE_BACKEND_LOGGER_HPP

#include "log_buffer.h"
#include <vector>
#include <memory>
#include <thread>
#include <condition_variable>
#include <chrono>

namespace water {
namespace componet{

class BackendLogger
{
public:
    typedef LogBuffer Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferVec;

    BackendLogger(const std::string filename);
    ~BackendLogger();

    void append(const char* msg, uint32_t len);

    void start();

    void stop()
    {
        m_running = false;
        m_thread.join();
    }

    void threadFunc();

private:
    static const uint32_t M_bufReserveSize = 20;
    std::string m_filename;
    BufferPtr m_curBuf;
    BufferPtr m_nextBuf;
    BufferVec m_fullBufs;
    bool m_running;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

}}
#endif //#define WATER_BASE_BACKEND_LOGGER_HPP
