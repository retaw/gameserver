#include "backend_logger.h"
#include "log_file.h"
#include <time.h>
#include <cassert>

namespace water{
namespace componet{

BackendLogger::BackendLogger(const std::string filename)
    : m_filename(filename),
      m_curBuf(new Buffer),
      m_nextBuf(new Buffer)
{
    m_fullBufs.reserve(M_bufReserveSize);
}

BackendLogger::~BackendLogger()
{
}

void BackendLogger::append(const char* msg, uint32_t len)
{
    m_mutex.lock();
    if (m_curBuf->remain() > len)
    {
        m_curBuf->put(msg, len);
    }
    else
    {
        m_fullBufs.push_back(std::move(m_curBuf));
        if (m_nextBuf)
        {
            m_curBuf = std::move(m_nextBuf);
        }
        else
        {
            m_curBuf.reset(new Buffer);
        }
        m_curBuf->put(msg, len);
        m_cond.notify_one();
    }
    m_mutex.unlock();
}

void BackendLogger::threadFunc()
{
    LogFile logFile(m_filename);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    BufferVec writeBufs;
    writeBufs.reserve(M_bufReserveSize);
    while (m_running)
    {
        //m_mutex.lock();
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(writeBufs.size() == 0);
        if(m_fullBufs.empty())
        {
            //cond_wait
            std::unique_lock<std::mutex> uqlock(m_mutex);
            m_cond.wait_for(uqlock, std::chrono::seconds(3));
        }
        m_fullBufs.push_back(std::move(m_curBuf));
        m_curBuf = std::move(newBuffer1);
        writeBufs.swap(m_fullBufs);
        if (!m_nextBuf)
        {
            m_nextBuf = std::move(newBuffer2);
        }
        //m_mutex.unlock();

        assert(!writeBufs.empty());

        if (writeBufs.size() > 25)
        {
            //throw exception
            printf("overflow writeBufs.size=%lu",(unsigned long)writeBufs.size());
        }
        for (uint32_t i = 0; i < writeBufs.size(); ++i)
        {
            logFile.append(writeBufs[i]->data(), writeBufs[i]->length());
        }

        if (writeBufs.size() > 2)
        {
            writeBufs.resize(2);
        }

        if (!newBuffer1)
        {
            assert(!writeBufs.empty());
            newBuffer1 = std::move(writeBufs.back());
            writeBufs.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!writeBufs.empty());
            newBuffer2 = std::move(writeBufs.back());
            writeBufs.pop_back();
            newBuffer2->reset();
        }

        writeBufs.clear();
    }
}

void BackendLogger::start()
{
    m_thread = std::thread(std::mem_fn(&BackendLogger::threadFunc), this);
    m_running = true;
}

}}
