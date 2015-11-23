/*
 * Description: 日志模块后台处理，输出日志数据到指定文件
 */
#ifndef WATER_COMPONET_FILE_WRITER_HPP
#define WATER_COMPONET_FILE_WRITER_HPP

#include "writer_buffer.h"
#include <vector>
#include <memory>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <condition_variable>
#include "writer.h"
#include "writer_file.h"

namespace water {
namespace componet{

class FileWriter : public Writer 
{
public:
    typedef WriterBuffer Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;

    FileWriter(const std::string filename);
    ~FileWriter();

    void append(const char* msg, const uint32_t len);
    void start();
    void stop();

    void run();

private:
    std::string m_filename;
    WriterFile m_writeFile;

    BufferPtr m_readBuf;
    BufferPtr m_writeBuf;

    volatile bool m_running = false;
    std::thread m_thread;
    std::mutex m_mutex;//写队列与队列交换互斥
    std::mutex m_mutexWait;//用于cond并对缓冲时间进行设置
    std::condition_variable cond;
};

}}
#endif //#define WATER_COMPONET_FILE_WRITER_HPP
