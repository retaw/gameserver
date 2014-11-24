#include "test.h"

#include "../event.h"

using namespace water;
using namespace componet;




class DoSth
{
public:
    void fun(int i, std::string str)
    {
        cout << "mem=>" << i << ":" << str << endl;
    }
};


void fun(int i, std::string str)
{
    cout << "fun=>" << i << ":" << str << endl;
}

void testDelegate()
{
    Delegate<void (int, std::string)> delegate(fun);
    delegate(5, "279");
}

int main()
{
    auto doSth = std::make_shared<DoSth>();

    /*
    auto regid1 = event.reg(std::bind(&DoSth::fun, doSth, std::placeholders::_1, std::placeholders::_2));
    auto regid2 = event.reg(fun);
    event.raise(1, "a");

    event.unreg(regid1);
    event.raise(2, "b");
    */
    Event<void(int, std::string)> event;

    Delegate<void(int, std::string)> handle1(std::bind(&DoSth::fun, doSth, std::placeholders::_1, std::placeholders::_2));
    event += handle1;
    event(1, "a");

    Delegate<void(int, std::string)> handle2(fun);
    event += handle2;
    event(2, "b");

    event -= handle1;
    event(3, "c");


    return 0;
}

