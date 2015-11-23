#ifndef HTTP_URL_CODE_H
#define HTTP_URL_CODE_H

namespace http
{
class UrlCode
{
public:
	~UrlCode()=default;
	static UrlCode& getMe()
	{
		return me;
	}
private:
	UrlCode()=default;
	static UrlCode me;
public:
	std::string urlEncode(const std::string& src);
	std::string urlDecode(const std::string &src);
private:
	int php_htoi(const char *s);
};

}
#endif
