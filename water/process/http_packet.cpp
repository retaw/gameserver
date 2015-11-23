#include "http_packet.h"
#include <sstream>
#include <cstring>

namespace water{
namespace process{

HttpPacket::HttpPacket():Packet(MAX_REQUEST_SIZE)
{
}
HttpPacket::HttpPacket(SizeType size):Packet(size)
{
}
HttpPacket::HttpPacket(const void* data, SizeType size):Packet(data,size)
{
}

HttpPacket::~HttpPacket()
{
}

void HttpPacket::addCursor(SizeType add)
{
    if(m_cursor + add > m_buf.size())
        EXCEPTION(net::PacketCursorOutOfRange, "datasize={}, cursor={}, add={}", m_buf.size(), m_cursor, add);
    m_cursor += add;  //must

	if (this->m_type == BuffType::recv)
	{
		if (this->state == ReadState::read_header)
		{
			m_headerLen = this->getHeaderLen(reinterpret_cast<const char *>(m_buf.data()), m_cursor);
			if (m_headerLen != 0 && m_headerLen <= m_cursor)
			{
				m_headerInfo.append(reinterpret_cast<const char *>(m_buf.data()), m_headerLen);
				state = ReadState::read_body;
			}
		}
		if (this->state == ReadState::read_body)
		{
			m_bodyLen = this->getBodyLength();
			if (static_cast<unsigned int>(m_bodyLen + m_headerLen) == m_cursor)
			{
				Packet::resize(m_bodyLen + m_headerLen);
				state = ReadState::read_ok;
			}
			else
			{
				Packet::resize(m_bodyLen + m_headerLen);
			}
		}
		if (this->state == ReadState::read_ok)
		{
			m_cursor = m_buf.size(); //读包完成，让上层调用返回
		}
	}
}

// http head is /r/n/r/n as eof
const int32_t HttpPacket::getHeaderLen(const char* buf, const int buflen) const
{
	const char  *s = nullptr;
	const char  *e = nullptr;
	int len = 0; 
	for (s = buf, e = s + buflen - 1; len <= 0 && s < e; s++) 
		/* Control characters are not allowed but >=128 is. */
		if (!isprint(* (unsigned char *) s) && *s != '\r' &&
				*s != '\n' && * (unsigned char *) s < 128) 
			len = -1;
		else if (s[0] == '\n' && s[1] == '\n')
			len = (int) (s - buf) + 2; 
		else if (s[0] == '\n' && &s[1] < e && 
				s[1] == '\r' && s[2] == '\n')
			len = (int) (s - buf) + 3; 
	return (len);
}

const int HttpPacket::getBodyLength() const
{
	int m_bodyLen = 0;
	std::stringstream ss(std::move(this->searchHeaderInfo(m_headerInfo, "Content-Length: ", "\r\n")));
	ss >> m_bodyLen;  //body长度
	return m_bodyLen;
}

std::string HttpPacket::searchHeaderInfo(const std::string& srcStr, const std::string &serachKey, const std::string& endKey) const
{
	std::string ret;
	std::string::size_type firstPos = srcStr.find(serachKey);
	if (firstPos != std::string::npos)
	{
		std::string::size_type secPos = srcStr.find(endKey, firstPos+serachKey.length());
		if (secPos == std::string::npos)
			return ret;
		ret = srcStr.substr(firstPos+serachKey.length(), secPos - (firstPos+serachKey.length()));
	}
	return ret;
}

}}

