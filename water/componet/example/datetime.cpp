#include "test.h"
#include "../datetime.h"
#include "../string_kit.h"

#include <chrono>
#include <time.h>

using namespace water;
using namespace componet;

void epoch()
{
    cout << ::time(nullptr) << endl;
    auto now = std::chrono::system_clock::now();
    cout << std::chrono::system_clock::to_time_t(now) << endl;
}

void strConvert()
{
    auto now = Clock::now();
    cout << Clock::to_time_t(now) << endl;

    std::string timeStr = timePointToString(now);
    cout << timeStr << endl;

    auto tp = stringToTimePoint(timeStr);
    cout << Clock::to_time_t(tp) << endl;
}

void compare()
{
    auto now = Clock::now();
    cout << toString(now) << endl;
    cout << toString(beginOfDay(now)) << endl;
    cout << toString(beginOfMonth(now)) << endl;
    cout << toString(beginOfWeek(now)) << endl;
    cout << toString(beginOfWeek(now, 1)) << endl;
}

int main()
{
    compare();
    return 0;
}
