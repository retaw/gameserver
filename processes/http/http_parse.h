//================================================
//作者:yechun
//时间:2014年11月21日 星期五 04时06分25秒
//文件名:http_parse.h
//描述: HTTP协议解析，仅仅支持GET 和 POST
//==============================================
#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H
#include <string>
#include <map>
#include "water/process/http_connection_manager.h"
#include "water/net/packet.h"
namespace http
{

struct HttpProtocInfo
{
	std::string request_method;    // "GET", "POST"
	std::string uri;           		// Normalized URI 
	std::string query_string;      //  terminated  
	int post_data_len = 0;
	std::string post_data;     // POST data buffer */
	std::string header_info;     //total http head
};

class HttpParse
{
public:
	HttpParse();
	~HttpParse();

public:
	bool parseHttp(water::process::HttpConnectionManager::ConnectionHolder::Ptr, water::net::Packet::CPtr );

private:
	const int getHeaderLen(const char *buf, const int buflen) const;
	bool readHeader(water::net::Packet::CPtr packet);
	bool parseHttpHeader();
	bool dealRequest(water::process::HttpConnectionManager::ConnectionHolder::Ptr connHolder);

private:
	HttpProtocInfo m_protoc_info;
};
}
#endif
