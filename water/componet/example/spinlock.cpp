#include "test.h"

#include "../spinlock.h"
#include <mutex>
#include <thread>

water::componet::Spinlock sl;
std::mutex m;

void printSomeNums()
{
    for(int i = 0; i < 100; ++i)
    {
        std::this_thread::yield();
        cout << i;
    }
    cout << endl;
}

void fun()
{
    std::lock_guard<water::componet::Spinlock> lock(sl);
    printSomeNums();
}

void testWithoutLock()
{
    std::vector<std::thread> threads;
    for(int i = 0; i < 5; ++i)
        threads.emplace_back(printSomeNums);

    for(std::thread& t : threads)
        t.join();
}

void testWithLock()
{
    std::vector<std::thread> threads;
    for(int i = 0; i < 5; ++i)
        threads.emplace_back(fun);
    for(std::thread& t : threads)
        t.join();
}

int times = 1000000;
void singleThreadBenchMutex()
{
    for(int i = 0; i < times; ++i)
        std::lock_guard<std::mutex> l(m);
}

void singleThreadBenchSpinlock()
{
    for(int i = 0; i < times; ++i)
        std::lock_guard<water::componet::Spinlock> l(sl);
}

void multiThreadBenchMutex()
{
    std::vector<std::thread> threads;
    for(int i = 0; i < 5; ++i)
        threads.emplace_back(singleThreadBenchMutex);
    for(std::thread& t : threads)
        t.join();
}

void multiThreadBenchSpinlock()
{
    std::vector<std::thread> threads;
    for(int i = 0; i < 5; ++i)
        threads.emplace_back(singleThreadBenchSpinlock);
    for(std::thread& t : threads)
        t.join();
}

int main()
{
    cout << "without lock:" << endl;
    testWithoutLock();
    cout << "with lock:" << endl;
    testWithLock();

    cout << "bench" << endl;
    PERFORMANCE(singleThreadBenchMutex, 1);
    PERFORMANCE(singleThreadBenchSpinlock, 1);
    times = 100000;
    PERFORMANCE(multiThreadBenchMutex, 1);
    PERFORMANCE(multiThreadBenchSpinlock, 1);
    return 0;
}
