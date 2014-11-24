#include "test.h"


#include "../format.h"
#include <sstream>

using namespace water;
using namespace componet;

class T
{
public:
    void appendToString(std::string* str) const
    {   
        char buf[128];
        snprintf(buf, 128, "%s", "class_T");
        str->append(buf);
    }   
};

void formatTest()
{
    static uint64_t num = 0;
    const char manName[] = "小明";
    std::string str = format("{} take {} apple;", manName, 99);
    num += str.size();
}

void streamTest()
{
    static uint64_t num = 0;
    std::stringstream ss; 
    const char manName[] = "小明";
    ss << manName << " take " << 99 << " apple;";
    num += ss.str().size();
}

void sprintfTest1()
{
    static uint64_t num = 0;
    char buf[128];
    const char manName[] = "小明";
    snprintf(buf, sizeof(buf), "%s take %u apple;", manName, 99);
}

void sprintfTest2()
{
    static uint64_t num = 0;
    char* buf = new char[128]; 
    const char manName[] = "小明";
    num += snprintf(buf, 128, "%s take %u apple;", manName, 99);
}

int main()
{

    cout << format("abcdefg{people name}, {info}", "小明", "hahah") << endl;
    /*
    uint32_t times = 1000000;
    performance(formatTest, times);
    performance(streamTest, times);
    performance(sprintfTest1, times);
    performance(sprintfTest2, times);
    */
}


