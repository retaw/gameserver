/*
 * Author: LiZhaojia - dantezhu@vip.qq.com
 *
 * Last modified: 2014-10-09 16:00 +0800
 *
 * Description:  时间处理的一些常用函数, 包括时间点的格式化, 时间点的自然日期处理等
 */

#ifndef WATER_BASE_DATE_TIME_H
#define WATER_BASE_DATE_TIME_H

#include <chrono>
#include <string>

namespace water{
namespace componet{


typedef std::chrono::system_clock Clock;
typedef Clock::time_point TimePoint;


//19700101-00:00:00 所有返回TimePoint的函数, 出错时返这个值
extern const TimePoint EPOCH; 

//以下涉及到周内自然日编号，一律按tm结构的默认规则，0-6表示周日到周六


//把时间点转成tm结构
bool timePointToTM(const TimePoint& tp, ::tm* detail);

//把满足YYYYMMDD-hh:mm:ss格式的字符串转成tp
TimePoint stringToTimePoint(const std::string& timeStr);

//tp转成YYYYMMDD-hh:mm:ss格式的字符串
std::string timePointToString(const TimePoint& tp);

//一个tp所在年是否是闰年
bool inLeapYear(const TimePoint& tp);

//tp所在的自然日的0点
TimePoint beginOfDay(const TimePoint& tp);

//两个tp是在同一个自然日内
bool inSameDay(TimePoint tp1, TimePoint tp2);

//beginOfLastDayInWeek(tp, 2) 即为tp之前（或当前）最近的上一个周二0点
TimePoint beginOfLastDayInWeek(TimePoint tp, int32_t dayInWeek);

//tp所在周的第一天的0点, sinceDayInWeek 表示把周几视为周的第一天, 默认周日为每周第一天
TimePoint beginOfWeek(TimePoint tp, int32_t sinceDayInWeek = 0);

//两个tp是否在同一周,  sinceDayInWeek 表示把周几视为一周的第一天, 默认周日为每周第一天
bool inSameWeek(TimePoint tp1, TimePoint tp2, int32_t sinceDayInWeek = 0);

//一个tp所在的自然月的第一天的0点
TimePoint beginOfMonth(TimePoint tp);

//两个tp是否在同一个自然月内
bool inSameMonth(TimePoint tp1, TimePoint tp2);

}}

#endif
