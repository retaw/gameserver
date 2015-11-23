#include "test.h"

#include "../reflector.h"

using namespace water;
using namespace componet;
using namespace test;

struct A
{
    A()
    {
    }
    virtual std::string name()
    {
        return "A";
    }

    int code = 0;
};

struct B0 : A
{
    virtual std::string name()
    {
        return "[B0]";
    }
};

struct B1 : A
{
    B1(std::string c)
    : code(c)
    {
    }
    virtual std::string name()
    {
        return code + "[B1]";
    }
    std::string code;
};

struct B2 : A
{
    B2(std::string code1_, std::string code2_)
    : code1(code1_), code2(code2_)
    {
    }
    virtual std::string name()
    {
        return code1 + code2 + "[B2]";
    }
    std::string code1;
    std::string code2;
};

void reflectorTest()
{
    Reflector<std::string, A> reflector0;
    reflector0.reg<B0>("B0");

    Reflector<std::string, A, std::string> reflector1;
    reflector1.reg<B1>("B1");

    Reflector<std::string, A, std::string, std::string> reflector2;
    reflector2.reg<B2>("B2");


    A* l[3] = 
    {
        reflector0.produce("B0") ,
        reflector1.produce("B1", "haha"),
        reflector2.produce("B2", "b2", "2b"),
    };

    for(A* a : l)
        cout << a << ":" << a->name() << endl;

}

int main()
{
    reflectorTest();
}
