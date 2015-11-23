#ifndef HTTP_HTTP_CALLBACK_H
#define HTTP_HTTP_CALLBACK_H
#include <functional>
#include <map>
#include "water/process/http_connection_manager.h"

namespace http
{

class HttpProtocInfo;

class HttpCallBackManager
{
	typedef std::function<bool (water::process::HttpConnectionManager::ConnectionHolder::Ptr, const HttpProtocInfo *)> HttpCallBack;
public:
	~HttpCallBackManager()
	{
	}
	static HttpCallBackManager& getMe()
	{
		return me;
	}
private:
	HttpCallBackManager()
	{
		m_funcMap.clear();
	}
	static HttpCallBackManager me;

public:
	void initCallBack();
	bool reg(const std::string &key, HttpCallBack fun);
	void del(const std::string &key) ;
	HttpCallBack getFun(const std::string &key) const;

private:
	std::map<std::string, HttpCallBack> m_funcMap;
};


}
#endif
