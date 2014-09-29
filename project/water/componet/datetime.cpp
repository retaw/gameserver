#include "datetime.h"

#include <time.h>


namespace water{

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




}

#ifdef UNIT_TEST

#include <time.h>
#include <iostream>
using namespace std;
using namespace water;

int main()
{
    cout << "time(nullptr):" << time(nullptr) << endl;

    TimePoint china = TimePoint::now(8 * 3600);
    cout << "china:" << china.toString() << " " << china.epochSeconds() << endl;

    TimePoint tp;
    tp.fromString(china.toString());
    cout << "copy: " << tp.toString() << " " << tp.epochSeconds() << endl;

    tp.setTimezone(-9 * 3600);
    cout << "copy: " << tp.toString() << " " << tp.epochSeconds() << endl;
    return 0;
}


#endif
