/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-02 19:03 +0800
 *
 * Description:  类型安全的高效率字符串格式化库
 */

#ifndef WATER_COMPONET_FORMAT_H
#define WATER_COMPONET_FORMAT_H

#include <string>
#include <cstring>
#include <cstdio>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

namespace water{
namespace componet{

//自定义类型， 需要实现 appendToString(std::string) const  成员函数, 把自身转为字符串并附加到str之后
template <typename T>
inline
typename std::enable_if<std::is_class<T>::value && !std::is_base_of<std::exception, T>::value, void>::type
appendToString(std::string* str, const T& arg)
{
    str->append(arg.toString());
}

//std::string
inline void appendToString(std::string* str, const std::string& arg)
{
    str->append(arg);
}

//c style string
inline void appendToString(std::string* str, const char* arg)
{
    str->append(arg);
}

inline void appendToString(std::string* str, char* arg)
{
    str->append(arg);
}

//std::exception
template <typename T>
inline 
typename std::enable_if<std::is_base_of<std::exception, T>::value, void>::type
appendToString(std::string* str, const T& ex)
{
    str->append(ex.what());
}

//字符, 这里, c++的类型系统非常合理, char 和 int8_t 不是一个类型, 但uint8_t 和unsigned char 是一个类型, 简直完美
inline void appendToString(std::string* str, char ch)
{
    str->append(1, ch);
}

//有符号整形
template <typename T>
inline
typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, void>::type 
appendToString(std::string* str, T arg)
{
    char buf[24];
    std::snprintf(buf, sizeof(buf) - 1, "%" PRIu64, static_cast<uint64_t>(arg));
    str->append(buf);
}

//无符号整形
template <typename T>
inline
typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, void>::type 
appendToString(std::string* str, T arg)
{
    char buf[24];
    std::snprintf(buf, sizeof(buf) - 1, "%" PRIu64, static_cast<int64_t>(arg));
    str->append(buf);
}

//浮点
template <typename T>
inline
typename std::enable_if<std::is_floating_point<T>::value, void>::type
appendToString(std::string* str, T arg)
{
    char buf[400];
    snprintf(buf, sizeof(buf), "%lf", static_cast<double>(arg));
    str->append(buf);
}

//枚举
template <typename T>
inline
typename std::enable_if<std::is_enum<T>::value, void>::type
appendToString(std::string* str, T arg)
{
    char buf[400];
    snprintf(buf, sizeof(buf), "%" PRIu64, static_cast<uint64_t>(arg));
    str->append(buf);
}

//指针
template <typename T>
inline
typename std::enable_if<std::is_pointer<T>::value, void>::type
appendToString(std::string* str, T arg)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "%p", arg);
    str->append(buf);
}

/**************************/

inline void formatAndAppend(std::string* str, const char* f)
{
    str->append(f);
}

template <typename T, typename... Args>
void formatAndAppend(std::string* str, const char* f, T&& firstArg, Args&&... args)
{
    for(std::string::size_type i = 0; f[i] != '\0'; ++i)
    {
        if(f[i] != '{')
        {
            str->append(1, f[i]);
            continue;
        }

        //f[i] == '{'的情况, 即一个占位符开始
        for(std::string::size_type j = i + 1; f[j] != '\0'; ++j)
        {
            if(f[j] != '}')
                continue;

            //f[j] == '}', 即匹配到了一个占位符
            appendToString(str, std::forward<T>(firstArg));
            return formatAndAppend(str, f + j + 1, std::forward<Args>(args)...);
        }
    }
}

template <typename... Args>
std::string format(const char* formatStr, Args&&... args)
{
    std::string ret;
    ret.reserve(std::strlen(formatStr) * 2);
    formatAndAppend(&ret, formatStr, std::forward<Args>(args)...);
    return ret;
}

template <typename... Args>
std::string format(const std::string& formatStr, Args&&... args)
{
    std::string ret;
    ret.reserve(formatStr.size() * 2);
    formatAndAppend(&ret, formatStr.c_str(), std::forward<Args>(args)...);
    return ret;
}


}}

#endif
