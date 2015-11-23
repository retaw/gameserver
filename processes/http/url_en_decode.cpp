//================================================
//作者:yechun
//时间:2014年11月20日 星期四 08时59分16秒
//文件名:url_en_decode.cpp
//描述: url 编码和反编码, 见https://github.com/php/php-src/blob/master/ext/standard/url.c
//对除了  - . _  字母 数字 之外所有可打印字符用 % +2个十六进制表示
//==============================================
#include <string>
#include "url_en_decode.h"
namespace http
{

UrlCode UrlCode::me;

int UrlCode::php_htoi(const char *s)
{
	int value;
	int c;
	c = ((unsigned char *)s)[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;
	c = ((unsigned char *)s)[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;
	return (value);
}

std::string UrlCode::urlEncode(const std::string& src)
{
	static char hex[] = "0123456789ABCDEF";
	std::string dst;

	for (size_t i = 0; i < src.size(); ++i)
	{
		unsigned char c = src[i];
		
		if (c == ' ') 
		{
			dst += '+';
		}
		else if ((c < '0' && c != '-' && c != '.') ||
				(c < 'A' && c > '9') ||
				(c > 'Z' && c < 'a' && c != '_') ||
				(c > 'z')) 
		{
			dst += '%';
			dst += hex[c >> 4];
			dst += hex[c & 15];
		}
		else 
		{
			dst += c;
		}
	}
	return dst;
}

std::string UrlCode::urlDecode(const std::string &src) 
{
	std::string dst;

	int len = src.length();
	const char *data = src.c_str();
	while (len--) 
	{
		if (*data == '+') 
		{
			dst += ' ';
		}
		else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))
				&& isxdigit((int) *(data + 2))) 
		{
			dst += (char) php_htoi(data + 1);
			data += 2;
			len -= 2;
		}
		else 
		{
			dst += (*data);
		}
		data++;
	}
	return dst;
}
/*
#include <iostream>
int main()
{
	std::string src = "updateabcsettitle='我是+%标题',content='我是内容?' where id=1aaa-._\\";
	std::cout<<src<<std::endl;
	std::string encodestr = std::move(UrlCode::getMe().urlEncode(src));
	std::string outStr = std::move(UrlCode::getMe().urlDecode(encodestr));
	std::cout<<encodestr<<std::endl<<std::endl;
	std::cout<<outStr<<std::endl;
}
*/

} //namespace
