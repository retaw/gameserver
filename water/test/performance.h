/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 22:01 +0800
 *
 * Description: 用于性能测试
 */

#include <iostream>
#include <memory>
#include <functional>
#include <chrono>
using namespace std;

void performance ( void (*pfun)(), uint32_t times, std::string info = "" )
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    cout << "\n\n" << info << " start:" << endl;
    start = std::chrono::system_clock::now();
    for(int i = 0; i < times; ++i)
        (*pfun)();
    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << std::dec << std::fixed 
    << info
    << " finished computation at " << std::ctime(&end_time)
    << "elapsed time: " << elapsed_seconds.count() 
    << "s; average time: " << elapsed_seconds.count() / times << endl;;
}

#define PERFORMANCE(fun, times) performance(fun, times, #fun)
