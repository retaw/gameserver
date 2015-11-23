/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-09 20:37 +0800
 *
 * Description: 
 */

#ifndef WATER_COMPONET_STRING_KIT_HPP
#define WATER_COMPONET_STRING_KIT_HPP

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

#include "format.h"
#include "datetime.h"


namespace water{
namespace componet{

/*****************字符串分割******************************/


//字符串分割，放入任意支持insert(pos, item)的容器，不保留空串
template <typename StrContiner>
void splitString(StrContiner* result, const std::string& str, const std::string& delimiter)
{
    if(result == nullptr)
        return;
    result->clear();
    std::string::size_type strEnd = str.size();
    std::string::size_type subBegin = 0;
    std::string::size_type subEnd = 0;
    while(true)
    {
        subEnd = str.find(delimiter, subBegin); //subEnd此时指向分隔符

        if(subEnd == std::string::npos)//没有剩余的分隔符了, 即切分完毕
        {
            if(subBegin != strEnd)// 最后一个分隔符不在串尾
                result->insert(result->end(), str.substr(subBegin, strEnd - subBegin));
            break;
        }

        if(subBegin != subEnd)// 当前位置的字符， 不是刚找到的分隔符
            result->insert(result->end(), str.substr(subBegin, subEnd - subBegin));

        subBegin = subEnd + delimiter.length(); //将begin指向下个子串的起点
    }
}
//字符串分割，不保留空串，vector的版本
std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);
void splitStr(std::vector<std::string>* ret, const std::string& str, const std::string& delimiter);

/**********************其它类型与字符串的转换************************/
//其他类型与字符串互转
template<typename T>
std::string toString(const T& obj)
{
    return format("{}", obj);
}

template <typename T>
inline
typename std::enable_if<std::is_class<T>::value, T>::type
fromString(const std::string& str)
{
    char buf[sizeof(T)];
    T* ret = new(buf) T();
    ret->fromString(str);
    return *ret;
}


template <typename T>
inline
typename std::enable_if<!std::is_class<T>::value, T>::type
fromString(const std::string& str)
{
    T ret = T();
    std::stringstream ss(str);
    ss >> ret;
    return ret;
}

//特化，时间字符串的互转, 字符串格式为 YYYYMMDD-hh:mm:ss
std::string toString(const TimePoint& tp);
template<>
TimePoint fromString<TimePoint>(const std::string& str);


//把字符串分割并转化为对应的一组用容器存放的对象
template<typename TContiner>
void fromString(TContiner* result, const std::string& str, const std::string& delimiter)
{
    using T = typename TContiner::value_type;
    if(result == nullptr)
        return;
    result->clear();

    std::vector<std::string> strItems = splitString(str, delimiter);
    for(const auto& item : strItems)
        result->insert(result->end(), fromString<T>(item));
}
//特化，把字符串先分割并转化为对应的一组用vector存放的对象
template<typename TContiner>
TContiner fromString(const std::string& str, const std::string& delimiter)
{
    TContiner result;
    fromString(&result, str, delimiter);
    return result;
}


}}

#endif 


