#include "file_writer.h"
#include <time.h>
#include <cassert>
#include <iostream>

namespace water{
namespace componet{

FileWriter::FileWriter(const std::string filename)
: Writer(WriterType::fileOut),m_filename(filename),
  m_writeFile(filename),
  m_readBuf(new Buffer),
  m_writeBuf(new Buffer)
{
}

FileWriter::~FileWriter()
{
}

void FileWriter::append(const char* msg, const uint32_t len)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (msg == nullptr)
        return;
    if (!m_running) 
        return;
    if (m_writeBuf->remain() > len)
    {
        m_writeBuf->put(msg, len);
    }
    else
    {
        std::cout << "overflow writeBufs lose msg = " << msg << std::endl;
    }

    if (m_writeBuf->remain() <= REMAIN_NOTIFY_BUFFER_SIZE)
    {
        cond.notify_one();
    }
}

void FileWriter::run()
{
    std::cout << "DEBUG:文件日志的线程已经开启" << std::endl;
    while (m_running || !(m_readBuf->empty() && m_writeBuf->empty()))
    {
        if (m_readBuf->empty())
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (!m_writeBuf->empty()) //读为空，写BUFFER有数据,switch the buffer
            {	
                BufferPtr temp;
                temp = std::move(m_readBuf);
                m_readBuf = std::move(m_writeBuf);
                m_writeBuf = std::move(temp);
            }
            lock.unlock();
            std::unique_lock<std::mutex> lockWait(m_mutexWait);
            cond.wait_for(lockWait, std::chrono::milliseconds(50));
        }
        if (!m_readBuf->empty())
        {
            m_writeFile.append(m_readBuf->data(), m_readBuf->length());
            m_readBuf->reset(); //重置
        }
    }

    std::cout << "DEBUG:文件日志关闭,线程将退出" << std::endl;
}

void FileWriter::start()
{
    std::cout << "DEBUG:准备生成文件日志线程" << std::endl;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_running)
    {
        m_thread = std::thread(std::mem_fn(&FileWriter::run), this);
        m_running = true;
    }
    else
    {
        std::cout << "ERROR:本进程已经有文件日志在运行：file_writer::start(),m_running为真" <<std::endl;
    }
}

void FileWriter::stop()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_running)
    {
        m_running = false;
        cond.notify_one();
        lock.unlock();	//解锁，否则join死锁
        m_thread.join();
        std::cout << "DEBUG:文件日志线程已经结束" << std::endl;
    }
    else
    {
        lock.unlock();
    }
}

}}
