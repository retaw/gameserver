#include "http_parse.h"
#include "url_en_decode.h"
#include "http_callback.h"
#include <sstream>
#include <sys/socket.h>
#include <cstring>

namespace http
{

HttpParse::HttpParse()
{
}

HttpParse::~HttpParse()
{
}

// http head is /r/n/r/n as eof
const int HttpParse::getHeaderLen(const char *buf, const int buflen) const
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

bool HttpParse::readHeader(water::net::Packet::CPtr packet)
{
	int headerLen = 0;
	int realyRead = packet->size();
	const char *buf = reinterpret_cast<const char *>(packet->data());

	headerLen = getHeaderLen(buf, realyRead);

	if (realyRead < headerLen)  //bad read
		return false;

	if (headerLen <= 0)  // http header is not ok
		return false;

	m_protoc_info.header_info.append(buf,headerLen);  //填充header
	if (realyRead >= headerLen)  //有body info
	{
		m_protoc_info.post_data_len = realyRead-headerLen;  //已经读取到的body长度
		m_protoc_info.post_data.append(buf + headerLen, realyRead - headerLen);  //已读取bodybuf
	}
	return true;
}


//根据协议格式，依次提取信息
bool HttpParse::parseHttpHeader()
{
	//依次提取request信息
	std::string::size_type posFirst, posSecond = std::string::npos;
	posFirst = m_protoc_info.header_info.find(" ");  //第一个空格
	if (posFirst == std::string::npos)
	{
		return false;
	}
	m_protoc_info.request_method = m_protoc_info.header_info.substr(0, posFirst);
	if (m_protoc_info.request_method != "GET" && m_protoc_info.request_method != "POST")
	{
		return false;
	}

	std::string::size_type posNext = posFirst+1;
	//依次提取uri信息
	posSecond = m_protoc_info.header_info.find(" ", posNext);  //第二个空格
	if (posSecond == std::string::npos)
	{
		return false;
	}
	m_protoc_info.uri = m_protoc_info.header_info.substr(posNext, posSecond - posNext);

	//url decode 
	m_protoc_info.uri = std::move(UrlCode::getMe().urlDecode(m_protoc_info.uri));
	//提取uri 和 request info
	std::string::size_type pos = m_protoc_info.uri.find("?");
	if (pos != std::string::npos)
	{
		m_protoc_info.query_string = m_protoc_info.uri.substr(pos + 1);  //2行相互依赖，不可以换序
		m_protoc_info.uri = m_protoc_info.uri.substr(0, pos);
	}

	return true;
}

bool HttpParse::parseHttp(water::process::HttpConnectionManager::ConnectionHolder::Ptr connHolder, water::net::Packet::CPtr packet)
{
	if (!this->readHeader(packet))
		return false;

	if (!this->parseHttpHeader())  //分析http协议头
		return false;

	return dealRequest(connHolder);
}

bool HttpParse::dealRequest(water::process::HttpConnectionManager::ConnectionHolder::Ptr connHolder)
{
	auto fun = HttpCallBackManager::getMe().getFun(m_protoc_info.uri);
	if (fun != nullptr)
		fun(connHolder, &m_protoc_info);
	return true;
}

}
