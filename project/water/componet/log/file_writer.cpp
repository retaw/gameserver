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
	if (msg == nullptr)
		return;
	if (!m_running)  //为了提高点效率，放锁外面
		return;
    m_writeLock.lock();
    if (m_writeBuf->remain() > len)
    {
        m_writeBuf->put(msg, len);
    }
    else
    {
		std::cout<<"overflow writeBufs lose msg = "<<msg<<std::endl;
    }
    m_writeLock.unlock();
}

void FileWriter::run()
{
    while (m_running)
    {
		if (m_readBuf->empty())
		{
			m_writeLock.lock();
			if (!m_writeBuf->empty()) //读为空，写BUFFER有数据,switch the buffer
			{   
					//printf("read buffer empty switch \n");
					BufferPtr temp;
					temp = std::move(m_readBuf);
					m_readBuf = std::move(m_writeBuf);
					m_writeBuf = std::move(temp);
			}
			m_writeLock.unlock();
		}

		if (!m_readBuf->empty())
		{
			m_writeFile.append(m_readBuf->data(), m_readBuf->length());
			m_readBuf->reset(); //重置
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void FileWriter::start()
{
    m_writeLock.lock();
	if (!m_running)
	{
		m_thread = std::thread(std::mem_fn(&FileWriter::run), this);
	    m_running = true;
	}
    m_writeLock.unlock();
}

void FileWriter::stop()
{
    m_writeLock.lock();
	if (m_running)
	{
		m_running = false;
		m_thread.join();

		//输出缓冲中的数据
		{
			if (m_readBuf->empty())
			{
				if (!m_writeBuf->empty()) //读为空，写BUFFER有数据,switch the buffer
				{   
					//printf("read buffer empty switch \n");
					BufferPtr temp;
					temp = std::move(m_readBuf);
					m_readBuf = std::move(m_writeBuf);
					m_writeBuf = std::move(temp);
				}
			}
			if (!m_readBuf->empty())
			{
				m_writeFile.append(m_readBuf->data(), m_readBuf->length());
				m_readBuf->reset(); //重置
			}
		}
	}
    m_writeLock.unlock();
}

}}
