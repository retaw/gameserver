#include "../log.h"
#include <unistd.h>
#include <iostream>
#include <functional>
#include <chrono>
#include <memory>

using namespace water::componet;

int32_t g_total;
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


void bench(Logger &log, bool flag = true)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    const int batch = 30;
    g_total = 0;

    for (int i = 0; i < batch; ++i)
    {
		if (i!=0 && i%15 == 0)
		{
			log.stopWriter(WriterType::stdOut);
			log.stopWriter(WriterType::fileOut);
		}
		if (i%10 == 0)
		{
			log.restartWriter(WriterType::stdOut);
		}

        if (flag)
            log.debug("{} {} {}", "Hello 0123456789", "abcdefghijklmnopqrstuvwxyz", i);
        else
            log.trace("{} {} {}", "Hello 0123456789", "abcdefghijklmnopqrstuvwxyz", i);

		g_total += sizeof("Hello 0123456789") + sizeof("abcdefghijklmnopqrstuvwxyz") + 20;
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> seconds = end - start;
    printf("%f seconds, %lu bytes, %.2f msg/s, %.2f MiB/s\n",
           seconds.count(), (long int)g_total, batch / seconds.count(), g_total / seconds.count() / 1024 / 1024);
}

int main(int argc, char**argv)
{
    using namespace std::placeholders;
	Logger log;

	log.setWriter(std::make_shared<FileWriter>("./game.log"));
	log.debug("int={some int},str={your name}", 25,"hello");
    log.trace("hello logging");

    bool flag = (argc > 1) ? true : false;
    bench(log);

    int a{32};
    float f{33.3};
    std::string s{"logging"};
    stFoo foo;
    log.error("int={}, float={}, {foo data}", a, f, foo);
}

