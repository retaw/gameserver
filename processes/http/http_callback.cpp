#include "http_callback.h"
#include "http_parse.h"
#include "water/process/http_connection_manager.h"
#include "water/process/http_packet.h"
#include "http.h"
#include <iostream>
#include <sstream>
namespace http
{

using namespace water;

HttpCallBackManager HttpCallBackManager::me;

bool HttpCallBackManager::reg(const std::string &key, HttpCallBack fun)
{
	auto iter = m_funcMap.find(key);
	if (iter != m_funcMap.end())
		return false;
	m_funcMap.insert({key,fun});
	return true;
}

void HttpCallBackManager::del(const std::string &key) 
{
	m_funcMap.erase(key);
}

HttpCallBackManager::HttpCallBack HttpCallBackManager::getFun(const std::string &key) const
{
	auto iter = m_funcMap.find(key);
	if (iter == m_funcMap.end())
		return nullptr;
	return iter->second;
}

bool tstFn(HttpConnectionManager::ConnectionHolder::Ptr connHolder, const HttpProtocInfo* httpInfo) 
{   
	std:: stringstream is, os, os2; 
	os << "http/1.0 200 ok\r\nContent-Length:10\r\nConnection:Keep-Alive\r\n\r\n1234567890";
	auto packet = HttpPacket::create(os.str().c_str(), os.str().size());
	if (!connHolder->conn->setSendPacket(packet))
		return false;
	while(!connHolder->conn->trySend()){ }
	return true;
}  

void HttpCallBackManager::initCallBack()
{
	//test reg example
	this->reg("/test", std::bind(tstFn, std::placeholders::_1, std::placeholders::_2));
}

}//namespace
