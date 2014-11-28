#include "test.h"
#include "../timer.h"

#include <thread>

using namespace water;
using namespace componet;

void doSth(const TimePoint& tp, int x)
{
    cout << x << ": ";
    time_t t = chrono::system_clock::to_time_t(tp); // convert to system time
    string ts = ctime(&t);                         // convert to calendar time
    ts.resize(ts.size()-1);                        // skip trailing newline
    std::cout << ts << " ";
    cout << (std::chrono::time_point_cast<std::chrono::milliseconds>(tp) - std::chrono::time_point_cast<std::chrono::seconds>(tp)).count() << endl;
    //std::this_thread::sleep_for(std::chrono::seconds(3));
}

int main()
{
    Timer timer;
    timer.regEventHandler(std::chrono::milliseconds(1000 * 2), std::bind(doSth, std::placeholders::_1, 2));
    timer.regEventHandler(std::chrono::milliseconds(1000 * 3), std::bind(doSth, std::placeholders::_1, 3));

    cout << timer.precision() << endl;
    timer.run();
    return 0;
}
