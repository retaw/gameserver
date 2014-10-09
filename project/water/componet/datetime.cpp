#include "datetime.h"

#include <time.h>


namespace water{
namespace componet{
namespace datetime{

const TimePoint epoch;

bool timePointToTM(const TimePoint& tp, ::tm* detail)
{
    if(detail == nullptr)
        return false;

    const time_t unixTime = Clock::to_time_t(tp);
    if(nullptr == ::localtime_r(&unixTime, detail))
        return false;
    
    return true;
}

TimePoint stringToTimePoint(const std::string& timeStr)
{
    ::tm detail;
    const char* format = "%Y%m%d-%H:%M:%S";
    const char* formatEnd = ::strptime(timeStr.c_str(), format, &detail);

    if(formatEnd == nullptr) //失败
        return epoch;

    uint64_t unixTime = ::timelocal(&detail);
    return Clock::from_time_t(unixTime);
}

std::string timePointToString(const TimePoint& tp)
{
    ::tm detail;
    if(!timePointToTM(tp, &detail))
        return "";

    char buf[40];
    const char* format = "%Y%m%d-%H:%M:%S";
    size_t bufOffset = ::strftime(buf, sizeof(buf), format, &detail);
    if(bufOffset == 0) //格式化失败
        return "";

    return buf;
}

bool inLeapYear(const TimePoint& tp)
{
    ::tm detail;
    if(!timePointToTM(tp, &detail))
        return false;
    return ((detail.tm_year % 4 == 0 && detail.tm_year % 100 != 0) //4的整数倍且非100的整数倍
            || detail.tm_year % 400 == 0 // 或 400的整数倍
           );
}

TimePoint beginOfDay(const TimePoint& tp)
{
    ::tm detail;
    if(!timePointToTM(tp, &detail))
        return epoch;

    const time_t unixTime = Clock::to_time_t(tp) - (detail.tm_hour * 3600 + 
                                                    detail.tm_min  * 60   + 
                                                    detail.tm_sec);
    return Clock::from_time_t(unixTime);
}

bool inSameDay(TimePoint tp1, TimePoint tp2)
{
    return beginOfDay(tp1) == beginOfDay(tp2);
}

TimePoint beginOfLastDayInWeek(TimePoint tp, int32_t dayInWeek)
{
    ::tm detail;
    if(!timePointToTM(tp, &detail))
        return epoch;

    const int32_t daysAfterDayInWeek = (detail.tm_wday + 7 - dayInWeek % 7) % 7; //上一个dayInWeek距今几天了
    const time_t unixTime = Clock::to_time_t(tp) - (daysAfterDayInWeek * 24 * 3600 +
                                                    detail.tm_hour * 3600 + 
                                                    detail.tm_min  * 60   + 
                                                    detail.tm_sec);
    return Clock::from_time_t(unixTime);
}

TimePoint beginOfWeek(TimePoint tp, int32_t sinceDayInWeek/* = 0*/)
{
    return beginOfLastDayInWeek(tp, sinceDayInWeek);
}

bool inSameWeek(TimePoint tp1, TimePoint tp2, int32_t sinceDayInWeek/* = 0*/)
{
    return beginOfWeek(tp1, sinceDayInWeek) == beginOfWeek(tp2, sinceDayInWeek);
}

TimePoint beginOfMonth(TimePoint tp)
{
    ::tm detail;
    if(!timePointToTM(tp, &detail))
        return epoch;

    const time_t unixTime = Clock::to_time_t(tp) - ((detail.tm_mday - 1) * 24 * 3600 +
                                                    detail.tm_hour * 3600 + 
                                                    detail.tm_min  * 60   + 
                                                    detail.tm_sec);
    return Clock::from_time_t(unixTime);
}

bool inSameMonth(TimePoint tp1, TimePoint tp2)
{
    return beginOfMonth(tp1) == beginOfMonth(tp2);
}

}}}


class TimePoint
{
public:

    struct Detail
    {
        int32_t year     = 0;
        int32_t month    = 0;
        int32_t day      = 0;
        int32_t hour     = 0;
        int32_t minute   = 0;
        int32_t second   = 0;
        int32_t weekday  = 0;
        int32_t yearday  = 0;
        int32_t offset   = 0;
    };

    TimePoint(uint64_t epochSeconds = 0, int32_t offset = 8);

    ~TimePoint();

    void setTimezone(int32_t offset);
    void setEpochMilliseconds(uint64_t epochSeconds);

    uint64_t epochMilliseconds() const;
    uint32_t epochSeconds() const;

    void setToNow();

    Detail detail() const;


    //格式化为字符串和从字符串逆格式化
    //格式为 YYYYMMDD-hh:mm:ss.timezoneoffset, 
    //如: 2014年1月2日12点30分30秒 东8区  :  "20140102-12:30:30.+28800"
    std::string toString() const;
    void fromString(const std::string& timeStr);

public:
    static TimePoint now(int32_t offset = 8);

private:
    void doGmtime() const;

private:
    uint64_t m_epochMilliseconds = 0;
    int32_t m_offset = 0;
    mutable tm* m_localtm = nullptr;
};

TimePoint::TimePoint(uint64_t epochSeconds, int32_t offset)
: m_epochMilliseconds(0), m_offset(offset)
{
}

TimePoint::~TimePoint()
{
    delete m_localtm;
}

void TimePoint::setTimezone(int32_t offset)
{
    m_offset = offset;

    //本地时间失效
    delete m_localtm;
    m_localtm = nullptr;
}

void TimePoint::setEpochMilliseconds(uint64_t epochSeconds)
{
    m_epochMilliseconds = epochSeconds;
    //本地时间失效
    delete m_localtm;
    m_localtm = nullptr;
}

uint64_t TimePoint::epochMilliseconds() const
{
    return m_epochMilliseconds;
}

uint32_t TimePoint::epochSeconds() const
{
    return static_cast<uint32_t>(epochMilliseconds() / 1000.0);
}

void TimePoint::setToNow()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    uint64_t ms = spec.tv_sec;
    ms = ms * 1000 + static_cast<uint64_t>(spec.tv_nsec / (1000 * 1000.0));

    setEpochMilliseconds(ms);
}

TimePoint::Detail TimePoint::detail() const
{
    doGmtime();
    const tm t = *m_localtm;

    Detail detail;

    detail.year     = t.tm_year + 1900;
    detail.month    = t.tm_mon + 1;
    detail.day      = t.tm_mday;
    detail.hour     = t.tm_hour;
    detail.minute   = t.tm_min;
    detail.second   = t.tm_sec;
    detail.weekday  = t.tm_wday;
    detail.yearday  = t.tm_yday;
    detail.offset = m_offset;

    return detail;
}

void TimePoint::doGmtime() const
{
    if(m_localtm != nullptr)
        return;
    m_localtm = new tm();

    const time_t timeValue = epochSeconds() + m_offset;
    gmtime_r(&timeValue, m_localtm);
    return;
}

std::string TimePoint::toString() const
{
    doGmtime();
    char buf[40];
    const char* format = "%Y%m%d-%H:%M:%S";
    size_t bufOffset = strftime(buf, sizeof(buf), format, m_localtm);
    if(bufOffset == 0)
        return "";

    snprintf(buf + bufOffset, sizeof(buf) - bufOffset, ".%+0.5d", m_offset);
    return buf;
}

void TimePoint::fromString(const std::string& timeStr)
{
    if(m_localtm == nullptr)
        m_localtm = new tm;

    const char* format = "%Y%m%d-%H:%M:%S";
    const char* timezoneStr = strptime(timeStr.c_str(), format, m_localtm);
    if(timezoneStr == nullptr)
    {
        delete m_localtm;
        m_localtm = nullptr;
        return;
    }

    const int32_t timezonePrefixSize = sizeof(".") - 1;
    sscanf(timezoneStr + timezonePrefixSize, "%d", &m_offset);

    tm t = *m_localtm;
    uint64_t unixTime = timegm(&t) - m_offset;
    setEpochMilliseconds(unixTime * 1000);
}

TimePoint TimePoint::now(int32_t offset)
{
    TimePoint tp;
    tp.setTimezone(offset);
    tp.setToNow();
    return tp;
}

