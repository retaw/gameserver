#include "test.h"

#include "../exception.h"

using namespace water;
using namespace componet;

DEFINE_EXCEPTION(TestException, ExceptionBase);

int main()
{
    try
    {
        EXCEPTION(TestException, "axdf{}", 1, 3);
    }
    catch (const ExceptionBase& ex)
    {
        cout << ex.what() << endl;
    }
    return 0;
}
