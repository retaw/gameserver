#include "../log.h"
#include <unistd.h>
#include <iostream>
#include <functional>
#include <chrono>
#include <memory>

using namespace water::componet;

struct stFoo
{
    int a{2};
    std::string s{"hello"};
public:
    void appendToString(std::string* str) const
    {
        str->append(s);
    }
};

LogStream& operator << (LogStream& ss, stFoo& foo)
{
    ss << "a=" << foo.a << " " << "s=" << foo.s;
    return ss;
}


void bench(Logger &log, bool flag = true, int st = 0)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    const int batch = st + 30000;
	int32_t g_total = 0;
	std::string str("Hello 0123456789abcdefghijklmnopqrstuvwxyz12345678910");


    for (int i = st; i < batch; ++i)
    {
	/*	if (i!=0 && i%15 == 0)
		{
			log.stopStdOutWriter();
		}
		if (i%10 == 0)
		{
			log.restartStdOutWriter();
		}*/

        if (flag)
            log.debug("{} {} {}", "Hello 0123456789", "abcdefghijklmnopqrstuvwxyz", i);
        else
            log.trace("{} {} {}", "Hello 0123456789", "abcdefghijklmnopqrstuvwxyz", i);

		g_total += str.length();
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> seconds = end - start;
    printf("%f seconds, %lu bytes, %.2f msg/s, %.2f MiB/s\n",
           seconds.count(), (long int)g_total, batch / seconds.count(), g_total / seconds.count() / 1024 / 1024);
}

void foo(Logger *log)
{
	bench(*log, false, 30001);
}

int main(int argc, char**argv)
{
    using namespace std::placeholders;
	Logger log;
	log.stopStdOutWriter();

	log.setWriter(std::make_shared<FileWriter>("./game.log"));
	std::thread work = std::thread(foo, &log); 


	//log.debug("int={some int},str={your name}", 25,"hello");
   // log.trace("hello logging");

    bool flag = (argc>1)?true:false;
    bench(log);

    int a{32};
    float f{33.3};
    std::string s{"logging"};
    stFoo foo;
    //log.error("int={},float={}, {foo data}", a, f, foo);

	work.join();

}
