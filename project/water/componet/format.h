/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-02 19:03 +0800
 *
 * Description:  类型安全的字符串格式化字符串库, 实测gcc O3优化下比stringstream快约1.5-3倍
 */

#ifndef WATER_BASE_FORMAT_H
#define WATER_BASE_FORMAT_H

#include <string>

namespace water{
namespace componet{

//自定义类型， 需要实现 appendToString(std::string) const;  成员函数, 把自身转为字符串并附加到str之后
template <typename T>
inline
typename std::enable_if<std::is_class<T>::value, void>::type
appendToString(std::string* str, const T& arg)
{
    arg.appendToString(str);
}

//整形
template <typename T>
inline
typename std::enable_if<std::is_integral<T>::value, void>::type 
appendToString(std::string* str, T arg)
{
    char buf[24];
    snprintf(buf, sizeof(buf) - 1, "%lu", static_cast<uint64_t>(arg));
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


inline void formatImpl(std::string* str, const char* f)
{
    str->append(f);
}

template <typename T, typename... Args>
void formatImpl(std::string* str, const char* f, const T& firstArg, const Args&... args)
{
    for(std::string::size_type i = 0; f[i] != '\0'; ++i)
    {
        if(f[i] != '{')
        {
            str->append(1, f[i]);
            continue;
        }

        //f[i] == '{'的情况, 即一个占位符开始
        for(std::string::size_type j = i; f[j] != '\0'; ++j)
        {
            if(f[j] != '}')
                continue;

            //f[j] == '}', 即匹配到了一个占位符
            appendToString(str, std::forward<const T>(firstArg));
            return formatImpl(str, f + i + 1, std::forward<const Args>(args)...);
        }
    }
}

template <typename... Args>
std::string format(const std::string& formatStr, const Args&... args)
{
    std::string ret;
    ret.reserve(formatStr.size() * 2);
    formatImpl(&ret, formatStr.c_str(), std::forward<const Args>(args)...);
    return std::move(ret);
}

}}

#endif
