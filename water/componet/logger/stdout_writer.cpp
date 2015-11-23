#include "stdout_writer.h"
#include <iostream>

namespace water{
namespace componet{

StdoutWriter::StdoutWriter():Writer(WriterType::stdOut)
{
}

StdoutWriter::~StdoutWriter()
{
}

void StdoutWriter::start()
{
    m_running = true;
}
void StdoutWriter::stop()
{
    m_running = false;
}

void StdoutWriter::append(const char* msg, const uint32_t len)
{
    if (!m_running)
        return;
    if (msg == nullptr)
        return;
    m_writeLock.lock();
    std::cout<<msg;
    m_writeLock.unlock();
    return;
}

}}
