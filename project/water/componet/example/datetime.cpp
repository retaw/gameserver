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
    auto now = datetime::Clock::now();
    cout << datetime::Clock::to_time_t(now) << endl;

    std::string timeStr = datetime::timePointToString(now);
    cout << timeStr << endl;

    auto tp = datetime::stringToTimePoint(timeStr);
    cout << datetime::Clock::to_time_t(tp) << endl;
}

void compare()
{
    auto now = datetime::Clock::now();
    cout << toString(now) << endl;
    cout << toString(datetime::beginOfDay(now)) << endl;
    cout << toString(datetime::beginOfMonth(now)) << endl;
    cout << toString(datetime::beginOfWeek(now)) << endl;
    cout << toString(datetime::beginOfWeek(now, 1)) << endl;
}

int main()
{
    compare();
    return 0;
}
